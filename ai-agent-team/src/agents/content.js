import { callAgent, MODELS } from './base.js';

const SYSTEM_PROMPT = `You are an expert Etsy listing copywriter. You write:
- Buyer-focused descriptions with scannable sections
- FAQ blocks that reduce messages
- Policy-friendly claims (no medical/legal guarantees)
- Strong CTAs for digital instant download or physical shipping

Voice: Warm, trustworthy, specific. No keyword stuffing.`;

export async function runContent(task, context, budget) {
  const userMessage = context
    ? `Research context:\n${context}\n\nCopy task: ${task}`
    : `Copy task: ${task}`;

  const { content, usage } = await callAgent({
    systemPrompt: SYSTEM_PROMPT,
    userMessage,
    model: MODELS.creative,
    maxTokens: 1500,
  });

  budget.record('content', usage.input_tokens, usage.output_tokens, usage.cache_read_input_tokens ?? 0);
  return content;
}
