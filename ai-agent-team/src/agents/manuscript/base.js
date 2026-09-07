import { GoogleGenerativeAI } from '@google/generative-ai';

const DEFAULT_MODEL = process.env.GEMINI_MODEL ?? 'gemini-2.0-flash';

let genAI;

function getClient() {
  if (!process.env.GEMINI_API_KEY) {
    throw new Error('GEMINI_API_KEY is required for Manuscript Master');
  }
  if (!genAI) {
    genAI = new GoogleGenerativeAI(process.env.GEMINI_API_KEY);
  }
  return genAI;
}

/**
 * Execute a Manuscript Master prompt with temperature and token controls.
 */
export async function callManuscriptAgent({
  systemPrompt,
  userMessage,
  temperature = 0.55,
  maxOutputTokens = 4000,
}) {
  const client = getClient();
  const model = client.getGenerativeModel({
    model: DEFAULT_MODEL,
    generationConfig: {
      temperature,
      maxOutputTokens,
    },
  });

  const fullPrompt = `${systemPrompt}\n\n---\n\n<user_request>\n${userMessage}\n</user_request>`;

  const result = await model.generateContent(fullPrompt);
  const content = result.response.text();

  const usage = {
    model: DEFAULT_MODEL,
    temperature,
    maxOutputTokens,
    promptLength: fullPrompt.length,
    responseLength: content.length,
  };

  return { content, usage };
}
