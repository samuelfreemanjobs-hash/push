#!/usr/bin/env node
/**
 * Etsy Product Systems Director v3.3 — catalog audit + build pipeline runner.
 * Usage:
 *   node etsy-department/agents/etsy-director-agent.mjs --list-listings
 *   node etsy-department/agents/etsy-director-agent.mjs --run
 */
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import { execSync } from 'child_process';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const deptRoot = path.join(__dirname, '..');
const appRoot = path.join(deptRoot, '..');
const listingsDir = path.join(deptRoot, 'Etsy Listings');
const productsRoot = path.join(appRoot, 'products');

const STANDALONE_SKUS = [
  { id: 'ai-client-onboarding-agent-kit', title: 'AI Client Onboarding Agent Kit', zip: 'ai-client-onboarding-agent-kit.zip' },
  { id: 'marketing-command-center-excel', title: 'Marketing Command Center Excel', zip: 'marketing-command-center-excel.zip' },
  { id: 'postcard-direct-mail-pack', title: 'Postcard Direct-Mail Pack', zip: 'postcard-direct-mail-pack.zip' },
  { id: 'copywriting-templates', title: 'Business Copywriting Templates', zip: 'business-copywriting-templates.zip' },
  { id: 'ai-ops-agent-library', title: 'AI Ops Agent Library', zip: 'ai-ops-agent-library.zip' },
  { id: 'business-in-a-box-starter', title: 'Business-in-a-Box Starter', zip: 'business-in-a-box-starter.zip' },
];

function parseArgs(argv) {
  return {
    list: argv.includes('--list-listings'),
    run: argv.includes('--run') || (!argv.includes('--list-listings') && argv.length <= 2),
  };
}

function readAgentTitle(profilePath) {
  if (!fs.existsSync(profilePath)) return null;
  const text = fs.readFileSync(profilePath, 'utf8');
  const m = text.match(/^#\s*Agent profile\s*[—–-]\s*(.+)/m);
  return m ? m[1].trim() : null;
}

function discoverAgentKits() {
  if (!fs.existsSync(productsRoot)) return [];
  return fs
    .readdirSync(productsRoot, { withFileTypes: true })
    .filter((d) => d.isDirectory() && /^agent-\d{2}-/.test(d.name))
    .sort()
    .map((d) => {
      const folder = d.name;
      const profile = path.join(productsRoot, folder, 'AGENT-PROFILE.md');
      const versionPath = path.join(productsRoot, folder, 'VERSION.txt');
      const title = readAgentTitle(profile) || folder;
      const version = fs.existsSync(versionPath) ? fs.readFileSync(versionPath, 'utf8').trim() : 'unknown';
      const zipName = `${folder}.zip`;
      const zipPath = path.join(appRoot, 'dist', zipName);
      return {
        product_id: folder,
        title,
        version,
        folder: `products/${folder}`,
        zip_path: `dist/${zipName}`,
        zip_ready: fs.existsSync(zipPath),
        listing_file: path.join(listingsDir, `${folder}_LISTING.md`),
      };
    });
}

function ensureListingsManifest(agents) {
  fs.mkdirSync(listingsDir, { recursive: true });
  for (const a of agents) {
    if (fs.existsSync(a.listing_file)) continue;
    const body = `---
product_id: ${a.product_id}
title: ${a.title}
version: ${a.version}
price_usd: 19.99
zip_path: ${a.zip_path}
tags: []
status: draft
store: Store A — AI Agent Kits
---

# ${a.title}

> Auto-generated listing stub by Etsy Product Systems Director v3.3. Edit copy in \`docs/ETSY-KEYWORD-PASS.md\` and rebuild.

## Deliverable

- Folder: \`${a.folder}\`
- ZIP target: \`${a.zip_path}\`

## Upload checklist

- [ ] ZIP built (\`npm run products:build\`)
- [ ] 13 Etsy tags
- [ ] 5–8 listing images (Canva)
- [ ] Digital download file attached
`;
    fs.writeFileSync(a.listing_file, body, 'utf8');
  }
}

function printListings(agents) {
  console.log('\n=== Etsy Product Systems Director v3.3 — publish catalog ===\n');
  console.log('Store: AI Agent Kits (Top 20 + standalone SKUs)\n');
  console.log('| # | product_id | title | version | ZIP ready | listing md |');
  console.log('|---|------------|-------|---------|-----------|------------|');
  agents.forEach((a, i) => {
    const listing = fs.existsSync(a.listing_file) ? 'yes' : 'no';
    console.log(
      `| ${i + 1} | ${a.product_id} | ${a.title} | ${a.version} | ${a.zip_ready ? 'yes' : 'no'} | ${listing} |`,
    );
  });

  console.log('\n--- Standalone SKUs ---\n');
  for (const s of STANDALONE_SKUS) {
    const zipPath = path.join(appRoot, 'dist', s.zip);
    console.log(`- ${s.id}: ${s.title} — ZIP ${fs.existsSync(zipPath) ? 'ready' : 'missing'}`);
  }

  const bundles = ['top-10-ai-agents-bundle.zip', 'top-20-ai-agents-bundle.zip'];
  console.log('\n--- Bundles ---\n');
  for (const b of bundles) {
    const p = path.join(appRoot, 'dist', b);
    console.log(`- ${b}: ${fs.existsSync(p) ? 'ready' : 'missing'}`);
  }
}

function runPipeline() {
  const pkgPath = path.join(appRoot, 'package.json');
  const pkg = JSON.parse(fs.readFileSync(pkgPath, 'utf8'));
  const scripts = pkg.scripts || {};

  console.log('\n=== Director RUN — 7-phase pipeline (automated legs) ===\n');

  const phases = [
    { name: '1 Intake / catalog scan', fn: () => discoverAgentKits().length },
    { name: '2 Listing manifest', fn: () => ensureListingsManifest(discoverAgentKits()) },
    { name: '3 Agent kit generate', script: 'agents:generate' },
    { name: '4 Excellence packs', script: 'products:excellence', optional: true },
    { name: '5 v2.1 verticals', script: 'products:v21', optional: true },
    { name: '6 Canva autofill CSV', script: 'products:canva-autofill', optional: true },
    { name: '7 ZIP build', script: 'products:build' },
  ];

  for (const phase of phases) {
    if (phase.fn) {
      const result = phase.fn();
      console.log(`✓ ${phase.name}${result != null ? ` (${result} agents)` : ''}`);
      continue;
    }
    if (!phase.script || !scripts[phase.script]) {
      if (phase.optional) {
        console.log(`○ ${phase.name} — skipped (no npm script)`);
      } else {
        console.log(`✗ ${phase.name} — missing npm script: ${phase.script}`);
      }
      continue;
    }
    console.log(`→ ${phase.name}: npm run ${phase.script}`);
    execSync(`npm run ${phase.script}`, { cwd: appRoot, stdio: 'inherit' });
    console.log(`✓ ${phase.name} complete`);
  }

  printListings(discoverAgentKits());
  console.log('\nDirector RUN finished. Next: human Etsy upload + Canva mockups.\n');
}

const { list, run } = parseArgs(process.argv.slice(2));
const agents = discoverAgentKits();
ensureListingsManifest(agents);

if (list) {
  printListings(agents);
} else if (run) {
  runPipeline();
} else {
  printListings(agents);
}
