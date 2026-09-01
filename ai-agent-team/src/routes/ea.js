import { Router } from 'express';
import {
  buildDailyDashboard,
  buildFollowupsReport,
  buildPlanToday,
  buildWeeklyReview,
  logDashboardRun,
  postToSlack,
} from '../services/ea.js';

const router = Router();

router.get('/dashboard', async (req, res) => {
  try {
    const dashboard = await buildDailyDashboard();
    res.json({ success: true, ...dashboard });
  } catch (error) {
    console.error('EA dashboard error:', error);
    res.status(500).json({ error: 'Failed to build dashboard', details: error.message });
  }
});

router.post('/dashboard', async (req, res) => {
  try {
    const postSlack = req.body?.postSlack !== false;
    const dashboard = await buildDailyDashboard();
    let slack = { posted: false };

    if (postSlack) {
      slack = await postToSlack(dashboard.text);
      await logDashboardRun({
        dashboardId: dashboard.dashboardId,
        runType: 'on_demand',
        primaryOutcome: dashboard.meta?.project,
        payload: dashboard.meta,
        slackPosted: slack.posted,
      });
    }

    res.json({ success: true, ...dashboard, slack });
  } catch (error) {
    console.error('EA dashboard error:', error);
    res.status(500).json({ error: 'Failed to build dashboard', details: error.message });
  }
});

router.post('/plan-today', async (req, res) => {
  try {
    const plan = await buildPlanToday();
    res.json({ success: true, ...plan });
  } catch (error) {
    res.status(500).json({ error: 'Failed to build plan', details: error.message });
  }
});

router.post('/followups', async (req, res) => {
  try {
    const report = await buildFollowupsReport();
    res.json({ success: true, ...report });
  } catch (error) {
    res.status(500).json({ error: 'Failed to build followups', details: error.message });
  }
});

router.post('/weekly-review', async (req, res) => {
  try {
    const review = await buildWeeklyReview();
    res.json({ success: true, ...review });
  } catch (error) {
    res.status(500).json({ error: 'Failed to build weekly review', details: error.message });
  }
});

export default router;
