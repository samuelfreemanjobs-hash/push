#!/usr/bin/env node
import { Server } from '@modelcontextprotocol/sdk/server/index.js';
import { StdioServerTransport } from '@modelcontextprotocol/sdk/server/stdio.js';
import {
  CallToolRequestSchema,
  ListToolsRequestSchema,
} from '@modelcontextprotocol/sdk/types.js';
import { z } from 'zod';
import { createLinearClientFromEnv } from './lib/linear-client.js';

function textResult(obj) {
  return {
    content: [{ type: 'text', text: typeof obj === 'string' ? obj : JSON.stringify(obj, null, 2) }],
  };
}

function configStatus() {
  return {
    LINEAR_API_KEY: Boolean(process.env.LINEAR_API_KEY),
    LINEAR_ACCESS_TOKEN: Boolean(process.env.LINEAR_ACCESS_TOKEN),
    LINEAR_DEFAULT_TEAM_ID: Boolean(process.env.LINEAR_DEFAULT_TEAM_ID),
    auth_mode: process.env.LINEAR_API_KEY ? 'personal_api_key' : process.env.LINEAR_ACCESS_TOKEN ? 'oauth' : 'none',
    docs: 'https://linear.app/developers/graphql',
    pm_project_hint: 'PM Product Matrix Operations',
  };
}

function clientOrError() {
  try {
    return { client: createLinearClientFromEnv() };
  } catch (err) {
    return { error: err.message, status: configStatus() };
  }
}

const server = new Server(
  { name: 'push-linear', version: '1.0.0' },
  { capabilities: { tools: {} } },
);

server.setRequestHandler(ListToolsRequestSchema, async () => ({
  tools: [
    {
      name: 'linear_config_status',
      description: 'Check Linear auth env vars (no secrets returned).',
      inputSchema: { type: 'object', properties: {} },
    },
    {
      name: 'linear_viewer',
      description: 'Get authenticated Linear user (viewer).',
      inputSchema: { type: 'object', properties: {} },
    },
    {
      name: 'linear_list_teams',
      description: 'List teams you can access.',
      inputSchema: {
        type: 'object',
        properties: { first: { type: 'number', default: 50 } },
      },
    },
    {
      name: 'linear_list_projects',
      description: 'List projects for a team.',
      inputSchema: {
        type: 'object',
        properties: {
          team_id: { type: 'string' },
          first: { type: 'number', default: 50 },
        },
        required: ['team_id'],
      },
    },
    {
      name: 'linear_find_project',
      description: 'Find a project by exact name (searches all teams unless team_id set).',
      inputSchema: {
        type: 'object',
        properties: {
          name: { type: 'string' },
          team_id: { type: 'string' },
        },
        required: ['name'],
      },
    },
    {
      name: 'linear_list_issues',
      description: 'List issues filtered by team and/or project.',
      inputSchema: {
        type: 'object',
        properties: {
          team_id: { type: 'string' },
          project_id: { type: 'string' },
          first: { type: 'number', default: 25 },
          after: { type: 'string', description: 'Pagination cursor' },
        },
      },
    },
    {
      name: 'linear_get_issue',
      description: 'Get one issue by UUID or identifier (e.g. ENG-123).',
      inputSchema: {
        type: 'object',
        properties: { id: { type: 'string' } },
        required: ['id'],
      },
    },
    {
      name: 'linear_create_issue',
      description:
        'Create an issue (write). Confirm with user first. Uses LINEAR_DEFAULT_TEAM_ID if team_id omitted.',
      inputSchema: {
        type: 'object',
        properties: {
          team_id: { type: 'string' },
          title: { type: 'string' },
          description: { type: 'string' },
          priority: { type: 'number', description: '0=none, 1=urgent, 2=high, 3=normal, 4=low' },
          project_id: { type: 'string' },
          state_id: { type: 'string' },
        },
        required: ['title'],
      },
    },
    {
      name: 'linear_update_issue',
      description: 'Update an issue (write). Confirm with user first.',
      inputSchema: {
        type: 'object',
        properties: {
          issue_id: { type: 'string' },
          title: { type: 'string' },
          description: { type: 'string' },
          priority: { type: 'number' },
          state_id: { type: 'string' },
          project_id: { type: 'string' },
        },
        required: ['issue_id'],
      },
    },
    {
      name: 'linear_create_comment',
      description: 'Add a comment to an issue (write). Confirm with user first.',
      inputSchema: {
        type: 'object',
        properties: {
          issue_id: { type: 'string' },
          body: { type: 'string' },
        },
        required: ['issue_id', 'body'],
      },
    },
    {
      name: 'linear_graphql',
      description:
        'Run a raw GraphQL query or mutation against Linear. Use for advanced cases only.',
      inputSchema: {
        type: 'object',
        properties: {
          query: { type: 'string' },
          variables: { type: 'object' },
        },
        required: ['query'],
      },
    },
  ],
}));

