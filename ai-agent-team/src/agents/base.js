import { GoogleGenerativeAI } from '@google/generative-ai';

export const MODELS = {
  fast: process.env.GEMINI_FAST_MODEL || 'gemini-2.0-flash',
  creative: process.env.GEMINI_CREATIVE_MODEL || 'gemini-2.0-flash',
};

function estimateTokens(text) {
  return Math.ceil((text?.length ?? 0) / 4);
}

/**
 * Call Gemini with a cached-style system instruction.
 * Returns { content, usage } for token budget tracking.
 */
export async function callAgent({
  systemPrompt,
  userMessage,
  model,
  maxTokens = 1000,
}) {
  if (!process.env.GEMINI_API_KEY) {
    throw new Error('GEMINI_API_KEY is required for agent calls');
  }

  const genAI = new GoogleGenerativeAI(process.env.GEMINI_API_KEY);
  const generativeModel = genAI.getGenerativeModel({
    model,
    systemInstruction: systemPrompt,
    generationConfig: { maxOutputTokens: maxTokens },
  });

  const result = await generativeModel.generateContent(userMessage);
  const content = result.response.text();

  const usage = {
    input_tokens: estimateTokens(systemPrompt) + estimateTokens(userMessage),
    output_tokens: estimateTokens(content),
    cache_read_input_tokens: 0,
  };

  return { content, usage };
}

/**
 * Extract JSON from model output (handles markdown fences).
 */
export function parseJsonFromModel(text) {
  const trimmed = text.trim();
  const fenceMatch = trimmed.match(/```(?:json)?\s*([\s\S]*?)```/);
  const raw = fenceMatch ? fenceMatch[1].trim() : trimmed;
  return JSON.parse(raw);
}
