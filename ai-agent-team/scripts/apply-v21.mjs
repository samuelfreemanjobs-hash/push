#!/usr/bin/env node
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import { ALL_AGENT_SLUGS } from './agent-slugs.mjs';
import { getV21Pack } from './agents/v21-vertical-loom-data.mjs';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const root = path.join(__dirname, '..');
const products = path.join(root, 'products');

export const VERSION_V21 = '2.1.0';

export const CHANGELOG_V21_APPEND = `
## 2.1.0 — Vertical packs + video scripts + deeper workflows (13–19)
- Added \`08-verticals/\` — Etsy seller, coach/consultant, local service input packs
- Added \`09-video-scripts/loom-15-second.md\` — shop preview script per agent
- Deepened workflows for agents **13–19** (structured deliverables + richer workflow 01 briefs)
`;

const README_V21_BLOCK = `

## v2.1 — Verticals & Loom

| Folder / file | Purpose |
|---------------|---------|
| \`08-verticals/\` | Paste-ready context for **Etsy**, **coach**, or **local** niches |
| \`09-video-scripts/\` | **15-second Loom** script for listing video or Pinterest |

**Version:** see \`VERSION.txt\`.
`;

const SLUGS_13_19 = ALL_AGENT_SLUGS.filter((s) => /^agent-(1[3-9])-/.test(s));

const EXTRA_SLUGS = ['ai-client-onboarding-agent-kit'];

export function applyV21(slugs = [...ALL_AGENT_SLUGS, ...EXTRA_SLUGS]) {
  for (const slug of slugs) {
    const base = path.join(products, slug);
    if (!fs.existsSync(base)) {
      console.warn('Missing kit folder', slug);
      continue;
    }

    const packKey =
      slug === 'ai-client-onboarding-agent-kit' ? 'agent-04-client-onboarding-agent-kit' : slug;
    const pack = getV21Pack(packKey);
    if (pack) {
      const vertDir = path.join(base, '08-verticals');
      fs.mkdirSync(vertDir, { recursive: true });
      fs.writeFileSync(
        path.join(vertDir, 'README.md'),
        `# Vertical packs\n\nPick one file below and paste the block into workflow inputs when you sell to that niche.\n`,
      );
      for (const [name, content] of Object.entries(pack.verticals)) {
        fs.writeFileSync(path.join(vertDir, name), content);
      }

      const videoDir = path.join(base, '09-video-scripts');
      fs.mkdirSync(videoDir, { recursive: true });
      fs.writeFileSync(path.join(videoDir, 'loom-15-second.md'), pack.loom);
    }

    let changelog = fs.existsSync(path.join(base, 'CHANGELOG.md'))
      ? fs.readFileSync(path.join(base, 'CHANGELOG.md'), 'utf8')
      : '# Changelog\n';
    if (!changelog.includes('2.1.0 — Vertical packs')) {
      changelog += CHANGELOG_V21_APPEND;
      fs.writeFileSync(path.join(base, 'CHANGELOG.md'), changelog);
    }

    fs.writeFileSync(path.join(base, 'VERSION.txt'), VERSION_V21);

    const readmePath = path.join(base, 'README-START-HERE.md');
    if (fs.existsSync(readmePath)) {
      let readme = fs.readFileSync(readmePath, 'utf8');
      if (!readme.includes('v2.1 — Verticals & Loom')) {
        readme += README_V21_BLOCK;
        fs.writeFileSync(readmePath, readme);
      }
    }

    console.log('v2.1 applied:', slug);
  }
}

const isMain =
  process.argv[1] && path.resolve(process.argv[1]) === path.resolve(fileURLToPath(import.meta.url));

if (isMain) {
  applyV21();
  console.log('Deepened workflow regen: run npm run agents:generate (includes 13–19).');
}
