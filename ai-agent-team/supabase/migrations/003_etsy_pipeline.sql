-- Etsy automation pipeline history

create table if not exists etsy_pipeline_runs (
  id uuid primary key default gen_random_uuid(),
  user_id text,
  task text not null,
  routing jsonb,
  results jsonb,
  token_usage jsonb,
  published boolean default false,
  created_at timestamptz default now()
);

create index if not exists etsy_pipeline_runs_user_created
  on etsy_pipeline_runs (user_id, created_at desc);

create table if not exists marketing_tasks (
  id uuid primary key default gen_random_uuid(),
  user_id text,
  task text not null,
  routing jsonb,
  results jsonb,
  token_usage jsonb,
  created_at timestamptz default now()
);
