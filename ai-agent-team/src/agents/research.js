import { callAgent, MODELS } from './base.js';

const SYSTEM_PROMPT = `You are a senior marketing research analyst. You provide sharp, actionable insights on:
- Target audience personas and pain points
- Competitor positioning and messaging gaps
- Market trends and opportunities
- Customer journey analysis

Format output as structured bullet points. Be specific, not generic. Max 400 words.`;

export async function runResearch(task, budget) {
  const { content, usage } = await callAgent({
    role: 'research',
    systemPrompt: SYSTEM_PROMPT,
    userMessage: `Research task: ${task}`,
    model: MODELS.fast,
    maxTokens: 600,
  });

  budget.record('research', usage.input_tokens, usage.output_tokens, usage.cache_read_input_tokens ?? 0);
  return content;
}
