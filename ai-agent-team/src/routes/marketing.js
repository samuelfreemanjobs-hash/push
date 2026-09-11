import express from 'express';
import { orchestrate } from '../agents/orchestrator.js';
import { runResearch } from '../agents/research.js';
import { runContent } from '../agents/content.js';
import { runSEO } from '../agents/seo.js';
import { runSocial } from '../agents/social.js';
import { runAnalytics } from '../agents/analytics.js';
import { TokenBudget } from '../utils/tokenBudget.js';
import { createClient } from '@supabase/supabase-js';

const router = express.Router();

const supabase =
  process.env.SUPABASE_URL && process.env.SUPABASE_KEY
    ? createClient(process.env.SUPABASE_URL, process.env.SUPABASE_KEY)
    : null;

const AGENT_MAP = {
  research: runResearch,
  seo: runSEO,
  analytics: runAnalytics,
};

router.post('/task', async (req, res) => {
  const { task, userId, tokenBudget = 8000 } = req.body;

  if (!task) return res.status(400).json({ error: 'task is required' });

  const budget = new TokenBudget(tokenBudget);
  const results = {};

  try {
    const routing = await orchestrate(task, budget);

    let researchContext = null;
    if (routing.agents.includes('research')) {
      researchContext = await runResearch(task, budget);
      results.research = researchContext;
    }

    if (routing.agents.includes('content')) {
      results.content = await runContent(task, researchContext, budget);
    }

    if (routing.agents.includes('social')) {
      const socialContext = results.content || researchContext;
      results.social = await runSocial(task, socialContext, budget);
    }

    const independentAgents = routing.agents.filter((a) => AGENT_MAP[a]);
    if (independentAgents.length > 0) {
      const parallelResults = await Promise.all(
        independentAgents.map((agentName) => AGENT_MAP[agentName](task, budget)),
      );
      independentAgents.forEach((name, i) => {
        results[name] = parallelResults[i];
      });
    }

    const tokenSummary = budget.summary();

    if (userId && supabase) {
      await supabase
        .from('marketing_tasks')
        .insert([
          {
            user_id: userId,
            task,
            routing: routing.agents,
            results,
            token_usage: tokenSummary,
            created_at: new Date().toISOString(),
          },
        ])
        .then(({ error }) => {
          if (error) console.error('Supabase insert error:', error);
        });
    }

    res.json({
      success: true,
      plan: routing.plan,
      agentsUsed: routing.agents,
      results,
      tokenUsage: tokenSummary,
    });
  } catch (err) {
    console.error('Marketing team error:', err);
    res.status(500).json({ error: err.message });
  }
});

router.post('/agent/:type', async (req, res) => {
  const { type } = req.params;
  const { task, context } = req.body;

  if (!task) return res.status(400).json({ error: 'task is required' });

  const budget = new TokenBudget(4000);

  try {
    let result;
    switch (type) {
      case 'research':
        result = await runResearch(task, budget);
        break;
      case 'content':
        result = await runContent(task, context, budget);
        break;
      case 'seo':
        result = await runSEO(task, budget);
        break;
      case 'social':
        result = await runSocial(task, context, budget);
        break;
      case 'analytics':
        result = await runAnalytics(task, budget);
        break;
      default:
        return res.status(400).json({ error: `Unknown agent type: ${type}` });
    }

    res.json({ success: true, agent: type, result, tokenUsage: budget.summary() });
  } catch (err) {
    console.error(`Agent ${type} error:`, err);
    res.status(500).json({ error: err.message });
  }
});

router.get('/agents', (_req, res) => {
  res.json({
    agents: [
      { name: 'research', description: 'Etsy niche and competitor research' },
      { name: 'content', description: 'Listing descriptions and shop copy' },
      { name: 'seo', description: 'Etsy titles and tags' },
      { name: 'social', description: 'Pinterest, Instagram, TikTok' },
      { name: 'analytics', description: 'Shop performance recommendations' },
    ],
    endpoints: {
      fullTeam: 'POST /api/marketing/task',
      singleAgent: 'POST /api/marketing/agent/:type',
      listAgents: 'GET /api/marketing/agents',
    },
  });
});

export default router;
