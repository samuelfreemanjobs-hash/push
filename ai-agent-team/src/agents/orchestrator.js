import { callAgent, MODELS, parseJsonFromModel } from './base.js';

const SYSTEM_PROMPT = `You are an Etsy shop automation orchestrator. Analyze the request and output JSON only:

{
  "agents": ["research", "product", "listing", "seo", "social", "analytics"],
  "plan": "one sentence on execution order",
  "publish": true
}

Agent roles:
- research: niche, competition, trends
- product: product concepts and pricing
- listing: full Etsy listing package (structured)
- seo: tags and title refinement
- social: Pinterest/IG/TikTok marketing
- analytics: interpret shop metrics

Include "product" and "listing" for new product workflows. Set publish true only when the user explicitly wants listings created on Etsy. Minimum agents only.`;

export async function orchestrate(task, budget) {
  if (!budget.hasCapacity(500)) {
    throw new Error('Insufficient token budget for orchestration');
  }

  const { content, usage } = await callAgent({
    systemPrompt: SYSTEM_PROMPT,
    userMessage: task,
    model: MODELS.fast,
    maxTokens: 350,
  });

  budget.record('orchestrator', usage.input_tokens, usage.output_tokens, usage.cache_read_input_tokens ?? 0);

  try {
    return parseJsonFromModel(content);
  } catch {
    return {
      agents: ['research', 'product', 'listing', 'seo', 'social'],
      plan: 'Full Etsy launch pipeline',
      publish: false,
    };
  }
}
