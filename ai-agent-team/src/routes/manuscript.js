import express from 'express';
import { createClient } from '@supabase/supabase-js';
import {
  createInitialState,
  getNextPlaybookStep,
  listModules,
  PLAYBOOK_STEPS,
  runFullPipeline,
  WORKFLOW_PHASES,
} from '../agents/manuscript/orchestrator.js';
import {
  runChapterLoop,
  runMetaPromptGenerator,
  runModule,
  runWithAudit,
} from '../agents/manuscript/module-runner.js';
import { compactState } from '../agents/manuscript/state.js';
import { loadPromptModule, loadSystemDirective } from '../agents/manuscript/prompt-loader.js';
import { getModule } from '../agents/manuscript/module-registry.js';

const router = express.Router();

const supabase = process.env.SUPABASE_URL && process.env.SUPABASE_KEY
  ? createClient(process.env.SUPABASE_URL, process.env.SUPABASE_KEY)
  : null;

async function persistManuscriptRun(userId, payload) {
  if (!userId || !supabase) return;
  const { error } = await supabase.from('manuscript_runs').insert([{
    user_id: userId,
    ...payload,
    created_at: new Date().toISOString(),
  }]);
  if (error) console.error('Supabase manuscript_runs insert error:', error);
}

// List all modules, phases, and playbook
router.get('/modules', (_req, res) => {
  res.json({
    agent: 'Manuscript Master',
    description: 'Autonomous KDP business book architect & executive ghostwriter',
    modules: listModules(),
    phases: WORKFLOW_PHASES,
    playbook: PLAYBOOK_STEPS,
    safeguards: {
      sixtyPercentRule: 'Cap sections at 800–1,500 words per generation pass',
      stateCompaction: 'JSON snapshot every 3 chapters',
      adversarialLoop: 'Generate → Audit (Module 14) → Polish',
    },
  });
});

// Get raw prompt template for a module (no AI call)
router.get('/prompt/:moduleId', async (req, res) => {
  try {
    const mod = getModule(req.params.moduleId);
    const template = await loadPromptModule(mod.key, {});
    res.json({
      moduleId: mod.key,
      name: mod.name,
      variables: mod.variables,
      temperature: mod.temperature,
      template,
    });
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
});

// Get system directive
router.get('/system-directive', async (_req, res) => {
  try {
    const directive = await loadSystemDirective();
    res.json({ systemDirective: directive });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// Run a single module
router.post('/module/:moduleId', async (req, res) => {
  const { variables = {}, state = null, userId } = req.body;

  try {
    const result = await runModule(req.params.moduleId, variables, state);

    await persistManuscriptRun(userId, {
      type: 'module',
      module_id: result.moduleId,
      variables,
      usage: result.usage,
    });

    res.json({ success: true, ...result });
  } catch (err) {
    console.error('Manuscript module error:', err);
    res.status(500).json({ error: err.message });
  }
});

// Run chapter drafting loop (06→04→03→07→09→14)
router.post('/chapter', async (req, res) => {
  const { chapterConfig, state: initialState, userId } = req.body;

  if (!chapterConfig?.chapter_number || !chapterConfig?.chapter_title) {
    return res.status(400).json({
      error: 'chapterConfig requires chapter_number and chapter_title',
    });
  }

  try {
    const state = initialState ?? createInitialState();
    const { results, state: updatedState } = await runChapterLoop(chapterConfig, state);

    await persistManuscriptRun(userId, {
      type: 'chapter_loop',
      chapter: chapterConfig.chapter_number,
      state: compactState(updatedState),
    });

    res.json({
      success: true,
      chapter: chapterConfig.chapter_number,
      results,
      state: compactState(updatedState),
      nextStep: getNextPlaybookStep(updatedState),
    });
  } catch (err) {
    console.error('Chapter loop error:', err);
    res.status(500).json({ error: err.message });
  }
});

// Run module with adversarial audit
router.post('/audit', async (req, res) => {
  const { moduleId, variables = {}, state = null, draftContent } = req.body;

  if (!moduleId) return res.status(400).json({ error: 'moduleId is required' });

  try {
    const result = await runWithAudit(moduleId, variables, state, draftContent);
    res.json({ success: true, ...result });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// Run full pipeline (phases 1–5)
router.post('/pipeline', async (req, res) => {
  const {
    variables = {},
    state = null,
    stopAfterPhase = 5,
    chapterConfigs = [],
    userId,
  } = req.body;

  try {
    const result = await runFullPipeline({
      variables,
      state,
      stopAfterPhase,
      chapterConfigs,
    });

    await persistManuscriptRun(userId, {
      type: 'pipeline',
      stop_after_phase: stopAfterPhase,
      state: result.state,
    });

    res.json({ success: true, ...result });
  } catch (err) {
    console.error('Pipeline error:', err);
    res.status(500).json({ error: err.message });
  }
});

// Meta-prompt generator for bespoke modules
router.post('/meta-prompt', async (req, res) => {
  const { rawUserTask } = req.body;
  if (!rawUserTask) return res.status(400).json({ error: 'rawUserTask is required' });

  try {
    const result = await runMetaPromptGenerator(rawUserTask);
    res.json({ success: true, ...result });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// Get recommended next step from current state
router.post('/next-step', (req, res) => {
  const state = req.body.state ?? createInitialState();
  res.json({
    nextStep: getNextPlaybookStep(state),
    compactState: compactState(state),
  });
});

export default router;
