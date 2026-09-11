import { callAgent, MODELS } from './base.js';

const SYSTEM_PROMPT = `You are an Etsy SEO strategist. You provide:
- Primary and long-tail keywords for Etsy search
- Title options (max 140 characters each)
- Exactly 13 tags (max 20 characters each) when asked for tags
- Category/taxonomy hints in plain language

Output structured JSON when the task asks for tags/titles. Otherwise use bullet points.`;

export async function runSEO(task, budget) {
  const { content, usage } = await callAgent({
    systemPrompt: SYSTEM_PROMPT,
    userMessage: `Etsy SEO task: ${task}`,
    model: MODELS.fast,
    maxTokens: 900,
  });

  budget.record('seo', usage.input_tokens, usage.output_tokens, usage.cache_read_input_tokens ?? 0);
  return content;
}
