import Anthropic from '@anthropic-ai/sdk';

const client = new Anthropic({ apiKey: process.env.ANTHROPIC_API_KEY });

// Models tiered by cost/capability
export const MODELS = {
  fast: 'claude-haiku-4-5-20251001',   // routing, classification, structured analysis
  creative: 'claude-sonnet-5',          // content creation, social copy
};

/**
 * Call a Claude agent with prompt caching on the system prompt.
 * Returns { content, usage } where usage tracks cache hits.
 */
export async function callAgent({ role, systemPrompt, userMessage, model, maxTokens = 1000 }) {
  const response = await client.messages.create({
    model,
    max_tokens: maxTokens,
    system: [
      {
        type: 'text',
        text: systemPrompt,
        cache_control: { type: 'ephemeral' }, // cache the system prompt — saves ~90% on repeated calls
      },
    ],
    messages: [{ role: 'user', content: userMessage }],
  });

  const content = response.content[0]?.text ?? '';
  const usage = response.usage;

  return { content, usage };
}
