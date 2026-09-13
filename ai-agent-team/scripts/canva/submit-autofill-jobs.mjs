#!/usr/bin/env node
/**
 * Submit dist/canva-autofill jobs to Canva Connect (brand template autofill).
 *
 * Env: CANVA_ACCESS_TOKEN (OAuth access token with design:write)
 * Config: config/canva-brand-templates.yaml
 */
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import {
  createAutofillJob,
  createDesignExport,
  getAutofillJob,
  getExportJob,
  pollUntilDone,
  toAutofillData,
} from './connect-client.mjs';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const root = path.join(__dirname, '..', '..');

function loadTemplateConfig(rootDir) {
  const jsonPath = path.join(rootDir, 'config', 'canva-brand-templates.json');
  if (!fs.existsSync(jsonPath)) return null;
  const raw = JSON.parse(fs.readFileSync(jsonPath, 'utf8'));
  const templates = {};
  for (const [key, val] of Object.entries(raw)) {
    if (key.startsWith('_')) continue;
    templates[key] = { id: val.brand_template_id, fields: val.fields || {} };
  }
  return templates;
}

function parseArgs(argv) {
  const opts = { dryRun: true, slug: null, asset: null, exportPng: false, limit: 0 };
  for (let i = 2; i < argv.length; i++) {
    const a = argv[i];
    if (a === '--submit') opts.dryRun = false;
    else if (a === '--export') opts.exportPng = true;
    else if (a === '--slug') opts.slug = argv[++i];
    else if (a === '--asset') opts.asset = argv[++i];
    else if (a === '--limit') opts.limit = Number(argv[++i]) || 0;
  }
  return opts;
}

async function main() {
  const opts = parseArgs(process.argv);
  const token = process.env.CANVA_ACCESS_TOKEN;
  const templates = loadTemplateConfig(root);

  if (!templates) {
    console.error('Missing config/canva-brand-templates.json — copy canva-brand-templates.example.json');
    process.exit(1);
  }

  const manifestPath = path.join(root, 'dist', 'canva-autofill', 'manifest.json');
  if (!fs.existsSync(manifestPath)) {
    console.error('Run npm run products:canva-autofill first');
    process.exit(1);
  }

  const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
  let jobs = [];
  for (const product of manifest.products) {
    for (const job of product.jobs) {
      if (opts.slug && job.slug !== opts.slug) continue;
      if (opts.asset && job.asset !== opts.asset) continue;
      jobs.push(job);
    }
  }
  if (opts.limit > 0) jobs = jobs.slice(0, opts.limit);

  console.log(`Jobs: ${jobs.length} · dryRun: ${opts.dryRun}`);

  if (opts.dryRun) {
    for (const job of jobs) {
      const tpl = templates[job.brand_template_key];
      console.log('—', job.title, tpl?.id ? `template ${tpl.id}` : '(no template id in yaml)');
    }
    console.log('\nDry run only. Use --submit with CANVA_ACCESS_TOKEN to create designs.');
    return;
  }

  if (!token) {
    console.error('Set CANVA_ACCESS_TOKEN (see docs/CANVA-AUTOMATION.md)');
    process.exit(1);
  }

  const outDir = path.join(root, 'dist', 'canva-exports');
  fs.mkdirSync(outDir, { recursive: true });
  const results = [];

  for (const job of jobs) {
    const tpl = templates[job.brand_template_key];
    if (!tpl?.id) {
      console.warn('Skip', job.asset, job.slug, '— no brand template id');
      continue;
    }
    const data = toAutofillData(job.fields, tpl.fields);
    console.log('Autofill', job.slug, job.asset);
    const created = await createAutofillJob(token, {
      brandTemplateId: tpl.id,
      title: job.title,
      data,
    });
    const jobId = created?.job?.id || created?.id;
    const done = await pollUntilDone(() => getAutofillJob(token, jobId));
    const designId = done?.job?.result?.design?.id || done?.design?.id;
    const entry = { slug: job.slug, asset: job.asset, designId, jobId };
    if (opts.exportPng && designId) {
      const exp = await createDesignExport(token, { designId, format: 'png' });
      const exportId = exp?.job?.id || exp?.id;
      const expDone = await pollUntilDone(() => getExportJob(token, exportId));
      entry.exportUrl = expDone?.job?.result?.urls?.[0] || expDone?.urls?.[0];
    }
    results.push(entry);
  }

  fs.writeFileSync(path.join(outDir, 'submit-results.json'), JSON.stringify(results, null, 2));
  console.log('Wrote dist/canva-exports/submit-results.json');
}

main().catch((err) => {
  console.error(err.message || err);
  if (err.body) console.error(JSON.stringify(err.body, null, 2));
  process.exit(1);
});