server.setRequestHandler(CallToolRequestSchema, async (request) => {
  const { name, arguments: args } = request.params;

  if (name === 'linear_config_status') {
    return textResult(configStatus());
  }

  const { client, error, status } = clientOrError();
  if (!client) {
    return textResult({ error, ...status });
  }

  try {
    if (name === 'linear_viewer') {
      return textResult(await client.viewer());
    }

    if (name === 'linear_list_teams') {
      const { first } = z.object({ first: z.number().default(50) }).parse(args ?? {});
      return textResult(await client.listTeams(first));
    }

    if (name === 'linear_list_projects') {
      const parsed = z
        .object({ team_id: z.string(), first: z.number().default(50) })
        .parse(args ?? {});
      return textResult(await client.listProjects(parsed.team_id, parsed.first));
    }

    if (name === 'linear_find_project') {
      const parsed = z
        .object({ name: z.string(), team_id: z.string().optional() })
        .parse(args ?? {});
      return textResult(await client.findProjectByName(parsed.name, parsed.team_id));
    }

    if (name === 'linear_list_issues') {
      const parsed = z
        .object({
          team_id: z.string().optional(),
          project_id: z.string().optional(),
          first: z.number().default(25),
          after: z.string().optional(),
        })
        .parse(args ?? {});
      return textResult(
        await client.listIssues({
          teamId: parsed.team_id,
          projectId: parsed.project_id,
          first: parsed.first,
          after: parsed.after,
        }),
      );
    }

    if (name === 'linear_get_issue') {
      const { id } = z.object({ id: z.string() }).parse(args ?? {});
      return textResult(await client.getIssue(id));
    }

    if (name === 'linear_create_issue') {
      const parsed = z
        .object({
          team_id: z.string().optional(),
          title: z.string(),
          description: z.string().optional(),
          priority: z.number().optional(),
          project_id: z.string().optional(),
          state_id: z.string().optional(),
        })
        .parse(args ?? {});

      const teamId = parsed.team_id || client.defaultTeamId;
      if (!teamId) {
        return textResult({
          error: 'team_id required (or set LINEAR_DEFAULT_TEAM_ID)',
        });
      }

      return textResult(
        await client.createIssue({
          teamId,
          title: parsed.title,
          description: parsed.description,
          priority: parsed.priority,
          projectId: parsed.project_id,
          stateId: parsed.state_id,
        }),
      );
    }

    if (name === 'linear_update_issue') {
      const parsed = z
        .object({
          issue_id: z.string(),
          title: z.string().optional(),
          description: z.string().optional(),
          priority: z.number().optional(),
          state_id: z.string().optional(),
          project_id: z.string().optional(),
        })
        .parse(args ?? {});

      const input = {};
      if (parsed.title) input.title = parsed.title;
      if (parsed.description) input.description = parsed.description;
      if (parsed.priority != null) input.priority = parsed.priority;
      if (parsed.state_id) input.stateId = parsed.state_id;
      if (parsed.project_id) input.projectId = parsed.project_id;

      return textResult(await client.updateIssue(parsed.issue_id, input));
    }

    if (name === 'linear_create_comment') {
      const parsed = z
        .object({ issue_id: z.string(), body: z.string() })
        .parse(args ?? {});
      return textResult(await client.createComment(parsed.issue_id, parsed.body));
    }

    if (name === 'linear_graphql') {
      const parsed = z
        .object({
          query: z.string(),
          variables: z.record(z.unknown()).optional(),
        })
        .parse(args ?? {});
      return textResult(await client.graphql(parsed.query, parsed.variables));
    }

    return { content: [{ type: 'text', text: `Unknown tool: ${name}` }], isError: true };
  } catch (err) {
    return {
      content: [
        {
          type: 'text',
          text: JSON.stringify(
            { message: err.message, details: err.details },
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
console.error('push-linear MCP server running on stdio');
