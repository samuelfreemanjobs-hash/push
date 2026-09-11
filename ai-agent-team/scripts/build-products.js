#!/usr/bin/env node
/**
 * Build Etsy-ready product files and ZIPs in dist/
 */
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import { execSync } from 'child_process';
import XLSX from 'xlsx';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const root = path.join(__dirname, '..');
const dist = path.join(root, 'dist');

function ensureDir(p) {
  fs.mkdirSync(p, { recursive: true });
}

function zipFolder(folderPath, zipPath) {
  const parent = path.dirname(folderPath);
  const name = path.basename(folderPath);
  execSync(`cd "${parent}" && zip -r "${zipPath}" "${name}"`, { stdio: 'inherit' });
}

function buildMarketingCommandCenter() {
  const outDir = path.join(root, 'products', 'marketing-command-center-excel');
  ensureDir(outDir);

  const wb = XLSX.utils.book_new();

  const instructions = [
    ['Marketing Command Center — [YOUR BRAND]'],
    ['Open each tab. Replace sample data. Dashboard pulls from other tabs where noted.'],
    [''],
    ['Tab guide:'],
    ['Dashboard', 'Weekly KPI snapshot'],
    ['Weekly Plan', 'Focus channels and offers this week'],
    ['Content Calendar', 'Posts, emails, assets'],
    ['Campaigns', 'Active campaigns and ROI'],
    ['Lead Sources', 'Where leads come from'],
  ];
  XLSX.utils.book_append_sheet(wb, XLSX.utils.aoa_to_sheet(instructions), 'Start Here');

  const dashboard = [
    ['Metric', 'This Week', 'Last Week', 'Goal', 'Notes'],
    ['Website sessions', 0, 0, 500, ''],
    ['Email subscribers', 0, 0, 50, ''],
    ['Leads', 0, 0, 20, ''],
    ['Sales calls booked', 0, 0, 8, ''],
    ['Revenue ($)', 0, 0, 5000, ''],
    ['Email open rate %', 0, 0, 35, ''],
    ['Social saves', 0, 0, 100, ''],
  ];
  XLSX.utils.book_append_sheet(wb, XLSX.utils.aoa_to_sheet(dashboard), 'Dashboard');

  const weekly = [
    ['Week of', 'Primary offer', 'Channel focus', 'Key message', 'CTA', 'Owner', 'Done?'],
    ['2026-01-06', 'Example offer', 'LinkedIn', 'Outcome hook', 'Book call', '[YOUR NAME]', 'No'],
  ];
  for (let i = 0; i < 11; i++) {
    weekly.push(['', '', '', '', '', '', '']);
  }
  XLSX.utils.book_append_sheet(wb, XLSX.utils.aoa_to_sheet(weekly), 'Weekly Plan');

  const calHeader = ['Date', 'Channel', 'Content type', 'Topic', 'Asset status', 'Link', 'Published'];
  const calendar = [calHeader];
  for (let w = 0; w < 12; w++) {
    for (const ch of ['LinkedIn', 'Email', 'Pinterest']) {
      calendar.push(['', ch, '', '', 'Draft', '', 'No']);
    }
  }
  XLSX.utils.book_append_sheet(wb, XLSX.utils.aoa_to_sheet(calendar), 'Content Calendar');

  const campaigns = [
    ['Campaign', 'Start', 'End', 'Budget', 'Spend', 'Leads', 'Revenue', 'ROI notes'],
    ['Q1 Launch', '', '', 500, 0, 0, 0, ''],
  ];
  XLSX.utils.book_append_sheet(wb, XLSX.utils.aoa_to_sheet(campaigns), 'Campaigns');

  const leads = [
    ['Source', 'Leads MTD', 'Cost per lead', 'Close rate %', 'Notes'],
    ['Organic social', 0, 0, 0, ''],
    ['Paid ads', 0, 0, 0, ''],
    ['Referrals', 0, 0, 0, ''],
    ['Etsy', 0, 0, 0, ''],
    ['Partnerships', 0, 0, 0, ''],
  ];
  XLSX.utils.book_append_sheet(wb, XLSX.utils.aoa_to_sheet(leads), 'Lead Sources');

  const xlsxPath = path.join(outDir, 'Marketing-Command-Center.xlsx');
  XLSX.writeFile(wb, xlsxPath);

  fs.writeFileSync(
    path.join(outDir, 'README.txt'),
    `Marketing Command Center Excel\n\nOpen Marketing-Command-Center.xlsx in Excel or Google Sheets (upload).\nReplace [YOUR BRAND] on Start Here tab.\n`,
  );
  fs.writeFileSync(
    path.join(outDir, 'LICENSE.txt'),
    fs.readFileSync(
      path.join(root, 'products', 'ai-client-onboarding-agent-kit', 'LICENSE.txt'),
      'utf8',
    ),
  );

  console.log('Built', xlsxPath);
}

