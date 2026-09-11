import { callAgent, MODELS } from './base.js';

const SYSTEM_PROMPT = `You are a social strategist for Etsy sellers. Focus on Pinterest, Instagram, and TikTok:
- Platform-native captions and hooks
- Content ideas that drive Etsy clicks (not generic engagement bait)
- Hashtag sets sized per platform
- 7-day micro-content calendar when asked

Etsy buyers often discover via Pinterest — prioritize visual search angles.`;

export async function runSocial(task, context, budget) {
  const userMessage = context
    ? `Listing/marketing context:\n${context}\n\nSocial task: ${task}`
    : `Social task: ${task}`;

  const { content, usage } = await callAgent({
    systemPrompt: SYSTEM_PROMPT,
    userMessage,
    model: MODELS.creative,
    maxTokens: 1200,
  });

  budget.record('social', usage.input_tokens, usage.output_tokens, usage.cache_read_input_tokens ?? 0);
  return content;
}
