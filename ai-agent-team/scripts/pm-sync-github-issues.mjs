#!/usr/bin/env node
/**
 * Refresh pm/backlog.yaml github_issue metadata from `gh issue list`.
 * Does not overwrite manual autonomous_next / status fields.
 */
import { execSync } from 'node:child_process';
import { readFileSync, writeFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import yaml from 'yaml';

const __dirname = dirname(fileURLToPath(import.meta.url));
const backlogPath = join(__dirname, '../pm/backlog.yaml');

const raw = readFileSync(backlogPath, 'utf8');
const doc = yaml.parse(raw);

let issuesJson;
try {
  issuesJson = execSync(
    'gh issue list --repo samuelfreemanjobs-hash/push --state all --limit 30 --json number,title,state',
    { encoding: 'utf8' }
  );
} catch (err) {
  console.error('gh issue list failed:', err.message);
  process.exit(1);
}

const issues = JSON.parse(issuesJson);
const byNumber = new Map(issues.map((i) => [i.number, i]));

for (const tier of ['P0', 'P1', 'P2']) {
  const items = doc.priorities?.[tier] ?? [];
  for (const item of items) {
    const num = item.github_issue;
    if (!num) continue;
    const gh = byNumber.get(num);
    if (gh) {
      item.github_state = gh.state;
      if (!item.title || item.title.length < 20) {
        item.title = gh.title;
      }
    }
  }
}

doc.updated_at = new Date().toISOString().slice(0, 10);
writeFileSync(backlogPath, yaml.stringify(doc), 'utf8');
console.log('Updated', backlogPath);
