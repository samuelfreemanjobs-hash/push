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

const supabase = createClient(
  process.env.SUPABASE_URL,
  process.env.SUPABASE_KEY
);

const AGENT_MAP = {
  research: runResearch,
  seo: runSEO,
  analytics: runAnalytics,
};

// Run the full marketing team on a task
router.post('/task', async (req, res) => {
  const { task, userId, tokenBudget = 8000 } = req.body;

  if (!task) return res.status(400).json({ error: 'task is required' });

  const budget = new TokenBudget(tokenBudget);
  const results = {};

  try {
    // Step 1: Orchestrator decides which agents to use
    const routing = await orchestrate(task, budget);

    // Step 2: Run research first if needed (other agents can use it as context)
    let researchContext = null;
    if (routing.agents.includes('research')) {
      researchContext = await runResearch(task, budget);
      results.research = researchContext;
    }

    // Step 3: Run content agent (may use research context)
    if (routing.agents.includes('content')) {
      results.content = await runContent(task, researchContext, budget);
    }

    // Step 4: Run social agent (may use content as context)
    if (routing.agents.includes('social')) {
      const socialContext = results.content || researchContext;
      results.social = await runSocial(task, socialContext, budget);
    }

    // Step 5: Run independent agents in parallel
    const independentAgents = routing.agents.filter(a => AGENT_MAP[a]);
    if (independentAgents.length > 0) {
      const parallelResults = await Promise.all(
        independentAgents.map(agentName => AGENT_MAP[agentName](task, budget))
      );
      independentAgents.forEach((name, i) => {
        results[name] = parallelResults[i];
      });
    }

    const tokenSummary = budget.summary();

    // Persist to Supabase if userId provided
    if (userId) {
      await supabase.from('marketing_tasks').insert([{
        user_id: userId,
        task,
        routing: routing.agents,
        results,
        token_usage: tokenSummary,
        created_at: new Date().toISOString(),
      }]).then(({ error }) => {
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

// Call a specific agent directly (bypass orchestrator)
router.post('/agent/:type', async (req, res) => {
  const { type } = req.params;
  const { task, context } = req.body;

  if (!task) return res.status(400).json({ error: 'task is required' });

  const budget = new TokenBudget(4000);

  try {
    let result;
    switch (type) {
      case 'research':  result = await runResearch(task, budget); break;
      case 'content':   result = await runContent(task, context, budget); break;
      case 'seo':       result = await runSEO(task, budget); break;
      case 'social':    result = await runSocial(task, context, budget); break;
      case 'analytics': result = await runAnalytics(task, budget); break;
      default:
        return res.status(400).json({ error: `Unknown agent type: ${type}` });
    }

    res.json({ success: true, agent: type, result, tokenUsage: budget.summary() });
  } catch (err) {
    console.error(`Agent ${type} error:`, err);
    res.status(500).json({ error: err.message });
  }
});

// List available agents and capabilities
router.get('/agents', (_req, res) => {
  res.json({
    agents: [
      { name: 'research', description: 'Market research, competitor analysis, audience insights', model: 'haiku' },
      { name: 'content', description: 'Blog posts, email copy, ad copy, landing pages', model: 'sonnet' },
      { name: 'seo', description: 'Keywords, meta tags, header structure, optimization', model: 'haiku' },
      { name: 'social', description: 'LinkedIn, Twitter, Instagram, Facebook posts', model: 'sonnet' },
      { name: 'analytics', description: 'Campaign metrics interpretation, recommendations', model: 'haiku' },
    ],
    endpoints: {
      fullTeam: 'POST /api/marketing/task',
      singleAgent: 'POST /api/marketing/agent/:type',
      listAgents: 'GET /api/marketing/agents',
    },
  });
});

export default router;
