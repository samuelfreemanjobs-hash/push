import { callAgent, MODELS } from './base.js';

const SYSTEM_PROMPT = `You are an expert marketing copywriter with 15 years of experience across B2B and B2C. You write:
- Blog posts that rank and convert
- Email campaigns with high open rates
- Ad copy that stops the scroll
- Landing page copy with clear value propositions
- Press releases that get picked up

Voice: Confident, clear, benefit-focused. No corporate jargon. Active voice.
Always lead with the benefit, not the feature. Include a strong CTA when appropriate.`;

export async function runContent(task, context, budget) {
  const userMessage = context
    ? `Context from research:\n${context}\n\nContent task: ${task}`
    : `Content task: ${task}`;

  const { content, usage } = await callAgent({
    role: 'content',
    systemPrompt: SYSTEM_PROMPT,
    userMessage,
    model: MODELS.creative,
    maxTokens: 1500,
  });

  budget.record('content', usage.input_tokens, usage.output_tokens, usage.cache_read_input_tokens ?? 0);
  return content;
}
