import { PLAYBOOK_STEPS, WORKFLOW_PHASES, listModules } from './module-registry.js';
import { runChapterLoop, runModule } from './module-runner.js';
import {
  compactState,
  createInitialState,
  getNextPlaybookStep,
  markMilestone,
  updateState,
} from './state.js';

/**
 * Execute a single playbook step by module ID.
 */
export async function executePlaybookStep(stepConfig, variables, state) {
  const { module } = stepConfig;

  if (module === 'chapter_loop') {
    const chapterNumber = stepConfig.chapter ?? variables.chapter_number ?? 1;
    const chapterState = updateState(state, {
      active_chapter: `Chapter ${chapterNumber}`,
    });

    const { results, state: updatedState } = await runChapterLoop(
      { ...variables, chapter_number: chapterNumber },
      chapterState
    );

    return {
      step: stepConfig,
      type: 'chapter_loop',
      results,
      state: updatedState,
    };
  }

  if (module === 'packaging') {
    const blurb = await runModule('12', variables, state);
    const bio = await runModule('13', variables, state);
    let updatedState = markMilestone(state, 'Blurb variants generated');
    updatedState = markMilestone(updatedState, 'Author bio variants generated');

    return {
      step: stepConfig,
      type: 'packaging',
      results: { blurb, bio },
      state: updatedState,
    };
  }

  const result = await runModule(module, variables, state);
  let updatedState = markMilestone(state, `Module ${module} complete`);

  if (module === '01') {
    updatedState = updateState(updatedState, {
      next_executable_step: 'Run Module 08: Title Testing Laboratory',
    });
  } else if (module === '08' && variables.selected_title) {
    updatedState = updateState(updatedState, {
      book_title: variables.selected_title,
      metadata: {
        ...updatedState.metadata,
        locked_title: variables.selected_title,
        locked_subtitle: variables.selected_subtitle ?? '',
        kdp_keywords: variables.kdp_keywords ?? [],
      },
      next_executable_step: 'Run Module 02: Outline Builder',
    });
  } else if (module === '02') {
    updatedState = updateState(updatedState, {
      metadata: { ...updatedState.metadata, outline: { raw: result.content } },
      next_executable_step: 'Run Module 05: Introduction Writer',
    });
  } else if (module === '05') {
    updatedState = markMilestone(updatedState, 'Introduction finalized');
    updatedState = updateState(updatedState, {
      next_executable_step: 'Begin Chapter Loop: Module 06→04→03→07→09→14',
    });
  }

  return {
    step: stepConfig,
    type: 'module',
    module,
    result,
    state: updatedState,
  };
}

/**
 * Run the full end-to-end Manuscript Master pipeline.
 * Stops after each major phase unless `autoAdvance` is true.
 */
export async function runFullPipeline(config) {
  const {
    variables = {},
    state: initialState = null,
    stopAfterPhase = 5,
    chapterConfigs = [],
  } = config;

  let state = initialState ?? createInitialState({
    book_title: variables.book_title ?? '',
    core_promise: variables.core_promise ?? '',
    target_persona: variables.target_audience ?? variables.primary_audience ?? '',
  });

  const phaseResults = [];

  for (const phase of WORKFLOW_PHASES) {
    if (phase.id > stopAfterPhase) break;

    const phaseOutput = { phase: phase.id, name: phase.name, steps: [] };

    if (phase.loop === 'per_chapter' && chapterConfigs.length > 0) {
      for (const chapterConfig of chapterConfigs) {
        const stepResult = await executePlaybookStep(
          { module: 'chapter_loop', chapter: chapterConfig.chapter_number },
          { ...variables, ...chapterConfig },
          state
        );
        state = stepResult.state;
        phaseOutput.steps.push(stepResult);
      }
    } else {
      for (const moduleId of phase.steps) {
        const stepResult = await executePlaybookStep(
          { module: moduleId },
          variables,
          state
        );
        state = stepResult.state;
        phaseOutput.steps.push(stepResult);
      }
    }

    phaseResults.push(phaseOutput);
  }

  return {
    phases: phaseResults,
    state: compactState(state),
    nextStep: getNextPlaybookStep(state),
    playbook: PLAYBOOK_STEPS,
  };
}

export { listModules, WORKFLOW_PHASES, PLAYBOOK_STEPS, getNextPlaybookStep, createInitialState };
