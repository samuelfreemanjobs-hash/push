-- MedFlow / Push runtime CRM schema (Supabase)
-- Run in Supabase SQL editor or via CLI migrate

-- Projects (MedFlow, internal ops, etc.)
create table if not exists projects (
  id uuid primary key default gen_random_uuid(),
  slug text unique not null,
  name text not null,
  status text not null default 'active' check (status in ('active', 'paused', 'archived')),
  primary_outcome text,
  created_at timestamptz not null default now(),
  updated_at timestamptz not null default now()
);

-- Operator gates (P0/P1 blockers surfaced on daily dashboard)
create table if not exists operator_gates (
  id uuid primary key default gen_random_uuid(),
  project_id uuid references projects(id) on delete cascade,
  priority text not null default 'P1' check (priority in ('P0', 'P1', 'P2', 'P3')),
  title text not null,
  doc_path text,
  status text not null default 'open' check (status in ('open', 'in_progress', 'done', 'cancelled')),
  due_date date,
  created_at timestamptz not null default now(),
  updated_at timestamptz not null default now()
);

-- Tasks
create table if not exists tasks (
  id uuid primary key default gen_random_uuid(),
  project_id uuid references projects(id) on delete set null,
  title text not null,
  description text,
  status text not null default 'todo' check (status in ('todo', 'in_progress', 'done', 'cancelled')),
  priority text not null default 'medium' check (priority in ('low', 'medium', 'high', 'critical')),
  revenue_task boolean not null default false,
  due_date date,
  completed_at timestamptz,
  created_at timestamptz not null default now(),
  updated_at timestamptz not null default now()
);

-- Follow-ups (CRM)
create table if not exists follow_ups (
  id uuid primary key default gen_random_uuid(),
  contact_name text,
  company text,
  channel text,
  subject text not null,
  status text not null default 'pending' check (status in ('pending', 'sent', 'replied', 'overdue', 'closed')),
  due_date date not null,
  notes text,
  created_at timestamptz not null default now(),
  updated_at timestamptz not null default now()
);

-- Leads (CRM)
create table if not exists leads (
  id uuid primary key default gen_random_uuid(),
  name text not null,
  company text,
  email text,
  phone text,
  source text,
  status text not null default 'new' check (status in ('new', 'contacted', 'qualified', 'proposal', 'won', 'lost')),
  notes text,
  created_at timestamptz not null default now(),
  updated_at timestamptz not null default now()
);

-- Opportunities (CRM pipeline)
create table if not exists opportunities (
  id uuid primary key default gen_random_uuid(),
  lead_id uuid references leads(id) on delete set null,
  title text not null,
  stage text not null default 'discovery' check (stage in ('discovery', 'demo', 'proposal', 'negotiation', 'won', 'lost')),
  value_usd numeric(12, 2),
  expected_close date,
  notes text,
  created_at timestamptz not null default now(),
  updated_at timestamptz not null default now()
);

-- Goals (weekly / quarterly)
create table if not exists goals (
  id uuid primary key default gen_random_uuid(),
  project_id uuid references projects(id) on delete cascade,
  title text not null,
  metric_target text,
  status text not null default 'active' check (status in ('active', 'achieved', 'missed', 'cancelled')),
  due_date date,
  created_at timestamptz not null default now(),
  updated_at timestamptz not null default now()
);

-- Dashboard run log (streak tracking)
create table if not exists dashboard_runs (
  id uuid primary key default gen_random_uuid(),
  dashboard_id text not null,
  run_type text not null default 'daily' check (run_type in ('daily', 'weekly', 'on_demand')),
  primary_outcome text,
  payload jsonb,
  slack_posted boolean not null default false,
  created_at timestamptz not null default now()
);

-- Conversations (existing agent chat log)
create table if not exists conversations (
  id uuid primary key default gen_random_uuid(),
  user_id text not null,
  user_message text not null,
  ai_response text not null,
  created_at timestamptz not null default now()
);

create index if not exists idx_tasks_status_due on tasks(status, due_date);
create index if not exists idx_follow_ups_status_due on follow_ups(status, due_date);
create index if not exists idx_operator_gates_status on operator_gates(status, priority);
create index if not exists idx_opportunities_stage on opportunities(stage);

-- Seed MedFlow as primary project
insert into projects (slug, name, status, primary_outcome)
values (
  'medflow',
  'MedFlow',
  'active',
  'Move one MedFlow revenue or product milestone forward today.'
)
on conflict (slug) do nothing;

-- Seed starter operator gates (edit via Supabase or API)
insert into operator_gates (project_id, priority, title, doc_path, status)
select p.id, g.priority, g.title, g.doc_path, 'open'
from projects p
cross join (
  values
    ('P0', 'Define MedFlow v1 wedge + ICP', 'docs/medflow/wedge.md'),
    ('P0', 'Wire n8n EA daily loop end-to-end', 'integrations/n8n/README.md'),
    ('P1', 'Deploy push runtime on Railway', 'docs/deploy/railway.md'),
    ('P1', 'Run Supabase CRM migration', 'supabase/migrations/001_crm_schema.sql')
) as g(priority, title, doc_path)
where p.slug = 'medflow'
  and not exists (
    select 1 from operator_gates og
    where og.project_id = p.id and og.title = g.title
  );
