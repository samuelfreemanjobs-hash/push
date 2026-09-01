import { Router } from 'express';
import { n8nAuthMiddleware } from '../lib/auth.js';
import {
  buildDailyDashboard,
  buildFollowupsReport,
  buildPlanToday,
  buildWeeklyReview,
  logDashboardRun,
  postToSlack,
} from '../services/ea.js';

const router = Router();

router.use(n8nAuthMiddleware);

// n8n cron: weekday daily dashboard → Slack
router.post('/ea-daily', async (req, res) => {
  try {
    const dashboard = await buildDailyDashboard();
    const slack = await postToSlack(dashboard.text);

    await logDashboardRun({
      dashboardId: dashboard.dashboardId,
      runType: 'daily',
      primaryOutcome: dashboard.meta?.project,
      payload: dashboard.meta,
      slackPosted: slack.posted,
    });

    res.json({
      success: true,
      dashboardId: dashboard.dashboardId,
      slack,
      preview: dashboard.text.slice(0, 200),
    });
  } catch (error) {
    console.error('n8n ea-daily error:', error);
    res.status(500).json({ error: 'ea-daily failed', details: error.message });
  }
});

// n8n Slack slash command router (body from n8n after parsing Slack payload)
router.post('/slack-ea', async (req, res) => {
  try {
    const command = (req.body.command || req.body.text || '').trim().toLowerCase();
    let result;

    if (command.includes('weekly')) {
      result = await buildWeeklyReview();
    } else if (command.includes('followup')) {
      result = await buildFollowupsReport();
    } else if (command.includes('plan')) {
      result = await buildPlanToday();
    } else {
      const dashboard = await buildDailyDashboard();
      result = { text: dashboard.text };
    }

    res.json({
      success: true,
      response_type: 'in_channel',
      text: result.text,
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      response_type: 'ephemeral',
      text: `EA error: ${error.message}`,
    });
  }
});

router.get('/health', (req, res) => {
  res.json({
    status: 'ok',
    service: 'n8n-webhooks',
    timestamp: new Date().toISOString(),
  });
});

export default router;
