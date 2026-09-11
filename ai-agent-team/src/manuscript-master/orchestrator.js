import { STATE_COMPACTION_INTERVAL, CHAPTER_LOOP_MODULES, PLAYBOOK_STEPS } from './config.js';
import { PROMPT_MODULES } from './prompt-modules.js';

/**
 * Create a fresh manuscript workflow session state.
 */
export function createSession(initialData = {}) {
  const now = new Date().toISOString();
  return {
    sessionId: initialData.sessionId || `mm_${Date.now()}`,
    createdAt: now,
    updatedAt: now,
    book_title: initialData.book_title || '',
    core_promise: initialData.core_promise || '',
    target_persona: initialData.target_audience || initialData.target_persona || '',
    active_chapter: null,
    narrative_threads: [],
    completed_milestones: [],
    currentPlaybookStep: 1,
    currentChapterNumber: 0,
    totalChapters: 0,
    chapterLoopIndex: 0,
    outline: null,
    selectedConcept: null,
    selectedTitle: null,
    kdpKeywords: [],
    outputs: {},
    stateSnapshots: [],
    next_executable_step: PLAYBOOK_STEPS[0].action,
  };
}

/**
 * Advance workflow to the next playbook step after completing a module.
 */
export function advanceWorkflow(session, moduleId, output) {
  session.outputs[moduleId] = output;
  session.updatedAt = new Date().toISOString();
  session.completed_milestones.push(`${moduleId} completed at ${session.updatedAt}`);

  switch (moduleId) {
    case '01_concept_generator':
      session.currentPlaybookStep = 2;
      session.next_executable_step = PLAYBOOK_STEPS[1].action;
      break;
    case '08_title_laboratory':
      session.currentPlaybookStep = 3;
      session.next_executable_step = PLAYBOOK_STEPS[2].action;
      break;
    case '02_outline_builder':
      session.currentPlaybookStep = 4;
      session.outline = output;
      session.totalChapters = extractChapterCount(output);
      session.next_executable_step = PLAYBOOK_STEPS[3].action;
      break;
    case '05_intro_writer':
      session.currentPlaybookStep = 5;
      session.currentChapterNumber = 1;
      session.chapterLoopIndex = 0;
      session.active_chapter = 'Chapter 1';
      session.next_executable_step = getChapterLoopStep(session, 1);
      break;
    case '11_conclusion_writer':
      session.currentPlaybookStep = 7;
      session.next_executable_step = 'Execute Module 12 (Blurb Engine) and Module 13 (Bio Architect)';
      break;
    case '12_blurb_engine':
    case '13_bio_architect':
      if (session.outputs['12_blurb_engine'] && session.outputs['13_bio_architect']) {
        session.next_executable_step = 'Manuscript workflow complete. Ready for KDP upload.';
      }
      break;
    default:
      if (CHAPTER_LOOP_MODULES.includes(moduleId)) {
        handleChapterLoopAdvance(session, moduleId);
      }
      break;
  }

  if (shouldCompactState(session)) {
    session.stateSnapshots.push(compactState(session));
  }

  return session;
}

function handleChapterLoopAdvance(session, moduleId) {
  const loopIndex = CHAPTER_LOOP_MODULES.indexOf(moduleId);
  const isLastInLoop = loopIndex === CHAPTER_LOOP_MODULES.length - 1;

  if (isLastInLoop) {
    const chapterNum = session.currentChapterNumber;
    session.completed_milestones.push(`Chapter ${chapterNum} draft frozen`);

    if (chapterNum >= session.totalChapters) {
      session.currentPlaybookStep = 6;
      session.active_chapter = 'Conclusion';
      session.next_executable_step = PLAYBOOK_STEPS[5].action;
    } else {
      session.currentChapterNumber = chapterNum + 1;
      session.chapterLoopIndex = 0;
      session.active_chapter = `Chapter ${session.currentChapterNumber}`;
      session.next_executable_step = getChapterLoopStep(session, session.currentChapterNumber);
    }
  } else {
    session.chapterLoopIndex = loopIndex + 1;
    session.next_executable_step = getChapterLoopStep(session, session.currentChapterNumber);
  }
}

function getChapterLoopStep(session, chapterNum) {
  const nextModule = CHAPTER_LOOP_MODULES[session.chapterLoopIndex];
  const moduleName = PROMPT_MODULES[nextModule]?.name || nextModule;
  return `Chapter ${chapterNum}: Execute ${moduleName} (Module ${nextModule.replace('_', ' ')})`;
}

function shouldCompactState(session) {
  if (!session.currentChapterNumber) return false;
  return session.currentChapterNumber % STATE_COMPACTION_INTERVAL === 0;
}

/**
 * Output strictly valid JSON state per system directive schema.
 */
