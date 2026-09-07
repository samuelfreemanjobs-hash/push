/**
 * Manuscript session state management with compaction every 3 chapters.
 */

export function createInitialState(overrides = {}) {
  return {
    book_title: '',
    core_promise: '',
    target_persona: '',
    active_chapter: null,
    narrative_threads: [],
    completed_milestones: [],
    next_executable_step: 'Run Module 01: Concept Generator',
    metadata: {
      selected_concept: null,
      locked_title: null,
      locked_subtitle: null,
      kdp_keywords: [],
      outline: null,
      chapters_completed: 0,
      last_compaction_at_chapter: 0,
    },
    ...overrides,
  };
}

export function updateState(state, patch) {
  return {
    ...state,
    ...patch,
    narrative_threads: patch.narrative_threads ?? state.narrative_threads,
    completed_milestones: patch.completed_milestones ?? state.completed_milestones,
    metadata: {
      ...state.metadata,
      ...(patch.metadata ?? {}),
    },
  };
}

export function markMilestone(state, milestone) {
  const completed = state.completed_milestones.includes(milestone)
    ? state.completed_milestones
    : [...state.completed_milestones, milestone];

  return updateState(state, { completed_milestones: completed });
}

export function markChapterComplete(state, chapterNumber, chapterTitle) {
  const chaptersCompleted = (state.metadata.chapters_completed ?? 0) + 1;
  const updated = updateState(state, {
    active_chapter: `Chapter ${chapterNumber}: ${chapterTitle}`,
    metadata: { ...state.metadata, chapters_completed: chaptersCompleted },
  });

  updated.completed_milestones = [
    ...updated.completed_milestones,
    `Chapter ${chapterNumber} draft finalized`,
  ];

  return updated;
}

/**
 * Returns true when state should be compacted (every 3 chapters per spec).
 */
export function shouldCompact(state) {
  const completed = state.metadata?.chapters_completed ?? 0;
  const lastCompaction = state.metadata?.last_compaction_at_chapter ?? 0;
  return completed > 0 && completed % 3 === 0 && completed > lastCompaction;
}

export function recordCompaction(state) {
  return updateState(state, {
    metadata: {
      ...state.metadata,
      last_compaction_at_chapter: state.metadata.chapters_completed,
      compacted_at: new Date().toISOString(),
    },
  });
}

/**
 * Produce a compact JSON snapshot for multi-turn sessions.
 */
export function compactState(state) {
  return {
    book_title: state.book_title,
    core_promise: state.core_promise,
    target_persona: state.target_persona,
    active_chapter: state.active_chapter,
    narrative_threads: state.narrative_threads,
    completed_milestones: state.completed_milestones,
    next_executable_step: state.next_executable_step,
  };
}

export function getNextPlaybookStep(state) {
  const { completed_milestones, metadata } = state;

  if (!completed_milestones.some((m) => m.includes('Module 01'))) {
    return { module: '01', action: 'Generate book concepts' };
  }
  if (!metadata.locked_title) {
    return { module: '08', action: 'Test and lock title/subtitle' };
  }
  if (!metadata.outline) {
    return { module: '02', action: 'Build chapter blueprint' };
  }
  if (!completed_milestones.some((m) => m.includes('Introduction'))) {
    return { module: '05', action: 'Draft introduction' };
  }

  const chaptersDone = metadata.chapters_completed ?? 0;
  const totalChapters = metadata.outline?.chapters?.length ?? 12;

  if (chaptersDone < totalChapters) {
    const nextChapter = chaptersDone + 1;
    return {
      module: 'chapter_loop',
      chapter: nextChapter,
      action: `Draft chapter ${nextChapter}: run 06→04→03→07→09→14`,
    };
  }

  if (!completed_milestones.some((m) => m.includes('Conclusion'))) {
    return { module: '11', action: 'Write conclusion' };
  }
  if (!completed_milestones.some((m) => m.includes('Blurb'))) {
    return { module: '12', action: 'Generate KDP blurb variants' };
  }
  if (!completed_milestones.some((m) => m.includes('Author bio'))) {
    return { module: '13', action: 'Write author bio variants' };
  }

  return { module: null, action: 'Manuscript pipeline complete' };
}
