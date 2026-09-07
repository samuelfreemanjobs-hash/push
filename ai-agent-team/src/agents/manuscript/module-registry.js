/**
 * Registry of all 14 Manuscript Master prompt modules with metadata.
 */
export const MODULES = {
  '01': {
    id: '01_concept_generator',
    name: 'Profitable Book Concept & KDP Market Arbitrage Generator',
    phase: 1,
    temperature: 0.85,
    maxOutputTokens: 4000,
    variables: ['niche_expertise', 'primary_audience', 'monetization_goal'],
  },
  '02': {
    id: '02_outline_builder',
    name: 'Architectural Blueprint & Transformation Outline Builder',
    phase: 2,
    temperature: 0.30,
    maxOutputTokens: 5000,
    variables: ['book_title', 'core_promise', 'target_audience'],
  },
  '03': {
    id: '03_chapter_expander',
    name: 'Section-by-Section Chapter Content Expander',
    phase: 3,
    temperature: 0.55,
    maxOutputTokens: 4000,
    wordBudget: { min: 800, max: 1500 },
    variables: [
      'chapter_number',
      'chapter_title',
      'target_audience',
      'chapter_objectives',
      'voice_and_tone',
    ],
  },
  '04': {
    id: '04_story_creator',
    name: 'Narrative & High-Impact Case Anecdote Creator',
    phase: 3,
    temperature: 0.55,
    maxOutputTokens: 2500,
    variables: ['target_concept', 'target_audience'],
  },
  '05': {
    id: '05_intro_writer',
    name: 'High-Converting Bestseller Introduction Writer',
    phase: 2,
    temperature: 0.55,
    maxOutputTokens: 2500,
    variables: ['book_title', 'author_background', 'core_enemy', 'promised_outcome'],
  },
  '06': {
    id: '06_research_integrator',
    name: 'Empirical Research & Data Evidence Integrator',
    phase: 3,
    temperature: 0.30,
    maxOutputTokens: 3000,
    variables: ['chapter_topic', 'target_industry'],
  },
  '07': {
    id: '07_exercise_designer',
    name: 'Reader Action Template & Framework Designer',
    phase: 3,
    temperature: 0.30,
    maxOutputTokens: 3000,
    variables: ['chapter_mechanism', 'reader_avatar'],
  },
  '08': {
    id: '08_title_laboratory',
    name: 'Amazon KDP Algorithm & Title-Testing Laboratory',
    phase: 1,
    temperature: 0.85,
    maxOutputTokens: 3000,
    variables: ['primary_subject', 'target_buyer', 'primary_keywords'],
  },
  '09': {
    id: '09_bridge_specialist',
    name: 'Narrative Momentum & Chapter Bridge Architect',
    phase: 3,
    temperature: 0.55,
    maxOutputTokens: 1500,
    variables: ['ending_chapter_content', 'upcoming_chapter_theme'],
  },
  '10': {
    id: '10_case_study_studio',
    name: 'Deep-Dive Before vs. After Case Study Studio',
    phase: 3,
    temperature: 0.55,
    maxOutputTokens: 4000,
    variables: ['methodology_name', 'desired_outcomes'],
  },
  '11': {
    id: '11_conclusion_writer',
    name: 'Decisive Conclusion & Future Vision Closer',
    phase: 4,
    temperature: 0.55,
    maxOutputTokens: 2500,
    variables: ['book_title', 'macro_transformation'],
  },
  '12': {
    id: '12_blurb_engine',
    name: 'High-Conversion Amazon KDP Product Page Blurb Engine',
    phase: 5,
    temperature: 0.85,
    maxOutputTokens: 2000,
    variables: ['book_title', 'target_reader', 'big_benefit', 'key_bullets'],
  },
  '13': {
    id: '13_bio_architect',
    name: 'High-Authority Author Bio & Credibility Architecture',
    phase: 5,
    temperature: 0.55,
    maxOutputTokens: 2000,
    variables: ['author_name', 'core_credentials', 'personal_philosophy'],
  },
  '14': {
    id: '14_quality_auditor',
    name: 'Adversarial Manuscript Auditor & Quality Scorecard',
    phase: 4,
    temperature: 0.10,
    maxOutputTokens: 4000,
    variables: ['chapter_text', 'target_avatar'],
  },
};

export const WORKFLOW_PHASES = [
  {
    id: 1,
    name: 'Market Intelligence & Positioning Matrix',
    steps: ['01', '08'],
  },
  {
    id: 2,
    name: 'Structural Architecture & Transformation',
    steps: ['02', '05'],
  },
  {
    id: 3,
    name: 'Modular Expansion & Narrative Injection',
    steps: ['06', '04', '03', '07', '09'],
    loop: 'per_chapter',
  },
  {
    id: 4,
    name: 'Adversarial Critique & Developmental Audit',
    steps: ['14', '11'],
  },
  {
    id: 5,
    name: 'Packaging, Back-Cover Conversion & Metas',
    steps: ['12', '13'],
  },
];

export const PLAYBOOK_STEPS = [
  { step: 1, module: '01', action: 'Generate 5 commercially viable book concepts' },
  { step: 2, module: '08', action: 'Lock title, subtitle, and KDP keywords' },
  { step: 3, module: '02', action: 'Build 10-14 chapter blueprint' },
  { step: 4, module: '05', action: 'Draft high-converting introduction' },
  {
    step: 5,
    module: 'chapter_loop',
    action: 'Per chapter: 06 → 04 → 03 → 07 → 09 → 14',
  },
  { step: 6, module: '11', action: 'Write conclusion with 90-day roadmap' },
  { step: 7, module: 'packaging', action: '12 (blurb) + 13 (bio)' },
];

export function getModule(moduleId) {
  const key = moduleId.replace(/^0+/, '').padStart(2, '0');
  const altKey = moduleId.length === 2 ? moduleId : key;
  const module = MODULES[altKey] ?? MODULES[moduleId];
  if (!module) {
    throw new Error(`Unknown module: ${moduleId}. Valid: ${Object.keys(MODULES).join(', ')}`);
  }
  return { key: altKey, ...module };
}

export function listModules() {
  return Object.entries(MODULES).map(([key, mod]) => ({
    id: key,
    moduleId: mod.id,
    name: mod.name,
    phase: mod.phase,
    variables: mod.variables,
  }));
}
