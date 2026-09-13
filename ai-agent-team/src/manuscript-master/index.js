import { GoogleGenerativeAI } from '@google/generative-ai';
import { SYSTEM_DIRECTIVE } from './system-directive.js';
import { MODULE_TEMPERATURE_MAP } from './config.js';
import { buildPrompt, listModules } from './prompt-modules.js';
import {
  createSession,
  advanceWorkflow,
  compactState,
  getNextModule,
  buildModuleVariables,
  getPlaybook,
  getWorkflowStatus,
} from './orchestrator.js';

const DEFAULT_MODEL = 'gemini-2.0-flash';

/**
 * Manuscript Master agent: executes prompt modules via Gemini with
 * temperature controls, adversarial audit loop, and workflow state.
 */
export class ManuscriptMasterAgent {
  constructor({ apiKey, model = DEFAULT_MODEL } = {}) {
    if (!apiKey) {
      throw new Error('GEMINI_API_KEY is required for Manuscript Master');
    }
    this.genAI = new GoogleGenerativeAI(apiKey);
    this.model = model;
    this.sessions = new Map();
  }

  getSession(sessionId) {
    return this.sessions.get(sessionId);
  }

  createWorkflowSession(initialData = {}) {
    const session = createSession(initialData);
    this.sessions.set(session.sessionId, session);
    return session;
  }

  listPromptModules() {
    return listModules();
  }

  getPlaybookSteps() {
    return getPlaybook();
  }

  getStatus(sessionId) {
    const session = this.sessions.get(sessionId);
    if (!session) throw new Error(`Session not found: ${sessionId}`);
    return getWorkflowStatus(session);
  }

  compactSessionState(sessionId) {
    const session = this.sessions.get(sessionId);
    if (!session) throw new Error(`Session not found: ${sessionId}`);
    return compactState(session);
  }

  /**
   * Build a prompt for a module without executing (preview mode).
   */
  previewModule(moduleId, variables = {}) {
    return buildPrompt(moduleId, variables);
  }

  /**
   * Execute a single prompt module against Gemini.
   */
  async executeModule(moduleId, variables = {}, options = {}) {
    const { runAudit = false, sessionId = null } = options;
    const { prompt, moduleName } = buildPrompt(moduleId, variables);
    const temperature = MODULE_TEMPERATURE_MAP[moduleId] ?? 0.55;

    const model = this.genAI.getGenerativeModel({
      model: this.model,
      generationConfig: { temperature },
      systemInstruction: SYSTEM_DIRECTIVE,
    });

    let output = await this.generate(model, prompt);

    if (runAudit && moduleId !== '14_quality_auditor') {
      output = await this.runAdversarialLoop(model, output, variables.target_avatar || variables.target_audience || '');
    }

    if (sessionId) {
      const session = this.sessions.get(sessionId);
      if (session) {
        advanceWorkflow(session, moduleId, output);
        if (variables.book_title) session.book_title = variables.book_title;
        if (variables.core_promise) session.core_promise = variables.core_promise;
        if (variables.target_audience) session.target_persona = variables.target_audience;
      }
    }

    return {
      moduleId,
      moduleName,
      temperature,
      output,
      sessionId,
    };
  }

  /**
   * Execute the next module in the workflow for a session.
   */
  async executeNext(sessionId, overrides = {}) {
    const session = this.sessions.get(sessionId);
    if (!session) throw new Error(`Session not found: ${sessionId}`);

    const moduleId = getNextModule(session);
    if (!moduleId) {
      return {
        complete: true,
        message: 'Manuscript workflow complete. Ready for KDP upload.',
        state: compactState(session),
      };
    }

    const variables = buildModuleVariables(session, moduleId, overrides);
    const runAudit = moduleId === '03_chapter_expander';

    const result = await this.executeModule(moduleId, variables, {
      runAudit,
      sessionId,
    });

    return {
      ...result,
      workflowStatus: getWorkflowStatus(session),
      state: compactState(session),
    };
  }

  /**
   * Adversarial self-critique loop: Generate → Brutal Audit → Targeted Polish.
   */
  async runAdversarialLoop(model, draft, targetAvatar) {
    const auditPrompt = buildPrompt('14_quality_auditor', {
      chapter_text: draft,
      target_avatar: targetAvatar || 'Business professional reader',
    }).prompt;

    const auditModel = this.genAI.getGenerativeModel({
      model: this.model,
      generationConfig: { temperature: MODULE_TEMPERATURE_MAP['14_quality_auditor'] },
      systemInstruction: SYSTEM_DIRECTIVE,
    });

    const auditResult = await this.generate(auditModel, auditPrompt);

    const polishPrompt = `<instructions>
You previously drafted content that received a developmental audit. Apply every mandatory fix from the audit report. Output only the polished, publication-ready text.
</instructions>
<constraints>
- Zero banned AI phrases.
- Preserve structure and word count targets.
- Implement all surgical rewrites from the audit.
</constraints>
<original_draft>
${draft}
</original_draft>
<audit_report>
${auditResult}
</audit_report>`;

    const polishModel = this.genAI.getGenerativeModel({
      model: this.model,
      generationConfig: { temperature: MODULE_TEMPERATURE_MAP['03_chapter_expander'] },
      systemInstruction: SYSTEM_DIRECTIVE,
    });

    const polished = await this.generate(polishModel, polishPrompt);

    return polished;
  }

  async generate(model, prompt) {
    const result = await model.generateContent(prompt);
    return result.response.text();
  }

  updateSession(sessionId, updates) {
    const session = this.sessions.get(sessionId);
    if (!session) throw new Error(`Session not found: ${sessionId}`);
    Object.assign(session, updates, { updatedAt: new Date().toISOString() });
    return session;
  }
}

export { SYSTEM_DIRECTIVE } from './system-directive.js';
export { buildPrompt, listModules } from './prompt-modules.js';
export {
  createSession,
  compactState,
  getPlaybook,
  getWorkflowStatus,
  getNextModule,
  buildModuleVariables,
} from './orchestrator.js';
