#!/usr/bin/env node
import { Server } from '@modelcontextprotocol/sdk/server/index.js';
import { StdioServerTransport } from '@modelcontextprotocol/sdk/server/stdio.js';
import {
  CallToolRequestSchema,
  ListToolsRequestSchema,
} from '@modelcontextprotocol/sdk/types.js';
import { z } from 'zod';
import { createGeminiClient, getGeminiConfig } from './lib/gemini-client.js';

function textResult(obj) {
  return {
    content: [{ type: 'text', text: typeof obj === 'string' ? obj : JSON.stringify(obj, null, 2) }],
  };
}

function configStatus() {
  const { apiKey, model } = getGeminiConfig();
  return {
    GEMINI_API_KEY: Boolean(apiKey),
    GEMINI_MODEL: model,
    docs: 'https://ai.google.dev/gemini-api/docs',
    get_key: 'https://aistudio.google.com/apikey',
  };
}

const server = new Server(
  { name: 'push-gemini', version: '1.0.0' },
  { capabilities: { tools: {} } },
);

server.setRequestHandler(ListToolsRequestSchema, async () => ({
  tools: [
    {
      name: 'gemini_config_status',
      description: 'Check Gemini API key and default model (no secrets returned).',
      inputSchema: { type: 'object', properties: {} },
    },
    {
      name: 'gemini_generate',
      description:
        'Single-turn text generation with Google Gemini. Uses GEMINI_MODEL unless model is overridden.',
      inputSchema: {
        type: 'object',
        properties: {
          prompt: { type: 'string', description: 'User prompt / task' },
          system_instruction: { type: 'string', description: 'Optional system prompt' },
          model: { type: 'string', description: 'e.g. gemini-2.0-flash, gemini-1.5-pro' },
          temperature: { type: 'number', minimum: 0, maximum: 2 },
          max_output_tokens: { type: 'number' },
        },
        required: ['prompt'],
      },
    },
    {
      name: 'gemini_chat',
      description:
        'Multi-turn chat. Pass full message history; last message must be role user.',
      inputSchema: {
        type: 'object',
        properties: {
          messages: {
            type: 'array',
            items: {
              type: 'object',
              properties: {
                role: { type: 'string', enum: ['user', 'assistant'] },
                text: { type: 'string' },
              },
              required: ['role', 'text'],
            },
          },
          system_instruction: { type: 'string' },
          model: { type: 'string' },
          temperature: { type: 'number' },
          max_output_tokens: { type: 'number' },
        },
        required: ['messages'],
      },
    },
  ],
}));

server.setRequestHandler(CallToolRequestSchema, async (request) => {
  const { name, arguments: args } = request.params;

  if (name === 'gemini_config_status') {
    return textResult(configStatus());
  }

  try {
    const client = createGeminiClient();

    if (name === 'gemini_generate') {
      const parsed = z
        .object({
          prompt: z.string(),
          system_instruction: z.string().optional(),
          model: z.string().optional(),
          temperature: z.number().min(0).max(2).optional(),
          max_output_tokens: z.number().positive().optional(),
        })
        .parse(args ?? {});

      return textResult(
        await client.generate({
          prompt: parsed.prompt,
          systemInstruction: parsed.system_instruction,
          model: parsed.model,
          temperature: parsed.temperature,
          maxOutputTokens: parsed.max_output_tokens,
        }),
      );
    }

    if (name === 'gemini_chat') {
      const parsed = z
        .object({
          messages: z.array(
            z.object({
              role: z.enum(['user', 'assistant']),
              text: z.string(),
            }),
          ),
          system_instruction: z.string().optional(),
          model: z.string().optional(),
          temperature: z.number().optional(),
          max_output_tokens: z.number().positive().optional(),
        })
        .parse(args ?? {});

      if (parsed.messages.length === 0) {
        return textResult({ error: 'messages must not be empty' });
      }

      return textResult(
        await client.chat({
          messages: parsed.messages,
          systemInstruction: parsed.system_instruction,
          model: parsed.model,
          temperature: parsed.temperature,
          maxOutputTokens: parsed.max_output_tokens,
        }),
      );
    }

    return { content: [{ type: 'text', text: `Unknown tool: ${name}` }], isError: true };
  } catch (err) {
    if (err.message?.includes('GEMINI_API_KEY')) {
      return textResult({ error: err.message, ...configStatus() });
    }
    return {
      content: [{ type: 'text', text: err.message || String(err) }],
      isError: true,
    };
  }
});

const transport = new StdioServerTransport();
await server.connect(transport);
console.error('push-gemini MCP server running on stdio');
