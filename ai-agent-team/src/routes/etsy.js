import express from 'express';
import { createClient } from '@supabase/supabase-js';
import {
  runEtsyPipeline,
  fetchShopSnapshot,
} from '../services/etsy-pipeline.js';
import { runAnalytics } from '../agents/analytics.js';
import { TokenBudget } from '../utils/tokenBudget.js';
import {
  buildAuthorizationUrl,
  exchangeAuthorizationCode,
  generatePkcePair,
} from '../etsy/oauth.js';

const router = express.Router();

const supabase =
  process.env.SUPABASE_URL && process.env.SUPABASE_KEY
    ? createClient(process.env.SUPABASE_URL, process.env.SUPABASE_KEY)
    : null;

function assertAutomationSecret(req) {
  const secret = process.env.ETSY_AUTOMATION_SECRET || process.env.N8N_WEBHOOK_SECRET;
  if (!secret) return true;
  const header = req.headers['x-etsy-automation-secret'] || req.headers['x-n8n-secret'];
  return header === secret;
}

async function persistRun(record) {
  if (!supabase) return;
  await supabase.from('etsy_pipeline_runs').insert([
    {
      user_id: record.userId,
      task: record.task,
      routing: record.routing,
      results: record.results,
      token_usage: record.tokenSummary,
      published: record.published,
      created_at: new Date().toISOString(),
    },
  ]);
}

router.get('/status', (_req, res) => {
  res.json({
    automation: {
      gemini: !!process.env.GEMINI_API_KEY,
      etsyApi: !!process.env.ETSY_API_KEY,
      etsyOAuth: !!process.env.ETSY_ACCESS_TOKEN,
      shopId: process.env.ETSY_SHOP_ID || null,
      supabase: !!supabase,
    },
    endpoints: {
      pipeline: 'POST /api/etsy/pipeline/run',
      shop: 'GET /api/etsy/shop/snapshot',
      oauthStart: 'GET /api/etsy/oauth/start',
      oauthCallback: 'GET /api/etsy/oauth/callback',
    },
  });
});

router.post('/pipeline/run', async (req, res) => {
  if (!assertAutomationSecret(req)) {
    return res.status(401).json({ error: 'Invalid automation secret' });
  }

  const { task, niche, productType, publish, tokenBudget, userId } = req.body;

  try {
    const result = await runEtsyPipeline({
      task,
      niche,
      productType,
      publish: publish === true,
      tokenBudget,
      userId,
      persist: userId ? persistRun : undefined,
    });
    res.json(result);
  } catch (err) {
    console.error('Etsy pipeline error:', err);
    res.status(500).json({ error: err.message });
  }
});

router.get('/shop/snapshot', async (req, res) => {
  if (!assertAutomationSecret(req)) {
    return res.status(401).json({ error: 'Invalid automation secret' });
  }

  try {
    const snapshot = await fetchShopSnapshot();
    const budget = new TokenBudget(3000);
    const insights = await runAnalytics(
      `Shop snapshot JSON:\n${JSON.stringify(snapshot, null, 2)}\n\nSummarize performance and next actions.`,
      budget,
    );
    res.json({ snapshot, insights, tokenUsage: budget.summary() });
  } catch (err) {
    console.error('Shop snapshot error:', err);
    res.status(500).json({ error: err.message });
  }
});

router.get('/oauth/start', (req, res) => {
  const clientId = process.env.ETSY_API_KEY;
  const redirectUri = process.env.ETSY_REDIRECT_URI;
  if (!clientId || !redirectUri) {
    return res.status(400).json({
      error: 'Set ETSY_API_KEY and ETSY_REDIRECT_URI',
    });
  }

  const { verifier, challenge } = generatePkcePair();
  const state = Buffer.from(JSON.stringify({ v: verifier })).toString('base64url');
  const url = buildAuthorizationUrl({
    clientId,
    redirectUri,
    state,
    codeChallenge: challenge,
  });

  res.json({
    authorizationUrl: url,
    note: 'Open authorizationUrl in a browser. State embeds PKCE verifier for callback.',
  });
});

router.get('/oauth/callback', async (req, res) => {
  const { code, state } = req.query;
  if (!code || !state) {
    return res.status(400).json({ error: 'code and state query params required' });
  }

  try {
    const parsed = JSON.parse(Buffer.from(state, 'base64url').toString('utf8'));
    const tokens = await exchangeAuthorizationCode({
      clientId: process.env.ETSY_API_KEY,
      redirectUri: process.env.ETSY_REDIRECT_URI,
      code,
      codeVerifier: parsed.v,
    });

    res.json({
      message: 'Store ETSY_ACCESS_TOKEN and ETSY_REFRESH_TOKEN in your secrets manager',
      access_token: tokens.access_token,
      refresh_token: tokens.refresh_token,
      expires_in: tokens.expires_in,
    });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

export default router;
