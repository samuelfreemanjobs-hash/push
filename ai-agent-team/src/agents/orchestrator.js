import { callAgent, MODELS } from './base.js';

const SYSTEM_PROMPT = `You are a marketing team orchestrator. Your ONLY job is to analyze a marketing request and output a JSON routing decision.

Available agents:
- research: market research, competitor analysis, audience insights, trends
- content: blog posts, email copy, landing page copy, ad copy, press releases
- seo: keyword research, meta descriptions, SEO audits, on-page optimization
- social: social media posts, captions, hashtag strategy, platform-specific content
- analytics: interpret marketing metrics, campaign performance, A/B test results, recommendations

Respond ONLY with this JSON structure (no other text):
{
  "agents": ["agent1", "agent2"],
  "plan": "one sentence describing what each agent will do",
  "priority": "agent1"
}

Choose the minimum set of agents needed. Never include an agent that isn't necessary.`;

export async function orchestrate(task, budget) {
  if (!budget.hasCapacity(500)) {
    throw new Error('Insufficient token budget for orchestration');
  }

  const { content, usage } = await callAgent({
    role: 'orchestrator',
    systemPrompt: SYSTEM_PROMPT,
    userMessage: task,
    model: MODELS.fast,
    maxTokens: 300,
  });

  budget.record('orchestrator', usage.input_tokens, usage.output_tokens, usage.cache_read_input_tokens ?? 0);

  try {
    return JSON.parse(content);
  } catch {
    // Fallback if model doesn't produce valid JSON
    return { agents: ['content'], plan: 'Generate marketing content', priority: 'content' };
  }
}
