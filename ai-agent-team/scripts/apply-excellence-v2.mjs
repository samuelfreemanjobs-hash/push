#!/usr/bin/env node
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import { ALL_AGENT_SLUGS } from './agent-slugs.mjs';
import { EXCELLENCE_V2, CHANGELOG_V2, VERSION } from './agents/excellence-v2-data.mjs';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const root = path.join(__dirname, '..');
const products = path.join(root, 'products');

const README_V2_BLOCK = `

## v2.0 — Excellence pack

| Folder / file | Purpose |
|---------------|---------|
| \`04-examples/\` | Fictional **Acme** sample so you see a finished result |
| \`06-scorecard.md\` | Score **≥85/100** before you ship to customers |
| \`07-artifacts/\` | Fillable CSV/Markdown — start without AI |
| \`CHANGELOG.md\` | Version history |

**Version:** see \`VERSION.txt\`.
`;

export function applyExcellenceV2(slugs = [...ALL_AGENT_SLUGS, 'ai-client-onboarding-agent-kit']) {
  for (const slug of slugs) {
    const data =
      EXCELLENCE_V2[slug] ||
      (slug === 'ai-client-onboarding-agent-kit'
        ? EXCELLENCE_V2['agent-04-client-onboarding-agent-kit']
        : null);
    if (!data) {
      console.warn('No excellence data for', slug);
      continue;
    }
    const base = path.join(products, slug);
    if (!fs.existsSync(base)) {
      console.warn('Missing kit folder', slug);
      continue;
    }

    const examplesDir = path.join(base, '04-examples');
    fs.mkdirSync(examplesDir, { recursive: true });
    fs.writeFileSync(
      path.join(examplesDir, 'README.md'),
      `# Examples\n\nOpen \`acme-sample.md\` to see what a strong finished run looks like. Replace with your own after running workflows.\n`,
    );
    fs.writeFileSync(path.join(examplesDir, 'acme-sample.md'), data.example);

    fs.writeFileSync(path.join(base, '06-scorecard.md'), data.scorecard);

    const artifactPath = path.join(base, data.artifact.name);
    fs.mkdirSync(path.dirname(artifactPath), { recursive: true });
    fs.writeFileSync(artifactPath, data.artifact.content);

    fs.writeFileSync(path.join(base, 'CHANGELOG.md'), CHANGELOG_V2);
    fs.writeFileSync(path.join(base, 'VERSION.txt'), VERSION);

    const readmePath = path.join(base, 'README-START-HERE.md');
    if (fs.existsSync(readmePath)) {
      let readme = fs.readFileSync(readmePath, 'utf8');
      if (!readme.includes('v2.0 — Excellence pack')) {
        readme += README_V2_BLOCK;
        fs.writeFileSync(readmePath, readme);
      }
    }

    console.log('Excellence v2 applied:', slug);
  }
}

const isMain =
  process.argv[1] && path.resolve(process.argv[1]) === path.resolve(fileURLToPath(import.meta.url));

if (isMain) {
  applyExcellenceV2();
}