function buildBusinessInABox() {
  const outDir = path.join(root, 'products', 'business-in-a-box-starter');
  ensureDir(outDir);

  fs.writeFileSync(
    path.join(outDir, 'README-START-HERE.md'),
    `# Business-in-a-Box Starter

This bundle includes:

1. **AI Client Onboarding Agent Kit** — see folder \`onboarding-kit/\` (full copy)
2. **Marketing Command Center** — \`Marketing-Command-Center.xlsx\`
3. **Copywriting templates** — \`50-business-copywriting-templates.md\`

Run \`npm run products:build\` from ai-agent-team to refresh bundled files.

Start with onboarding SOP, then load the Excel dashboard, then use copy templates for your first campaign.
`,
  );

  const onboardingSrc = path.join(root, 'products', 'ai-client-onboarding-agent-kit');
  const onboardingDest = path.join(outDir, 'onboarding-kit');
  execSync(`rm -rf "${onboardingDest}" && cp -R "${onboardingSrc}" "${onboardingDest}"`);

  const mccSrc = path.join(
    root,
    'products',
    'marketing-command-center-excel',
    'Marketing-Command-Center.xlsx',
  );
  if (fs.existsSync(mccSrc)) {
    fs.copyFileSync(mccSrc, path.join(outDir, 'Marketing-Command-Center.xlsx'));
  }

  const copySrc = path.join(root, 'products', 'copywriting-templates', '50-business-copywriting-templates.md');
  if (fs.existsSync(copySrc)) {
    fs.copyFileSync(copySrc, path.join(outDir, '50-business-copywriting-templates.md'));
  }

  fs.copyFileSync(
    path.join(root, 'products', 'ai-client-onboarding-agent-kit', 'LICENSE.txt'),
    path.join(outDir, 'LICENSE.txt'),
  );

  console.log('Built business-in-a-box at', outDir);
}

function main() {
  ensureDir(dist);
  buildMarketingCommandCenter();
  buildBusinessInABox();

  const zips = [
    ['products/ai-client-onboarding-agent-kit', 'ai-client-onboarding-agent-kit.zip'],
    ['products/marketing-command-center-excel', 'marketing-command-center-excel.zip'],
    ['products/postcard-direct-mail-pack', 'postcard-direct-mail-pack.zip'],
    ['products/copywriting-templates', 'business-copywriting-templates.zip'],
    ['products/ai-ops-agent-library', 'ai-ops-agent-library.zip'],
    ['products/business-in-a-box-starter', 'business-in-a-box-starter.zip'],
  ];

  for (const [rel, zipName] of zips) {
    const folder = path.join(root, rel);
    if (!fs.existsSync(folder)) {
      console.warn('Skip zip, missing folder:', rel);
      continue;
    }
    const zipPath = path.join(dist, zipName);
    if (fs.existsSync(zipPath)) fs.unlinkSync(zipPath);
    zipFolder(folder, zipPath);
    console.log('Zipped', zipPath);
  }

  console.log('\nDone. Upload files from dist/ to Etsy digital delivery.');
}

main();
