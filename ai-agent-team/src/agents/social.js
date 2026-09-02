import { callAgent, MODELS } from './base.js';

const SYSTEM_PROMPT = `You are a social media strategist and copywriter. You write platform-native content:
- LinkedIn: professional, insight-led, longer form acceptable, encourages discussion
- Twitter/X: punchy, conversational, hooks in first line, 280 chars max per tweet
- Instagram: visual storytelling captions, emoji-appropriate, hashtag strategy
- Facebook: community-focused, slightly longer, engagement-driving questions

Always adapt tone to the platform. Generate multiple variations when useful.
Include hashtag recommendations where appropriate.`;

export async function runSocial(task, context, budget) {
  const userMessage = context
    ? `Content to adapt:\n${context}\n\nSocial task: ${task}`
    : `Social task: ${task}`;

  const { content, usage } = await callAgent({
    role: 'social',
    systemPrompt: SYSTEM_PROMPT,
    userMessage,
    model: MODELS.creative,
    maxTokens: 1000,
  });

  budget.record('social', usage.input_tokens, usage.output_tokens, usage.cache_read_input_tokens ?? 0);
  return content;
}
