#!/usr/bin/env node
import crypto from 'node:crypto';
import { Server } from '@modelcontextprotocol/sdk/server/index.js';
import { StdioServerTransport } from '@modelcontextprotocol/sdk/server/stdio.js';
import {
  CallToolRequestSchema,
  ListToolsRequestSchema,
} from '@modelcontextprotocol/sdk/types.js';
import { z } from 'zod';
import { createEtsyClientFromEnv } from './lib/etsy-client.js';
import {
  buildAuthorizationUrl,
  exchangeAuthorizationCode,
  generatePkcePair,
  refreshAccessToken,
} from './lib/oauth.js';

function textResult(obj) {
  return {
    content: [{ type: 'text', text: typeof obj === 'string' ? obj : JSON.stringify(obj, null, 2) }],
  };
}

function configStatus() {
  return {
    ETSY_API_KEY: Boolean(process.env.ETSY_API_KEY),
    ETSY_ACCESS_TOKEN: Boolean(process.env.ETSY_ACCESS_TOKEN),
    ETSY_REFRESH_TOKEN: Boolean(process.env.ETSY_REFRESH_TOKEN),
    ETSY_SHOP_ID: Boolean(process.env.ETSY_SHOP_ID),
    ETSY_REDIRECT_URI: Boolean(process.env.ETSY_REDIRECT_URI),
    docs: 'https://developer.etsy.com/documentation/',
  };
}

const server = new Server(
  { name: 'push-etsy-open-api', version: '1.0.0' },
  { capabilities: { tools: {} } },
);

server.setRequestHandler(ListToolsRequestSchema, async () => ({
  tools: [
    {
      name: 'etsy_config_status',
      description: 'Check which Etsy env vars are set (no secret values returned).',
      inputSchema: { type: 'object', properties: {} },
    },
    {
      name: 'etsy_build_oauth_url',
      description:
        'Build Etsy OAuth authorization URL with PKCE. Returns verifier — store it to exchange the code later.',
      inputSchema: {
        type: 'object',
        properties: {
          state: { type: 'string', description: 'CSRF state (optional; random if omitted)' },
        },
      },
    },
    {
      name: 'etsy_exchange_oauth_code',
      description:
        'Exchange authorization code for access + refresh tokens (paste code from redirect).',
      inputSchema: {
        type: 'object',
        properties: {
          code: { type: 'string' },
          code_verifier: { type: 'string' },
        },
        required: ['code', 'code_verifier'],
      },
    },
    {
      name: 'etsy_refresh_token',
      description: 'Refresh access token using ETSY_REFRESH_TOKEN env var.',
      inputSchema: { type: 'object', properties: {} },
    },
    {
      name: 'etsy_get_shop',
      description: 'Get shop metadata for ETSY_SHOP_ID.',
      inputSchema: { type: 'object', properties: {} },
    },
    {
      name: 'etsy_list_active_listings',
      description: 'List active shop listings (read-only).',
      inputSchema: {
        type: 'object',
        properties: {
          limit: { type: 'number', default: 25 },
          offset: { type: 'number', default: 0 },
        },
      },
    },
    {
      name: 'etsy_create_draft_listing',
      description:
        'Create a draft listing (write). Confirm with the user before calling. Body matches Etsy Open API v3 createDraftListing JSON.',
      inputSchema: {
        type: 'object',
        properties: {
          listing: {
            type: 'object',
            description: 'Draft listing fields (quantity, title, description, price, taxonomy_id, etc.)',
          },
        },
        required: ['listing'],
      },
    },
    {
      name: 'etsy_update_listing',
      description: 'PATCH a listing (write). Confirm with user before calling.',
      inputSchema: {
        type: 'object',
        properties: {
          listing_id: { type: 'number' },
          fields: { type: 'object' },
        },
        required: ['listing_id', 'fields'],
      },
    },
  ],
}));

server.setRequestHandler(CallToolRequestSchema, async (request) => {
  const { name, arguments: args } = request.params;

  try {
    if (name === 'etsy_config_status') {
      return textResult(configStatus());
    }

    if (name === 'etsy_build_oauth_url') {
      const clientId = process.env.ETSY_API_KEY;
      const redirectUri = process.env.ETSY_REDIRECT_URI;
      if (!clientId || !redirectUri) {
        return textResult({
          error: 'Set ETSY_API_KEY and ETSY_REDIRECT_URI first',
          ...configStatus(),
        });
      }
      const { verifier, challenge } = generatePkcePair();
      const state =
        (args && typeof args.state === 'string' && args.state) ||
        crypto.randomUUID?.() ||
        String(Date.now());
      const url = buildAuthorizationUrl({
        clientId,
        redirectUri,
        state,
        codeChallenge: challenge,
      });
      return textResult({
        authorization_url: url,
        code_verifier: verifier,
        state,
        next: 'Open URL in browser, approve, then call etsy_exchange_oauth_code with code + code_verifier',
      });
    }

    if (name === 'etsy_exchange_oauth_code') {
      const parsed = z
        .object({ code: z.string(), code_verifier: z.string() })
        .parse(args ?? {});
      const tokens = await exchangeAuthorizationCode({
        clientId: process.env.ETSY_API_KEY,
        redirectUri: process.env.ETSY_REDIRECT_URI,
        code: parsed.code,
        codeVerifier: parsed.code_verifier,
      });
      return textResult({
        ...tokens,
        hint: 'Save access_token as ETSY_ACCESS_TOKEN and refresh_token as ETSY_REFRESH_TOKEN in your env.',
      });
    }

    if (name === 'etsy_refresh_token') {
      if (!process.env.ETSY_REFRESH_TOKEN) {
        return textResult({ error: 'ETSY_REFRESH_TOKEN not set' });
      }
      const tokens = await refreshAccessToken({
        clientId: process.env.ETSY_API_KEY,
        refreshToken: process.env.ETSY_REFRESH_TOKEN,
      });
      return textResult(tokens);
    }

    const client = createEtsyClientFromEnv();

    if (name === 'etsy_get_shop') {
      return textResult(await client.getShop());
    }

    if (name === 'etsy_list_active_listings') {
      const { limit, offset } = z
        .object({ limit: z.number().default(25), offset: z.number().default(0) })
        .parse(args ?? {});
      return textResult(await client.getActiveListings(limit, offset));
    }

    if (name === 'etsy_create_draft_listing') {
      const { listing } = z.object({ listing: z.record(z.unknown()) }).parse(args ?? {});
      return textResult(await client.createDraftListing(listing));
    }

    if (name === 'etsy_update_listing') {
      const { listing_id, fields } = z
        .object({ listing_id: z.number(), fields: z.record(z.unknown()) })
        .parse(args ?? {});
      return textResult(await client.updateListing(listing_id, fields));
    }

    return { content: [{ type: 'text', text: `Unknown tool: ${name}` }], isError: true };
  } catch (err) {
    return {
      content: [
        {
          type: 'text',
          text: JSON.stringify(
            { message: err.message, status: err.status, details: err.details },
            null,
            2,
          ),
        },
      ],
      isError: true,
    };
  }
});

const transport = new StdioServerTransport();
await server.connect(transport);
console.error('push-etsy-open-api MCP server running on stdio');
