/** Manuscript Master configuration: temperatures, safeguards, workflow phases */

export const TEMPERATURE_GUIDELINES = {
  ideation: 0.85,
  outlining: 0.3,
  expansion: 0.55,
  audit: 0.1,
};

export const MODULE_TEMPERATURE_MAP = {
  '01_concept_generator': TEMPERATURE_GUIDELINES.ideation,
  '02_outline_builder': TEMPERATURE_GUIDELINES.outlining,
  '03_chapter_expander': TEMPERATURE_GUIDELINES.expansion,
  '04_story_creator': TEMPERATURE_GUIDELINES.expansion,
  '05_intro_writer': TEMPERATURE_GUIDELINES.expansion,
  '06_research_integrator': TEMPERATURE_GUIDELINES.outlining,
  '07_exercise_designer': TEMPERATURE_GUIDELINES.outlining,
  '08_title_laboratory': TEMPERATURE_GUIDELINES.ideation,
  '09_bridge_specialist': TEMPERATURE_GUIDELINES.expansion,
  '10_case_study_studio': TEMPERATURE_GUIDELINES.expansion,
  '11_conclusion_writer': TEMPERATURE_GUIDELINES.expansion,
  '12_blurb_engine': TEMPERATURE_GUIDELINES.ideation,
  '13_bio_architect': TEMPERATURE_GUIDELINES.expansion,
  '14_quality_auditor': TEMPERATURE_GUIDELINES.audit,
  meta_prompt_generator: TEMPERATURE_GUIDELINES.outlining,
};

export const BANNED_PHRASES = [
  "In today's fast-paced digital world",
  'Delve into',
  'Tapestry',
  'Testament',
  'Beacon',
  'Symphony',
  'Crucial',
  "It's important to remember",
  'Needless to say',
  'In conclusion',
];

export const WORD_BUDGETS = {
  chapter: { min: 2000, max: 2800 },
  introduction: { min: 1000, max: 1500 },
  conclusion: { min: 800, max: 1200 },
  sectionBlock: { min: 800, max: 1500 },
  vignette: { min: 200, max: 300 },
  blurb: { min: 250, max: 350 },
};

export const WORKFLOW_PHASES = [
  {
    id: 'phase_1',
    name: 'Market Intelligence & Positioning',
    steps: ['01_concept_generator', '08_title_laboratory'],
  },
  {
    id: 'phase_2',
    name: 'Structural Architecture & Transformation',
    steps: ['02_outline_builder'],
  },
  {
    id: 'phase_3',
    name: 'Modular Expansion & Narrative Injection',
    steps: ['05_intro_writer', 'chapter_loop'],
  },
  {
    id: 'phase_4',
    name: 'Adversarial Critique & Developmental Audit',
    steps: ['14_quality_auditor'],
  },
  {
    id: 'phase_5',
    name: 'Packaging, Back-Cover Conversion & Metas',
    steps: ['11_conclusion_writer', '12_blurb_engine', '13_bio_architect'],
  },
];

export const CHAPTER_LOOP_MODULES = [
  '06_research_integrator',
  '04_story_creator',
  '03_chapter_expander',
  '07_exercise_designer',
  '09_bridge_specialist',
  '14_quality_auditor',
];

export const PLAYBOOK_STEPS = [
  { step: 1, module: '01_concept_generator', action: 'Generate 5 concepts; select winning concept' },
  { step: 2, module: '08_title_laboratory', action: 'Lock title, subtitle, and 7 KDP keyword strings' },
  { step: 3, module: '02_outline_builder', action: 'Lock 10-12 chapter progression and milestones' },
  { step: 4, module: '05_intro_writer', action: 'Establish credibility and reader promise' },
  {
    step: 5,
    module: 'chapter_loop',
    action: 'Per chapter: research → stories → draft → exercises → bridge → audit',
  },
  { step: 6, module: '11_conclusion_writer', action: 'Cement retention and 90-day action plan' },
  {
    step: 7,
    module: 'packaging',
    action: 'Blurb engine + bio architect for KDP listing',
  },
];

export const STATE_COMPACTION_INTERVAL = 3;
