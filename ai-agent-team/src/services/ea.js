import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import yaml from 'js-yaml';
import { requireSupabase } from '../lib/supabase.js';
import { generateText } from '../lib/gemini.js';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const CONFIG_PATH = path.join(__dirname, '../../config/ea-dashboard.yaml');

function loadConfig() {
  const raw = fs.readFileSync(CONFIG_PATH, 'utf8');
  return yaml.load(raw);
}

function formatDate(date, timezone) {
  return new Intl.DateTimeFormat('en-US', {
    timeZone: timezone,
    weekday: 'long',
    year: 'numeric',
    month: 'long',
    day: 'numeric',
  }).format(date);
}

function dashboardId(date, timezone) {
  const parts = new Intl.DateTimeFormat('en-CA', {
    timeZone: timezone,
    year: 'numeric',
    month: '2-digit',
    day: '2-digit',
  }).formatToParts(date);
  const y = parts.find((p) => p.type === 'year')?.value;
  const m = parts.find((p) => p.type === 'month')?.value;
  const d = parts.find((p) => p.type === 'day')?.value;
  return `${y}-${m}-${d}`;
}

function isFriday(date, timezone) {
  const weekday = new Intl.DateTimeFormat('en-US', {
    timeZone: timezone,
    weekday: 'short',
  }).format(date);
  return weekday.startsWith('Fri');
}

async function fetchProject(slug) {
  const db = requireSupabase();
  const { data, error } = await db
    .from('projects')
    .select('*')
    .eq('slug', slug)
    .maybeSingle();
  if (error) throw error;
  return data;
}

async function fetchOpenGates(projectId) {
  const db = requireSupabase();
  let query = db
    .from('operator_gates')
    .select('*')
    .in('status', ['open', 'in_progress'])
    .order('priority', { ascending: true })
    .order('created_at', { ascending: true });

  if (projectId) {
    query = query.eq('project_id', projectId);
  }

  const { data, error } = await query;
  if (error) throw error;
  return data || [];
}

async function fetchRevenueTask(projectId) {
  const db = requireSupabase();
  let query = db
    .from('tasks')
    .select('*')
    .eq('revenue_task', true)
    .in('status', ['todo', 'in_progress'])
    .order('priority', { ascending: false })
    .order('due_date', { ascending: true, nullsFirst: false })
    .limit(1);

  if (projectId) {
    query = query.eq('project_id', projectId);
  }

  const { data, error } = await query;
  if (error) throw error;
  return data?.[0] || null;
}

async function fetchOverdueFollowups() {
  const db = requireSupabase();
  const today = new Date().toISOString().slice(0, 10);
  const { data, error } = await db
    .from('follow_ups')
    .select('*')
    .in('status', ['pending', 'overdue'])
    .lte('due_date', today)
    .order('due_date', { ascending: true })
    .limit(10);
  if (error) throw error;
  return data || [];
}

async function fetchPipelineSummary() {
  const db = requireSupabase();
  const { data, error } = await db
    .from('opportunities')
    .select('stage, value_usd')
    .not('stage', 'in', '("won","lost")');
  if (error) throw error;

  const rows = data || [];
  const count = rows.length;
  const value = rows.reduce((sum, row) => sum + Number(row.value_usd || 0), 0);
  return { count, value };
}

function gateLines(gates) {
  if (!gates.length) {
    return '☐ No open operator gates — add gates in Supabase `operator_gates`';
  }
  return gates
    .map((g) => `☐ [${g.priority}] ${g.title}${g.doc_path ? ` → ${g.doc_path}` : ''}`)
    .join('\n');
}

function followupLines(followups) {
  if (!followups.length) {
    return 'No overdue follow-ups.';
  }
  return followups
    .map(
      (f) =>
        `• ${f.subject}${f.company ? ` (${f.company})` : ''} — due ${f.due_date} [${f.status}]`
    )
    .join('\n');
}

function focusBlockLines(config, date) {
  const blocks = [...config.focus_blocks.weekday];
  if (isFriday(date, config.business.timezone) && config.focus_blocks.friday_suffix) {
    blocks[blocks.length - 1] = {
      ...blocks[blocks.length - 1],
      hint: config.focus_blocks.friday_suffix,
    };
  }
  return blocks.map((b) => `• ${b.label}: ${b.hint}`).join('\n');
}

function metricsTable(config) {
  const header = '| Metric | Target | Actual |\n|--------|--------|--------|';
  const rows = config.metrics
    .map((m) => `| ${m.label} | ${m.target} | ___ |`)
    .join('\n');
  return `${header}\n${rows}`;
}

