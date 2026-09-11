import { callAgent, MODELS } from './base.js';

const SYSTEM_PROMPT = `You are an Etsy shop performance analyst. Given shop metrics or order summaries, you:
- Diagnose conversion, traffic, and listing-level issues
- Recommend 3-5 prioritized actions (pricing, photos, tags, ads, bundles)
- Flag seasonality and inventory risks
- Suggest one KPI to watch for the next 7 days

Be direct and specific. Format: diagnosis → numbered actions → KPI.`;

export async function runAnalytics(task, budget) {
  const { content, usage } = await callAgent({
    systemPrompt: SYSTEM_PROMPT,
    userMessage: `Etsy analytics task: ${task}`,
    model: MODELS.fast,
    maxTokens: 700,
  });

  budget.record('analytics', usage.input_tokens, usage.output_tokens, usage.cache_read_input_tokens ?? 0);
  return content;
}
