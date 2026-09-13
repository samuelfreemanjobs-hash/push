/**
 * Minimal Linear GraphQL client.
 * @see https://linear.app/developers/graphql
 *
 * Auth: personal API key → Authorization: <key> (no Bearer)
 *       OAuth token     → Authorization: Bearer <token>
 */

const GRAPHQL_URL = 'https://api.linear.app/graphql';

export class LinearApiError extends Error {
  constructor(message, details) {
    super(message);
    this.details = details;
  }
}

export function createLinearClientFromEnv() {
  const apiKey = process.env.LINEAR_API_KEY?.trim();
  const oauthToken = process.env.LINEAR_ACCESS_TOKEN?.trim();

  if (!apiKey && !oauthToken) {
    throw new Error('Set LINEAR_API_KEY (personal) or LINEAR_ACCESS_TOKEN (OAuth)');
  }

  const headers = {
    'Content-Type': 'application/json',
  };

  if (apiKey) {
    headers.Authorization = apiKey;
  } else {
    headers.Authorization = `Bearer ${oauthToken}`;
  }

  return {
    defaultTeamId: process.env.LINEAR_DEFAULT_TEAM_ID?.trim() || null,

    async graphql(query, variables) {
      const res = await fetch(GRAPHQL_URL, {
        method: 'POST',
        headers,
        body: JSON.stringify({ query, variables }),
      });

      const text = await res.text();
      let payload;
      try {
        payload = text ? JSON.parse(text) : {};
      } catch {
        payload = { raw: text };
      }

      if (!res.ok) {
        throw new LinearApiError(`Linear HTTP ${res.status}`, payload);
      }
      if (payload.errors?.length) {
        throw new LinearApiError(payload.errors.map((e) => e.message).join('; '), payload);
      }
      return payload.data;
    },

    async viewer() {
      const data = await this.graphql(`query { viewer { id name email } }`);
      return data.viewer;
    },

    async listTeams(first = 50) {
      const data = await this.graphql(
        `query Teams($first: Int!) {
          teams(first: $first) {
            nodes { id key name }
          }
        }`,
        { first },
      );
      return data.teams.nodes;
    },

    async listProjects(teamId, first = 50) {
      const data = await this.graphql(
        `query Projects($teamId: String!, $first: Int!) {
          team(id: $teamId) {
            projects(first: $first) { nodes { id name slugId state } }
          }
        }`,
        { teamId, first },
      );
      return data.team?.projects?.nodes ?? [];
    },

    async findProjectByName(name, teamId) {
      const teams = teamId ? [{ id: teamId }] : await this.listTeams();
      const needle = name.toLowerCase();
      for (const team of teams) {
        const projects = await this.listProjects(team.id);
        const hit = projects.find((p) => p.name.toLowerCase() === needle);
        if (hit) return { ...hit, teamId: team.id };
      }
      return null;
    },

    async listIssues({ teamId, projectId, first = 25, after } = {}) {
      const filter = {};
      if (teamId) filter.team = { id: { eq: teamId } };
      if (projectId) filter.project = { id: { eq: projectId } };

      const data = await this.graphql(
        `query Issues($first: Int!, $after: String, $filter: IssueFilter) {
          issues(first: $first, after: $after, filter: $filter) {
            nodes {
              id identifier title url priority
              state { id name type }
              assignee { id name }
              project { id name }
            }
            pageInfo { hasNextPage endCursor }
          }
        }`,
        { first, after: after ?? null, filter: Object.keys(filter).length ? filter : undefined },
      );
      return data.issues;
    },

    async getIssue(idOrIdentifier) {
      const data = await this.graphql(
        `query Issue($id: String!) {
          issue(id: $id) {
            id identifier title description url priority
            state { id name type }
            assignee { id name }
            project { id name }
            team { id key name }
          }
        }`,
        { id: idOrIdentifier },
      );
      return data.issue;
    },

    async createIssue({ teamId, title, description, priority, projectId, stateId }) {
      const input = { teamId, title };
      if (description) input.description = description;
      if (priority != null) input.priority = priority;
      if (projectId) input.projectId = projectId;
      if (stateId) input.stateId = stateId;

      const data = await this.graphql(
        `mutation Create($input: IssueCreateInput!) {
          issueCreate(input: $input) {
            success
            issue { id identifier title url }
          }
        }`,
        { input },
      );
      return data.issueCreate;
    },

    async updateIssue(issueId, input) {
      const data = await this.graphql(
        `mutation Update($id: String!, $input: IssueUpdateInput!) {
          issueUpdate(id: $id, input: $input) {
            success
            issue { id identifier title url state { name } }
          }
        }`,
        { id: issueId, input },
      );
      return data.issueUpdate;
    },

    async createComment(issueId, body) {
      const data = await this.graphql(
        `mutation Comment($input: CommentCreateInput!) {
          commentCreate(input: $input) {
            success
            comment { id body url }
          }
        }`,
        { input: { issueId, body } },
      );
      return data.commentCreate;
    },
  };
}
