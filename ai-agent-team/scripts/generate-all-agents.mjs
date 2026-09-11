#!/usr/bin/env node
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import { LICENSE, workflowFile } from './agent-kit-shared.js';
import { AGENT_KITS } from './agents/top10-definitions.mjs';
import { AGENT_KITS_11_20 } from './agents/market-top20-11-20.mjs';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const root = path.join(__dirname, '..');
const products = path.join(root, 'products');

const MASTER_INDEX = [
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
  ['11', 'Customer Support Reply', 'agent-11-customer-support-reply-agent-kit'],
  ['12', 'SOP & Operations Doc', 'agent-12-sop-operations-doc-agent-kit'],
  ['13', 'Lead Magnet & Opt-in', 'agent-13-lead-magnet-opt-in-agent-kit'],
  ['14', 'Direct Mail Campaign', 'agent-14-direct-mail-campaign-agent-kit'],
  ['15', 'Brand Voice & Messaging', 'agent-15-brand-voice-messaging-agent-kit'],
  ['16', 'Blog & SEO Article', 'agent-16-blog-seo-article-agent-kit'],
  ['17', 'Business-in-a-Box Orchestrator', 'agent-17-business-in-a-box-orchestrator-agent-kit'],
  ['18', 'AI Ops & Admin', 'agent-18-ai-ops-admin-agent-kit'],
  ['19', 'Etsy Shop Analytics', 'agent-19-etsy-shop-analytics-agent-kit'],
  ['20', 'Coach & Consultant Professional', 'agent-20-coach-consultant-professional-agent-kit'],
];

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

function attachMarketingExcel(slug) {
  const xlsxSrc = path.join(products, 'marketing-command-center-excel', 'Marketing-Command-Center.xlsx');
  const xlsxDest = path.join(products, slug, '03-templates', 'Marketing-Command-Center.xlsx');
  if (fs.existsSync(xlsxSrc)) {
    fs.mkdirSync(path.dirname(xlsxDest), { recursive: true });
    fs.copyFileSync(xlsxSrc, xlsxDest);
  }
}

function attachPostcardPack() {
  const src = path.join(products, 'postcard-direct-mail-pack');
  const dest = path.join(products, 'agent-14-direct-mail-campaign-agent-kit', '03-templates', 'postcard-direct-mail-pack');
  if (fs.existsSync(src)) {
    fs.rmSync(dest, { recursive: true, force: true });
    fs.cpSync(src, dest, { recursive: true });
  }
}

function attachAiOpsLibrary() {
  const src = path.join(products, 'ai-ops-agent-library', '50-ai-ops-agent-workflows.md');
  const dest = path.join(products, 'agent-18-ai-ops-admin-agent-kit', '03-templates', '50-ai-ops-agent-workflows.md');
  if (fs.existsSync(src)) {
    fs.mkdirSync(path.dirname(dest), { recursive: true });
    fs.copyFileSync(src, dest);
  }
}

function attachBusinessInABoxReadme() {
  const readme = `# Business-in-a-Box files

Run \`npm run products:build\` to refresh the full \`business-in-a-box-starter\` ZIP.

This orchestrator agent (kit 17) tells you *when* to deploy onboarding, marketing, and copy SKUs—not a duplicate of every file.
`;
  writeFile('agent-17-business-in-a-box-orchestrator-agent-kit/03-templates/BUNDLE-README.md', readme);
}

function writeMasterIndex() {
  const lines = [
    '# Top 20 AI Agent Kits — Master index',
    '',
    'Market-demand catalog (agents 1–20). Run `npm run products:build` for ZIPs.',
    '',
    '| # | Agent | Folder |',
    '|---|--------|--------|',
  ];
  for (const [n, name, folder] of MASTER_INDEX) {
    lines.push(`| ${n} | ${name} | \`${folder}/\` |`);
  }
  lines.push('', 'Bundle: `top-20-ai-agents-bundle.zip`');
  writeFile('top-20-ai-agents-bundle/README-START-HERE.md', lines.join('\n'));
  writeFile('top-20-ai-agents-bundle/LICENSE.txt', LICENSE);
}

function main() {
  for (const kit of AGENT_KITS) {
    if (kit.slug === 'agent-03-marketing-planner-agent-kit') continue;
    if (kit.slug === 'agent-04-client-onboarding-agent-kit') continue;
    materializeKit(kit);
  }
  const kit03 = AGENT_KITS.find((k) => k.slug === 'agent-03-marketing-planner-agent-kit');
  if (kit03) materializeKit(kit03);
  attachMarketingExcel('agent-03-marketing-planner-agent-kit');

  for (const kit of AGENT_KITS_11_20) {
    materializeKit(kit);
  }

  copyOnboardingKit();
  attachPostcardPack();
  attachAiOpsLibrary();
  attachBusinessInABoxReadme();
  writeMasterIndex();
  console.log('All 20 agent kits generated.');
}

const isMain =
  process.argv[1] && path.resolve(process.argv[1]) === path.resolve(fileURLToPath(import.meta.url));

if (isMain) {
  main();
}
