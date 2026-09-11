import { callAgent, MODELS, parseJsonFromModel } from './base.js';

const SYSTEM_PROMPT = `You create Etsy-ready listings for business digital downloads (templates, AI agent kits, marketing systems).

Lead titles with the OUTCOME (e.g. "Client Onboarding System" not "ChatGPT prompts").
Description structure: who it's for → what's included (bullet list) → how to use in 15 min → license/usage → FAQ.

Respond ONLY with JSON:

{
  "title": "max 140 chars",
  "tags": ["exactly 13 tags, each max 20 chars"],
  "description": "markdown ok, buyer-focused",
  "priceUsd": number,
  "quantity": number,
  "type": "download",
  "who_made": "i_did",
  "when_made": "2020_2025",
  "is_supply": false,
  "materials": ["Digital file"],
  "personalization": "optional instructions or null",
  "imagePrompts": ["3-5 prompts for mockups: laptop dashboard, postcard mailer, workflow diagram"]
}

Follow Etsy policies. Tags = real Etsy searches (small business template, ai prompt kit, marketing planner, etc.).`;

export async function runEtsyListing(task, productContext, budget) {
  const userMessage = productContext
    ? `Product plan:\n${JSON.stringify(productContext, null, 2)}\n\nListing brief: ${task}`
    : `Listing brief: ${task}`;

  const { content, usage } = await callAgent({
    systemPrompt: SYSTEM_PROMPT,
    userMessage,
    model: MODELS.creative,
    maxTokens: 2000,
  });

  budget.record('etsy-listing', usage.input_tokens, usage.output_tokens, usage.cache_read_input_tokens ?? 0);

  try {
    return parseJsonFromModel(content);
  } catch {
    return { raw: content, parseError: true };
  }
}
