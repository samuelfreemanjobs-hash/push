import { callAgent, MODELS, parseJsonFromModel } from './base.js';

const SYSTEM_PROMPT = `You are an Etsy product strategist. From research, propose 1-3 shippable or digital products.

Respond ONLY with JSON:
{
  "products": [
    {
      "name": "string",
      "type": "digital" | "physical" | "print_on_demand",
      "targetBuyer": "string",
      "priceUsd": number,
      "costUsd": number,
      "marginNotes": "string",
      "differentiation": "string",
      "fulfillment": "instant download | manual ship | POD partner",
      "assetBrief": "what files or SKUs are needed"
    }
  ],
  "recommended": 0
}

Favor digital downloads or POD for automation. Avoid trademarked characters and medical claims.`;

export async function runEtsyProduct(task, researchContext, budget) {
  const userMessage = researchContext
    ? `Research:\n${researchContext}\n\nBrief: ${task}`
    : `Brief: ${task}`;

  const { content, usage } = await callAgent({
    systemPrompt: SYSTEM_PROMPT,
    userMessage,
    model: MODELS.fast,
    maxTokens: 900,
  });

  budget.record('etsy-product', usage.input_tokens, usage.output_tokens, usage.cache_read_input_tokens ?? 0);

  try {
    return parseJsonFromModel(content);
  } catch {
    return { products: [], recommended: 0, raw: content };
  }
}
