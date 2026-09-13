import { GoogleGenerativeAI } from '@google/generative-ai';

export function getGeminiConfig() {
  const apiKey = process.env.GEMINI_API_KEY?.trim();
  const model =
    process.env.GEMINI_MODEL?.trim() ||
    process.env.GOOGLE_GENERATIVE_AI_MODEL?.trim() ||
    'gemini-2.0-flash';
  return { apiKey, model };
}

export function createGeminiClient() {
  const { apiKey, model: defaultModel } = getGeminiConfig();
  if (!apiKey) {
    throw new Error('GEMINI_API_KEY is not set');
  }
  const genAI = new GoogleGenerativeAI(apiKey);

  return {
    defaultModel,

    getModel({ model, systemInstruction, temperature, maxOutputTokens } = {}) {
      const name = model || defaultModel;
      return genAI.getGenerativeModel({
        model: name,
        systemInstruction: systemInstruction || undefined,
        generationConfig: {
          ...(temperature != null ? { temperature } : {}),
          ...(maxOutputTokens != null ? { maxOutputTokens } : {}),
        },
      });
    },

    async generate({ prompt, model, systemInstruction, temperature, maxOutputTokens }) {
      const m = this.getModel({ model, systemInstruction, temperature, maxOutputTokens });
      const result = await m.generateContent(prompt);
      const response = result.response;
      return {
        text: response.text(),
        model: model || defaultModel,
        usageMetadata: response.usageMetadata ?? null,
      };
    },

    async chat({ messages, model, systemInstruction, temperature, maxOutputTokens }) {
      const m = this.getModel({ model, systemInstruction, temperature, maxOutputTokens });
      const chat = m.startChat({
        history: messages.slice(0, -1).map((msg) => ({
          role: msg.role === 'assistant' ? 'model' : 'user',
          parts: [{ text: msg.text }],
        })),
      });
      const last = messages[messages.length - 1];
      if (!last || last.role !== 'user') {
        throw new Error('Last message must be role user with text');
      }
      const result = await chat.sendMessage(last.text);
      const response = result.response;
      return {
        text: response.text(),
        model: model || defaultModel,
        usageMetadata: response.usageMetadata ?? null,
      };
    },
  };
}