export function compactState(session) {
  return {
    book_title: session.book_title,
    core_promise: session.core_promise,
    target_persona: session.target_persona,
    active_chapter: session.active_chapter,
    narrative_threads: session.narrative_threads,
    completed_milestones: session.completed_milestones,
    next_executable_step: session.next_executable_step,
    compactedAt: new Date().toISOString(),
  };
}

/**
 * Resolve which module to run next based on session state.
 */
export function getNextModule(session) {
  const step = session.currentPlaybookStep;

  if (step === 1) return '01_concept_generator';
  if (step === 2) return '08_title_laboratory';
  if (step === 3) return '02_outline_builder';
  if (step === 4) return '05_intro_writer';
  if (step === 5) return CHAPTER_LOOP_MODULES[session.chapterLoopIndex];
  if (step === 6) return '11_conclusion_writer';
  if (step === 7) {
    if (!session.outputs['12_blurb_engine']) return '12_blurb_engine';
    if (!session.outputs['13_bio_architect']) return '13_bio_architect';
    return null;
  }

  return null;
}

/**
 * Build variable map for the next module execution from session context.
 */
export function buildModuleVariables(session, moduleId, overrides = {}) {
  const base = {
    niche_expertise: overrides.niche_expertise || session.niche_expertise || '',
    primary_audience: overrides.primary_audience || session.target_persona || '',
    monetization_goal: overrides.monetization_goal || session.monetization_goal || 'KDP Royalties',
    book_title: overrides.book_title || session.book_title || session.selectedTitle || '',
    core_promise: overrides.core_promise || session.core_promise || '',
    target_audience: overrides.target_audience || session.target_persona || '',
    target_buyer: overrides.target_buyer || session.target_persona || '',
    target_avatar: overrides.target_avatar || session.target_persona || '',
    reader_avatar: overrides.reader_avatar || session.target_persona || '',
    primary_subject: overrides.primary_subject || session.book_title || '',
    primary_keywords: overrides.primary_keywords || session.kdpKeywords?.join(', ') || '',
    macro_transformation: overrides.macro_transformation || session.core_promise || '',
    author_name: overrides.author_name || session.author_name || '',
    author_background: overrides.author_background || session.author_background || '',
    core_credentials: overrides.core_credentials || session.author_background || '',
    personal_philosophy: overrides.personal_philosophy || session.personal_philosophy || '',
    core_enemy: overrides.core_enemy || session.core_enemy || '',
    promised_outcome: overrides.promised_outcome || session.core_promise || '',
    big_benefit: overrides.big_benefit || session.core_promise || '',
    key_bullets: overrides.key_bullets || '',
    chapter_number: String(session.currentChapterNumber || overrides.chapter_number || '1'),
    chapter_title: overrides.chapter_title || session.active_chapter || '',
    chapter_objectives: overrides.chapter_objectives || '',
    voice_and_tone:
      overrides.voice_and_tone ||
      'Direct, authoritative, practitioner-led, conversational but incisive',
    chapter_topic: overrides.chapter_topic || session.active_chapter || '',
    target_industry: overrides.target_industry || session.niche_expertise || '',
    target_concept: overrides.target_concept || session.core_promise || '',
    chapter_mechanism: overrides.chapter_mechanism || '',
    methodology_name: overrides.methodology_name || session.book_title || '',
    desired_outcomes: overrides.desired_outcomes || session.core_promise || '',
    ending_chapter_content: overrides.ending_chapter_content || '',
    upcoming_chapter_theme: overrides.upcoming_chapter_theme || '',
    chapter_text: overrides.chapter_text || '',
    target_reader: overrides.target_reader || session.target_persona || '',
    raw_user_task: overrides.raw_user_task || '',
  };

  const module = PROMPT_MODULES[moduleId];
  if (!module) return base;

  const filtered = {};
  for (const key of module.variables) {
    if (base[key] !== undefined && base[key] !== '') {
      filtered[key] = base[key];
    }
  }

  return { ...filtered, ...overrides };
}

function extractChapterCount(outlineText) {
  if (!outlineText || typeof outlineText !== 'string') return 12;
  const matches = outlineText.match(/### Chapter \d+/gi);
  return matches ? matches.length : 12;
}

export function getPlaybook() {
  return PLAYBOOK_STEPS;
}

export function getWorkflowStatus(session) {
  return {
    sessionId: session.sessionId,
    currentPlaybookStep: session.currentPlaybookStep,
    totalPlaybookSteps: PLAYBOOK_STEPS.length,
    activeChapter: session.active_chapter,
    currentChapterNumber: session.currentChapterNumber,
    totalChapters: session.totalChapters,
    nextModule: getNextModule(session),
    nextExecutableStep: session.next_executable_step,
    completedMilestones: session.completed_milestones.length,
    isComplete: getNextModule(session) === null && session.outputs['13_bio_architect'],
  };
}
