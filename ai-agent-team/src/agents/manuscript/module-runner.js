import { callManuscriptAgent } from './base.js';
import { getModule } from './module-registry.js';
import { buildFullPrompt, loadMetaPromptGenerator } from './prompt-loader.js';
import {
  compactState,
  markChapterComplete,
  recordCompaction,
  shouldCompact,
  updateState,
} from './state.js';

/**
 * Run a single Manuscript Master prompt module.
 */
export async function runModule(moduleId, variables = {}, state = null) {
  const mod = getModule(moduleId);
  const missing = mod.variables.filter((v) => !variables[v] && variables[v] !== '');
  if (missing.length > 0) {
    throw new Error(
      `Module ${moduleId} requires variables: ${missing.join(', ')}`
    );
  }

  const stateContext = state ? compactState(state) : null;
  const systemPrompt = await buildFullPrompt(mod.key, variables, stateContext);

  const { content, usage } = await callManuscriptAgent({
    systemPrompt,
    userMessage: `Execute Module ${mod.key}: ${mod.name}. Follow the output_format exactly.`,
    temperature: mod.temperature,
    maxOutputTokens: mod.maxOutputTokens,
  });

  return {
    moduleId: mod.key,
    moduleName: mod.name,
    content,
    usage,
    stateSnapshot: stateContext,
  };
}

/**
 * Run the adversarial critique loop: Generate → Audit → (optional polish pass).
 */
export async function runWithAudit(moduleId, variables, state, draftContent) {
  const draft = await runModule(moduleId, variables, state);

  const auditResult = await runModule('14', {
    chapter_text: draftContent ?? draft.content,
    target_avatar: variables.target_audience ?? variables.target_avatar ?? state?.target_persona ?? '',
  }, state);

  return {
    draft,
    audit: auditResult,
  };
}

/**
 * Run the per-chapter drafting loop: 06 → 04 → 03 → 07 → 09 → 14
 */
export async function runChapterLoop(chapterConfig, state) {
  const {
    chapter_number,
    chapter_title,
    chapter_objectives,
    chapter_topic,
    chapter_mechanism,
    target_audience,
    voice_and_tone = 'Direct, authoritative, practitioner-led, conversational but incisive',
    target_industry = '',
    upcoming_chapter_theme = '',
  } = chapterConfig;

  const results = {};

  results.research = await runModule('06', {
    chapter_topic: chapter_topic ?? chapter_objectives,
    target_industry: target_industry || target_audience,
  }, state);

  results.stories = await runModule('04', {
    target_concept: chapter_mechanism ?? chapter_objectives,
    target_audience,
  }, state);

  results.draft = await runModule('03', {
    chapter_number: String(chapter_number),
    chapter_title,
    target_audience,
    chapter_objectives: `${chapter_objectives}\n\nResearch context:\n${results.research.content.slice(0, 2000)}`,
    voice_and_tone,
  }, state);

  results.exercises = await runModule('07', {
    chapter_mechanism: chapter_mechanism ?? chapter_title,
    reader_avatar: target_audience,
  }, state);

  const draftEnding = results.draft.content.slice(-800);
  results.bridge = await runModule('09', {
    ending_chapter_content: draftEnding,
    upcoming_chapter_theme,
  }, state);

  results.audit = await runModule('14', {
    chapter_text: results.draft.content,
    target_avatar: target_audience,
  }, state);

  let updatedState = markChapterComplete(state, chapter_number, chapter_title);

  if (shouldCompact(updatedState)) {
    updatedState = recordCompaction(updatedState);
  }

  return { results, state: updatedState };
}

/**
 * Generate a bespoke prompt module from a raw task description.
 */
export async function runMetaPromptGenerator(rawUserTask) {
  const systemPrompt = await loadMetaPromptGenerator({ raw_user_task: rawUserTask });

  const { content, usage } = await callManuscriptAgent({
    systemPrompt,
    userMessage: `Forge a battle-tested XML prompt module for this task:\n\n${rawUserTask}`,
    temperature: 0.30,
    maxOutputTokens: 3000,
  });

  return { content, usage };
}
