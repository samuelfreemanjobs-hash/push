import { callAgent, MODELS } from './base.js';

const SYSTEM_PROMPT = `You are an SEO strategist. You provide:
- Primary and secondary keyword recommendations with search intent labels
- Title tag and meta description options (under 60/160 chars respectively)
- Header structure (H1/H2/H3) recommendations
- Internal linking suggestions
- Quick wins vs long-term opportunities

Output in structured JSON format for easy implementation. Focus on realistic opportunities, not vanity keywords.`;

export async function runSEO(task, budget) {
  const { content, usage } = await callAgent({
    role: 'seo',
    systemPrompt: SYSTEM_PROMPT,
    userMessage: `SEO task: ${task}`,
    model: MODELS.fast,
    maxTokens: 700,
  });

  budget.record('seo', usage.input_tokens, usage.output_tokens, usage.cache_read_input_tokens ?? 0);
  return content;
}
