#!/usr/bin/env node
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import { LICENSE, workflowFile } from './agent-kit-shared.js';
import { AGENT_KITS } from './agents/top10-definitions.mjs';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const root = path.join(__dirname, '..');
const products = path.join(root, 'products');

function writeFile(relPath, content) {
  const full = path.join(products, relPath);
  fs.mkdirSync(path.dirname(full), { recursive: true });
  fs.writeFileSync(full, content);
}

function materializeKit(kit) {
  const base = kit.slug;
  writeFile(`${base}/LICENSE.txt`, LICENSE);
  writeFile(`${base}/README-START-HERE.md`, kit.readme);
  writeFile(`${base}/AGENT-PROFILE.md`, kit.profile);
  writeFile(`${base}/01-playbook/playbook.md`, kit.playbook);
  writeFile(`${base}/05-implementation/setup-guide.md`, kit.setup);
  kit.workflows.forEach((w, i) => {
    const num = String(i + 1).padStart(2, '0');
    writeFile(
      `${base}/02-workflows/${num}-${w.slug}.md`,
      workflowFile(num, w.title, w.job, w.inputs, w.brief, w.qc),
    );
  });
  if (kit.templates) {
    for (const [name, content] of Object.entries(kit.templates)) {
      writeFile(`${base}/03-templates/${name}`, content);
    }
  }
  console.log('Generated', base);
}

function copyOnboardingKit() {
  const src = path.join(products, 'ai-client-onboarding-agent-kit');
  const dest = path.join(products, 'agent-04-client-onboarding-agent-kit');
  fs.rmSync(dest, { recursive: true, force: true });
  fs.cpSync(src, dest, { recursive: true });
  const profile = `# Agent profile — Client Onboarding

**Outcome:** New clients reach "onboarding complete" in 14 days with zero dropped balls.

**Inputs:** Offer type, brand voice, scheduling link, portal link.

**Outputs:** Welcome emails, intake, kickoff assets, tracker row per client.

**KPI:** % clients with kickoff held within 7 days of payment.

This kit is the full production package (12 workflows + SOP + templates + tracker).
`;
  fs.writeFileSync(path.join(dest, 'AGENT-PROFILE.md'), profile);
  console.log('Copied agent-04-client-onboarding-agent-kit');
}

function writeMarketingPlannerAgent() {
  const slug = 'agent-03-marketing-planner-agent-kit';
  const kit = AGENT_KITS.find((k) => k.slug === slug);
  if (kit) materializeKit(kit);
  const xlsxSrc = path.join(products, 'marketing-command-center-excel', 'Marketing-Command-Center.xlsx');
  const xlsxDest = path.join(products, slug, '03-templates', 'Marketing-Command-Center.xlsx');
  if (fs.existsSync(xlsxSrc)) {
    fs.mkdirSync(path.dirname(xlsxDest), { recursive: true });
    fs.copyFileSync(xlsxSrc, xlsxDest);
  }
}

function writeMasterIndex() {
  const lines = [
    '# Top 10 AI Agent Kits — Master index',
    '',
    'Each folder is a standalone Etsy SKU. Run `npm run products:build` to ZIP all.',
    '',
    '| # | Agent | Folder |',
    '|---|--------|--------|',
  ];
  const order = [
    ['1', 'Etsy Listing SEO', 'agent-01-etsy-listing-seo-agent-kit'],
    ['2', 'Social Content Machine', 'agent-02-social-content-machine-agent-kit'],
    ['3', 'Marketing Planner', 'agent-03-marketing-planner-agent-kit'],
    ['4', 'Client Onboarding', 'agent-04-client-onboarding-agent-kit'],
    ['5', 'Email Sequence', 'agent-05-email-sequence-agent-kit'],
    ['6', 'Ad Copy', 'agent-06-ad-copy-agent-kit'],
    ['7', 'Sales Proposal & Discovery', 'agent-07-sales-proposal-discovery-agent-kit'],
    ['8', 'Copy Swipe File', 'agent-08-copy-swipe-agent-kit'],
    ['9', 'POD Design Prompt', 'agent-09-pod-design-prompt-agent-kit'],
    ['10', 'Listing Mockup & Photo Brief', 'agent-10-listing-mockup-photo-brief-agent-kit'],
  ];
  for (const [n, name, folder] of order) {
    lines.push(`| ${n} | ${name} | \`${folder}/\` |`);
  }
  lines.push('', 'Bundle ZIP: `top-10-ai-agents-bundle.zip` (all agents).');
  writeFile('top-10-ai-agents-bundle/README-START-HERE.md', lines.join('\n'));
  writeFile('top-10-ai-agents-bundle/LICENSE.txt', LICENSE);
}

for (const kit of AGENT_KITS) {
  if (kit.slug === 'agent-03-marketing-planner-agent-kit') continue;
  if (kit.slug === 'agent-04-client-onboarding-agent-kit') continue;
  materializeKit(kit);
}
writeMarketingPlannerAgent();
copyOnboardingKit();
writeMasterIndex();
console.log('Top 10 agent kits generated. Prefer: npm run agents:generate (all 20).');
