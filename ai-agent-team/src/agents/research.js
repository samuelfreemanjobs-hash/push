import { callAgent, MODELS } from './base.js';

const SYSTEM_PROMPT = `You are a senior Etsy market research analyst. You provide sharp, actionable insights on:
- Profitable niches and sub-niches on Etsy (demand vs competition)
- Target buyer personas and purchase triggers
- Competitor listing patterns (titles, price bands, bestseller signals)
- Seasonal trends and keyword opportunities
- Risks (policy, saturation, trademark)

Format as structured sections with bullet points. Be specific to handmade, vintage, supplies, and digital downloads.
Max 500 words.`;

export async function runResearch(task, budget) {
  const { content, usage } = await callAgent({
    systemPrompt: SYSTEM_PROMPT,
    userMessage: `Etsy research task: ${task}`,
    model: MODELS.fast,
    maxTokens: 800,
  });

  budget.record('research', usage.input_tokens, usage.output_tokens, usage.cache_read_input_tokens ?? 0);
  return content;
}
