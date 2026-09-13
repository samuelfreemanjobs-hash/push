import { Router } from 'express';
import { ManuscriptMasterAgent } from '../manuscript-master/index.js';

export function createManuscriptMasterRouter(apiKey) {
  const router = Router();
  const agent = new ManuscriptMasterAgent({ apiKey });

  router.get('/modules', (_req, res) => {
    res.json({ success: true, modules: agent.listPromptModules() });
  });

  router.get('/playbook', (_req, res) => {
    res.json({ success: true, playbook: agent.getPlaybookSteps() });
  });

  router.post('/sessions', (req, res) => {
    try {
      const session = agent.createWorkflowSession(req.body || {});
      res.json({
        success: true,
        sessionId: session.sessionId,
        state: agent.compactSessionState(session.sessionId),
      });
    } catch (error) {
      res.status(500).json({ error: error.message });
    }
  });

  router.get('/sessions/:sessionId', (req, res) => {
    try {
      const status = agent.getStatus(req.params.sessionId);
      const state = agent.compactSessionState(req.params.sessionId);
      res.json({ success: true, status, state });
    } catch (error) {
      res.status(404).json({ error: error.message });
    }
  });

  router.patch('/sessions/:sessionId', (req, res) => {
    try {
      const session = agent.updateSession(req.params.sessionId, req.body || {});
      res.json({
        success: true,
        state: agent.compactSessionState(session.sessionId),
      });
    } catch (error) {
      res.status(404).json({ error: error.message });
    }
  });

  router.post('/preview', (req, res) => {
    try {
      const { moduleId, variables } = req.body;
      if (!moduleId) {
        return res.status(400).json({ error: 'moduleId is required' });
      }
      const preview = agent.previewModule(moduleId, variables || {});
      res.json({ success: true, ...preview });
    } catch (error) {
      res.status(400).json({ error: error.message });
    }
  });

  router.post('/execute', async (req, res) => {
    try {
      const { moduleId, variables, sessionId, runAudit } = req.body;
      if (!moduleId) {
        return res.status(400).json({ error: 'moduleId is required' });
      }

      const result = await agent.executeModule(moduleId, variables || {}, {
        sessionId,
        runAudit: runAudit ?? false,
      });

      const response = { success: true, ...result };
      if (sessionId) {
        response.workflowStatus = agent.getStatus(sessionId);
        response.state = agent.compactSessionState(sessionId);
      }

      res.json(response);
    } catch (error) {
      res.status(500).json({ error: error.message });
    }
  });

  router.post('/sessions/:sessionId/next', async (req, res) => {
    try {
      const result = await agent.executeNext(req.params.sessionId, req.body || {});
      res.json({ success: true, ...result });
    } catch (error) {
      res.status(500).json({ error: error.message });
    }
  });

  return router;
}