export async function buildDailyDashboard({ now = new Date() } = {}) {
  const config = loadConfig();
  const tz = config.business.timezone;
  const project = await fetchProject(config.primary_project_slug);
  const gates = await fetchOpenGates(project?.id);
  const revenueTask = await fetchRevenueTask(project?.id);
  const followups = await fetchOverdueFollowups();
  const pipeline = await fetchPipelineSummary();

  const primaryOutcome =
    project?.primary_outcome ||
    `Move ${config.business.name} forward with one concrete win today.`;

  const topRevenueTask = revenueTask
    ? `${revenueTask.title}${revenueTask.description ? `\n${revenueTask.description}` : ''}`
    : `Define today's #1 ${config.business.name} revenue task in Supabase (tasks.revenue_task = true)`;

  const id = dashboardId(now, tz);
  const text = [
    `📊 DAILY EXECUTIVE DASHBOARD`,
    `${formatDate(now, tz)} · ${tz} · ${config.business.name}`,
    `Operator: ${config.business.operator}`,
    '',
    `Reply in this thread when you complete sections — streak tracked in Supabase.`,
    '',
    '-------------------------',
    '',
    `🎯 PRIMARY OUTCOME`,
    primaryOutcome,
    '',
    '-------------------------',
    '',
    `💰 TOP REVENUE TASK — do first`,
    topRevenueTask,
    `☐ Done · Time spent: ___ min · Notes: ___`,
    '',
    '-------------------------',
    '',
    `Today's focus blocks:`,
    focusBlockLines(config, now),
    '',
    '-------------------------',
    '',
    `✅ COMPLETION CHECKLIST`,
    `Morning setup (5 min)`,
    `☐ Read primary outcome — committed for today`,
    `☐ Calendar / conflicts checked`,
    `☐ Energy level (1–5): ___`,
    '',
    `Revenue execution`,
    `☐ Top revenue task completed`,
    `☐ At least 1 outbound touch OR 1 operator gate cleared`,
    `☐ Pipeline / tracker updated`,
    '',
    `End of day (5 min)`,
    `☐ EOD score (1–5): ___`,
    `☐ Biggest win today: ___`,
    `☐ Biggest blocker: ___`,
    `☐ Tomorrow's #1 task: ___`,
    '',
    '-------------------------',
    '',
    `🚧 OPEN OPERATOR GATES`,
    gateLines(gates),
    '',
    '-------------------------',
    '',
    `📈 PIPELINE SNAPSHOT`,
    `Open opportunities: ${pipeline.count} · Est. value: $${pipeline.value.toLocaleString()}`,
    '',
    '-------------------------',
    '',
    `📈 DAILY METRICS (fill in)`,
    metricsTable(config),
    '',
    `Score guide: 4–5 = win · 3 = solid · <3 = rerun top revenue task tomorrow 9 AM`,
    '',
    '-------------------------',
    '',
    `⚡ ACCOUNTABILITY`,
    `What's the one thing that, if you don't do it today, makes next week harder?`,
    '',
    '-------------------------',
    '',
    `Commands: ${config.commands.join(' · ')}`,
    `Config: config/ea-dashboard.yaml`,
    `Dashboard ID: ${id}`,
  ].join('\n');

  return {
    dashboardId: id,
    text,
    meta: {
      project: project?.slug || config.primary_project_slug,
      openGates: gates.length,
      overdueFollowups: followups.length,
      pipeline,
    },
  };
}

export async function buildFollowupsReport() {
  const followups = await fetchOverdueFollowups();
  const text = [
    `📬 FOLLOW-UPS`,
    followupLines(followups),
    '',
    `Total overdue/pending due today or earlier: ${followups.length}`,
  ].join('\n');
  return { text, count: followups.length };
}

export async function buildWeeklyReview() {
  const config = loadConfig();
  const db = requireSupabase();
  const weekAgo = new Date(Date.now() - 7 * 24 * 60 * 60 * 1000).toISOString();

  const [{ data: completedTasks }, { data: runs }, pipeline] = await Promise.all([
    db
      .from('tasks')
      .select('title, completed_at')
      .eq('status', 'done')
      .gte('completed_at', weekAgo),
    db.from('dashboard_runs').select('dashboard_id, created_at').gte('created_at', weekAgo),
    fetchPipelineSummary(),
  ]);

  const text = [
    `📅 WEEKLY REVIEW — ${config.business.name}`,
    `Operator: ${config.business.operator}`,
    '',
    `Tasks completed (7d): ${completedTasks?.length || 0}`,
    (completedTasks || []).map((t) => `• ${t.title}`).join('\n') || '• None logged',
    '',
    `Dashboard runs (7d): ${runs?.length || 0}`,
    `Open pipeline: ${pipeline.count} opps · $${pipeline.value.toLocaleString()}`,
    '',
    `Next week #1 focus: clear top P0 gate for ${config.business.name}.`,
  ].join('\n');

  return { text };
}

export async function buildPlanToday() {
  const config = loadConfig();
  const dashboard = await buildDailyDashboard();
  const prompt = [
    `You are an executive assistant for ${config.business.operator} building ${config.business.name}.`,
    'Given this dashboard context, produce a tight plan for today in bullet form (max 8 bullets).',
    'Prioritize revenue and P0 gates. Be specific and actionable.',
    '',
    dashboard.text,
  ].join('\n');

  const aiPlan = await generateText(prompt);
  const text = [
    `🗓️ PLAN TODAY — ${config.business.name}`,
    '',
    aiPlan || 'AI plan unavailable (set GEMINI_API_KEY). Use the daily dashboard focus blocks.',
    '',
    '---',
    'Source dashboard ID:',
    dashboard.dashboardId,
  ].join('\n');

  return { text, dashboardId: dashboard.dashboardId };
}

export async function logDashboardRun({ dashboardId, runType, primaryOutcome, payload, slackPosted }) {
  const db = requireSupabase();
  const { error } = await db.from('dashboard_runs').insert([
    {
      dashboard_id: dashboardId,
      run_type: runType,
      primary_outcome: primaryOutcome,
      payload,
      slack_posted: slackPosted,
    },
  ]);
  if (error) throw error;
}

export async function postToSlack(text) {
  const webhook = process.env.SLACK_WEBHOOK_URL;
  if (!webhook) {
    return { posted: false, reason: 'SLACK_WEBHOOK_URL not set' };
  }

  const response = await fetch(webhook, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ text }),
  });

  if (!response.ok) {
    const body = await response.text();
    throw new Error(`Slack webhook failed: ${response.status} ${body}`);
  }

  return { posted: true };
}
