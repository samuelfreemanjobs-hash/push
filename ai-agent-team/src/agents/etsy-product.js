import { callAgent, MODELS, parseJsonFromModel } from './base.js';

const SYSTEM_PROMPT = `You are an Etsy product strategist for B2B digital downloads. Buyers want OUTCOMES: working systems, not random prompts.

Product types you may propose:
- Excel/Google Sheets templates (dashboards, KPI trackers, financial models)
- Notion-style or markdown SOP packs (deliver as PDF + CSV + instructions)
- Prompt + workflow libraries positioned as "AI agent kits" for one job (e.g. client onboarding, weekly marketing)
- Mini PWA / HTML dashboard starters (zip with README)
- Postcard / direct-mail design templates (PDF + Canva link instructions)
- "Business in a box" bundles (3-5 files with implementation checklist)

Respond ONLY with JSON:
{
  "products": [
    {
      "name": "string",
      "type": "digital",
      "outcomePromise": "one sentence measurable outcome",
      "targetBuyer": "string",
      "priceUsd": number,
      "costUsd": number,
      "marginNotes": "string",
      "differentiation": "string",
      "fulfillment": "instant download",
      "assetBrief": "exact files to include",
      "etsySearchAngle": "primary keyword phrase"
    }
  ],
  "recommended": 0
}

Price digital kits $9.99-$79.99 unless enterprise bundle. No medical/legal guarantees. Do not use trademarked app names in product names.`;

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
