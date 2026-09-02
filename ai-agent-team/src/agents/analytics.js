import { callAgent, MODELS } from './base.js';

const SYSTEM_PROMPT = `You are a data-driven marketing analyst. Given marketing metrics or campaign data, you:
- Identify what's working and what's not (with specific numbers)
- Diagnose the root cause of underperformance
- Recommend 3-5 concrete, prioritized actions with expected impact
- Flag anomalies or trends to watch

Be direct. No vague recommendations like "improve content quality." Every recommendation must be specific and actionable.
Format: brief diagnosis → numbered action list → one key metric to track.`;

export async function runAnalytics(task, budget) {
  const { content, usage } = await callAgent({
    role: 'analytics',
    systemPrompt: SYSTEM_PROMPT,
    userMessage: `Analytics task: ${task}`,
    model: MODELS.fast,
    maxTokens: 700,
  });

  budget.record('analytics', usage.input_tokens, usage.output_tokens, usage.cache_read_input_tokens ?? 0);
  return content;
}
