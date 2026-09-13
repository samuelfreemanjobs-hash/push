#!/usr/bin/env node
import { Server } from '@modelcontextprotocol/sdk/server/index.js';
import { StdioServerTransport } from '@modelcontextprotocol/sdk/server/stdio.js';
import {
  CallToolRequestSchema,
  ListToolsRequestSchema,
} from '@modelcontextprotocol/sdk/types.js';
import { z } from 'zod';
import {
  createAutofillJob,
  createDesignExport,
  getAutofillJob,
  getExportJob,
  pollUntilDone,
  toAutofillData,
} from './lib/canva-client.js';

function getToken() {
  return process.env.CANVA_ACCESS_TOKEN || '';
}

function textResult(obj) {
  return {
    content: [{ type: 'text', text: typeof obj === 'string' ? obj : JSON.stringify(obj, null, 2) }],
  };
}

const server = new Server(
  { name: 'push-canva-connect', version: '1.0.0' },
  { capabilities: { tools: {} } },
);

server.setRequestHandler(ListToolsRequestSchema, async () => ({
  tools: [
    {
      name: 'canva_config_status',
      description:
        'Check whether Canva Connect env vars are set (does not print secrets). Use CANVA_ACCESS_TOKEN from OAuth.',
      inputSchema: { type: 'object', properties: {} },
    },
    {
      name: 'canva_create_autofill',
      description:
        'Create a Canva brand-template autofill job. Requires user-confirmed copy; mutates Canva designs.',
      inputSchema: {
        type: 'object',
        properties: {
          brand_template_id: { type: 'string', description: 'Canva brand template ID' },
          title: { type: 'string', description: 'Design title' },
          fields: {
            type: 'object',
            description: 'Key-value text fields (e.g. Headline, Subhead)',
            additionalProperties: { type: 'string' },
          },
          field_map: {
            type: 'object',
            description: 'Optional map ourKey → Canva element name; defaults to identity',
            additionalProperties: { type: 'string' },
          },
          wait: { type: 'boolean', description: 'Poll until job completes', default: false },
        },
        required: ['brand_template_id', 'title', 'fields'],
      },
    },
    {
      name: 'canva_get_autofill',
      description: 'Get status of a Canva autofill job by ID.',
      inputSchema: {
        type: 'object',
        properties: { job_id: { type: 'string' } },
        required: ['job_id'],
      },
    },
    {
      name: 'canva_export_design',
      description: 'Export a Canva design to PNG/PDF. Creates export job; optional wait for URL.',
      inputSchema: {
        type: 'object',
        properties: {
          design_id: { type: 'string' },
          format: { type: 'string', enum: ['png', 'pdf', 'jpg'], default: 'png' },
          wait: { type: 'boolean', default: true },
        },
        required: ['design_id'],
      },
    },
  ],
}));

server.setRequestHandler(CallToolRequestSchema, async (request) => {
  const { name, arguments: args } = request.params;

  if (name === 'canva_config_status') {
    const token = getToken();
    return textResult({
      canva_access_token_set: Boolean(token),
      hint: token
        ? 'Token present. For chat-driven designs, also connect official Canva MCP in Cursor Settings.'
        : 'Set CANVA_ACCESS_TOKEN (OAuth user access token from Canva Developers).',
      official_cursor_mcp:
        'Cursor → Settings → Tools & MCP → Canva → Connect (separate from this Connect API server)',
      docs: 'https://www.canva.dev/docs/connect/',
    });
  }

  const token = getToken();
  if (!token) {
    return {
      content: [
        {
          type: 'text',
          text: 'CANVA_ACCESS_TOKEN is not set. Create a Canva integration and complete OAuth.',
        },
      ],
      isError: true,
    };
  }

  try {
    if (name === 'canva_create_autofill') {
      const parsed = z
        .object({
          brand_template_id: z.string(),
          title: z.string(),
          fields: z.record(z.string()),
          field_map: z.record(z.string()).optional(),
          wait: z.boolean().optional(),
        })
        .parse(args ?? {});

      const fieldMap = parsed.field_map ?? parsed.fields;
      const data =
        parsed.field_map
          ? toAutofillData(parsed.fields, parsed.field_map)
          : Object.fromEntries(
              Object.entries(parsed.fields).map(([k, v]) => [
                k,
                { type: 'text', text: String(v) },
              ]),
            );

      const created = await createAutofillJob(token, {
        brandTemplateId: parsed.brand_template_id,
        title: parsed.title,
        data,
      });

      const jobId = created?.job?.id || created?.id;
      if (parsed.wait && jobId) {
        const done = await pollUntilDone(() => getAutofillJob(token, jobId));
        return textResult(done);
      }
      return textResult(created);
    }

    if (name === 'canva_get_autofill') {
      const { job_id } = z.object({ job_id: z.string() }).parse(args ?? {});
      return textResult(await getAutofillJob(token, job_id));
    }

    if (name === 'canva_export_design') {
      const parsed = z
        .object({
          design_id: z.string(),
          format: z.enum(['png', 'pdf', 'jpg']).default('png'),
          wait: z.boolean().default(true),
        })
        .parse(args ?? {});

      const created = await createDesignExport(token, {
        designId: parsed.design_id,
        format: parsed.format,
      });
      const exportId = created?.job?.id || created?.id;
      if (parsed.wait && exportId) {
        const done = await pollUntilDone(() => getExportJob(token, exportId));
        return textResult(done);
      }
      return textResult(created);
    }

    return { content: [{ type: 'text', text: `Unknown tool: ${name}` }], isError: true };
  } catch (err) {
    return {
      content: [{ type: 'text', text: err.message || String(err) }],
      isError: true,
    };
  }
});

const transport = new StdioServerTransport();
await server.connect(transport);
console.error('push-canva-connect MCP server running on stdio');
