-- MedFlow 12-month playbook seeds
-- Run after 001_crm_schema.sql

update projects
set
  primary_outcome = 'Advance one MedFlow milestone: build, SPARK, pilot, or paid conversion.',
  name = 'MedFlow',
  updated_at = now()
where slug = 'medflow';

-- Replace generic Phase 1 gates with playbook gates
delete from operator_gates
where project_id = (select id from projects where slug = 'medflow')
  and title in (
    'Define MedFlow v1 wedge + ICP',
    'Wire n8n EA daily loop end-to-end',
    'Deploy push runtime on Railway',
    'Run Supabase CRM migration'
  );

insert into operator_gates (project_id, priority, title, doc_path, status)
select p.id, g.priority, g.title, g.doc_path, 'open'
from projects p
cross join (
  values
    -- Months 1-2
    ('P0', 'HIPAA infra live (AWS BAA + Supabase encryption documented)', 'docs/MEDFLOW-OS.md#compliance-non-negotiables'),
    ('P0', 'BAA template from healthcare attorney (~$500)', 'docs/medflow/wedge.md'),
    ('P0', 'Core MVP loop: audio → transcript → note → CPT → approval UI', 'docs/medflow/zero-to-revenue.md'),
    ('P0', 'Apply Ann Arbor SPARK (SPARK Central Innovation Center)', 'docs/medflow/zero-to-revenue.md'),
    ('P1', 'LinkedIn build-in-public cadence (2×/week)', 'docs/medflow/zero-to-revenue.md'),
    ('P1', 'n8n EA daily loop live on Railway', 'integrations/n8n/README.md'),
    -- Months 2-4
    ('P0', 'Warm intro to 1 pilot clinic (SPARK — not cold outreach)', 'docs/MEDFLOW-OS.md#months-2-4--design-partner'),
    ('P0', 'BAA signed with design partner before any PHI', 'docs/medflow/wedge.md'),
    ('P1', 'Chrome extension overlay prototype (Manifest V3)', 'docs/MEDFLOW-OS.md#phase-2-build-priorities-product'),
    -- Months 5-6
    ('P0', 'Design partner converts to paid ($250/provider/mo)', 'docs/medflow/zero-to-revenue.md'),
    ('P0', 'Apply PitchMI (MEDC)', 'docs/medflow/zero-to-revenue.md'),
    ('P1', 'Written testimonial + case study published', 'docs/MEDFLOW-OS.md#what-success-looks-like-month-12')
) as g(priority, title, doc_path)
where p.slug = 'medflow'
  and not exists (
    select 1 from operator_gates og
    where og.project_id = p.id and og.title = g.title
  );

-- Revenue task: today's build priority
insert into tasks (project_id, title, description, status, priority, revenue_task, due_date)
select
  p.id,
  'Ship encounter capture → transcript API endpoint',
  'First vertical slice of core loop. No PHI until BAA signed — use synthetic audio for dev.',
  'todo',
  'critical',
  true,
  current_date + interval '14 days'
from projects p
where p.slug = 'medflow'
  and not exists (
    select 1 from tasks t
    where t.project_id = p.id and t.title like 'Ship encounter capture%'
  );

-- Quarterly goals
insert into goals (project_id, title, metric_target, status, due_date)
select p.id, g.title, g.metric_target, 'active', g.due_date
from projects p
cross join (
  values
    ('Month 6: 1 paying practice', '1 clinic @ $750 MRR', (current_date + interval '6 months')::date),
    ('Month 12: 5–8 paying practices', '$45–72K ARR', (current_date + interval '12 months')::date),
    ('Month 12: SPARK + grant pipeline active', '1 application submitted', (current_date + interval '12 months')::date)
) as g(title, metric_target, due_date)
where p.slug = 'medflow'
  and not exists (
    select 1 from goals gl where gl.project_id = p.id and gl.title = g.title
  );
