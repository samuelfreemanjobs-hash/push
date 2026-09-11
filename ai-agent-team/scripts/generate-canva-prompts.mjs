#!/usr/bin/env node
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import { CANVA_GLOBAL, CANVA_PRODUCTS } from './agents/canva-art-data.mjs';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const root = path.join(__dirname, '..');
const outDir = path.join(root, 'docs', 'canva-prompts');
const indexPath = path.join(root, 'docs', 'CANVA-ART-PROMPTS.md');

function promptBlock(product, assetKey, title, size, imagePrompt, textOverlay) {
  return `### ${title}
**Size:** ${size}

**Canva → Magic Media / Text to Image** (paste as one prompt):

\`\`\`
${imagePrompt}
\`\`\`

**Text overlay** (add in Canva after; keep inside safe zone):

${textOverlay}
`;
}

function renderProduct(p) {
  const { stylePrefix } = CANVA_GLOBAL;
  const accentNote = `Accent color ${p.accent} for icons, underlines, and CTA button.`;
  const baseImage = `${stylePrefix} ${accentNote} Visual theme: ${p.metaphor}. Product: ${p.name}. Mood: trustworthy, outcome-focused, not hypey AI glitter.`;

  const heroOverlay = `- **Headline:** ${p.headline}
- **Subhead:** ${p.subhead}
- **Badge:** Instant Digital Download
- **Footer small:** ${CANVA_GLOBAL.brandPlaceholder}`;

  const insideList = p.inside.map((i) => `- ${i}`).join('\n');
  const galleryOverlay = `- **Title:** What's Inside
${insideList}
- **Corner badge:** v2.1 · Scorecard + Artifacts`;

  const pinterestOverlay = `- **Pin title:** ${p.headline}
- **Subline:** ${p.subhead}
- **CTA strip:** Shop ${CANVA_GLOBAL.brandPlaceholder} on Etsy`;

  const salesOverlay = `- **H1:** ${p.headline}
- **H2:** ${p.subhead}
- **3 bullets:** ${p.bullets.join(' · ')}
- **CTA button text:** Get instant access
- **Trust line:** Commercial use license · Not a raw prompt dump`;

  const thumbOverlay = `- **Large 3–4 words only:** ${p.headline.split(' ').slice(0, 4).join(' ')}
- **Tiny:** ${p.price !== 'Bundle' ? p.price : 'Bundle save'}`;

  const sections = [
    `# Canva art prompts — ${p.name}`,
    '',
    `**Slug / ZIP:** \`${p.slug}\` · **Price ref:** ${p.price}`,
    '',
    'Copy prompts into **Canva → Create a design → Magic Media**, or use **Magic Design** with the text blocks below.',
    '',
    '---',
    '',
    '## Brand kit (reuse every design)',
    '',
    `- ${CANVA_GLOBAL.stylePrefix}`,
    `- **Fonts:** ${CANVA_GLOBAL.fonts}`,
    `- **Accent:** ${p.accent}`,
    '',
    '---',
    '',
    '## 1. Etsy listing hero (image 1)',
    '',
    promptBlock(
      p,
      'etsyHero',
      'Primary listing image',
      CANVA_GLOBAL.sizes.etsyHero,
      `${baseImage} Composition: centered device or document mock showing abstract workflow folders (playbook, workflows, scorecard). Bold empty headline band across top 20%. No tiny unreadable text in the artwork.`,
      heroOverlay,
    ),
    '',
    '## 2. Etsy gallery — What\'s inside (image 2)',
    '',
    promptBlock(
      p,
      'gallery',
      'Inside the download',
      CANVA_GLOBAL.sizes.etsyGallery,
      `${baseImage} Composition: isometric folder explosion with 5 labeled tabs (Playbook, Workflows, Examples, Scorecard, Artifacts). Clean icons only; labels can be generic. Light shadow.`,
      galleryOverlay,
    ),
    '',
    '## 3. Etsy gallery — How it works (image 3)',
    '',
    promptBlock(
      p,
      'how',
      '3-step flow',
      CANVA_GLOBAL.sizes.etsyGallery,
      `${baseImage} Composition: three steps left-to-right with arrows — (1) Paste workflow (2) Run your AI tool (3) Ship with scorecard. Minimal line icons.`,
      `- **Step 1:** Copy workflow brief
- **Step 2:** Run in your AI tool
- **Step 3:** Score ≥85 · then publish`,
    ),
    '',
    '## 4. Etsy gallery — Verticals (image 4)',
    '',
    promptBlock(
      p,
      'verticals',
      'Works for your niche',
      CANVA_GLOBAL.sizes.etsyGallery,
      `${baseImage} Composition: three equal columns with simple icons — shopping bag (Etsy seller), speech bubble (coach), map pin (local service). Soft column headers area at top.`,
      `- **Col 1:** Etsy digital seller
- **Col 2:** Coach / consultant
- **Col 3:** Local service
- **Note:** See folder \`08-verticals/\` in download`,
    ),
    '',
    '## 5. Pinterest pin',
    '',
    promptBlock(
      p,
      'pinterest',
      'Traffic pin',
      CANVA_GLOBAL.sizes.pinterest,
      `${baseImage} Vertical composition: top 40% bold color block ${p.accent}, bottom 60% light workspace flat lay with laptop and checklist. Space for large title text in upper block.`,
      pinterestOverlay,
    ),
    '',
    '## 6. Sales page / Gumroad hero',
    '',
    promptBlock(
      p,
      'sales',
      'Landing hero',
      CANVA_GLOBAL.sizes.salesHero,
      `${baseImage} Wide cinematic: left third text-safe solid #F7F5F0, right two thirds abstract product UI mock. Subtle gradient. Professional SaaS landing vibe.`,
      salesOverlay,
    ),
    '',
    '## 7. Sales page — Feature strip',
    '',
    `**Size:** ${CANVA_GLOBAL.sizes.salesWide}

**Layout prompt (Canva Magic Design):**

> Wide feature section, 3 icons in a row on off-white background, accent ${p.accent}. Icons represent: ${p.bullets.join(', ')}. Short headings only; no paragraph text in generated art.

**Text:**
${p.bullets.map((b, i) => `- **Feature ${i + 1}:** ${b}`).join('\n')}`,
    '',
    '## 8. Thumbnail-safe variant (Etsy crop)',
    '',
    promptBlock(
      p,
      'thumb',
      'Bold thumbnail',
      CANVA_GLOBAL.sizes.thumb,
      `${baseImage} Composition: single large symbolic icon for ${p.metaphor}, very minimal background, high contrast, no small text in image. Designed to read at 500px.`,
      thumbOverlay,
    ),
    '',
    '## 9. Magic Design one-liner (optional)',
    '',
    'Paste into **Canva Magic Design** with template search *Etsy digital product* or *ebook mockup*:',
    '',
    `\`\`\`
Create an Etsy digital product listing image set for "${p.name}". ${p.subhead}. Style: modern, ${p.accent} accent, off-white, navy text. Include: hero, what's inside checklist, 3-step how it works. Headline: "${p.headline}".
\`\`\``,
    '',
  ];

  return sections.join('\n');
}

function renderIndex() {
  const lines = [
    '# Canva art prompts — all products',
    '',
    'Copy-paste **image prompts** into Canva Magic Media and **text overlays** into your template. One file per SKU in [`canva-prompts/`](./canva-prompts/).',
    '',
    '```bash',
    'npm run products:canva-prompts   # regenerate from scripts/agents/canva-art-data.mjs',
    '```',
    '',
    '## Global style',
    '',
    CANVA_GLOBAL.stylePrefix,
    '',
    `**Fonts:** ${CANVA_GLOBAL.fonts}`,
    '',
    '## Sizes cheat sheet',
    '',
    '| Use | Dimensions |',
    '|-----|------------|',
    ...Object.entries(CANVA_GLOBAL.sizes).map(([k, v]) => `| ${k} | ${v} |`),
    '',
    '## Product index',
    '',
    '| Product | File |',
    '|---------|------|',
  ];

  for (const p of CANVA_PRODUCTS) {
    lines.push(`| ${p.name} | [${p.slug}.md](./canva-prompts/${p.slug}.md) |`);
  }

  lines.push(
    '',
    '## Canva workflow tips',
    '',
    '1. **Magic Media:** Use section prompts verbatim; regenerate until safe zone is clear.',
    '2. **Text:** Add headlines in Canva (do not bake long text into AI images — Etsy mobile).',
    '3. **Consistency:** Same off-white + navy + product accent across all 5 Etsy images.',
    '4. **Bundles:** Use bundle file for hero; crop agent icons from individual kits for collage slide.',
    '5. **Video:** Pair image 1 with `09-video-scripts/loom-15-second.md` from each agent kit.',
    '',
    'See also [ETSY-KEYWORD-PASS.md](./ETSY-KEYWORD-PASS.md) for listing titles and [PRODUCT-ROADMAP-IMPROVEMENTS.md](./PRODUCT-ROADMAP-IMPROVEMENTS.md).',
    '',
  );

  return lines.join('\n');
}

function main() {
  fs.mkdirSync(outDir, { recursive: true });
  for (const p of CANVA_PRODUCTS) {
    const file = path.join(outDir, `${p.slug}.md`);
    fs.writeFileSync(file, renderProduct(p));
    console.log('Wrote', path.relative(root, file));
  }
  fs.writeFileSync(indexPath, renderIndex());
  console.log('Wrote', path.relative(root, indexPath));
  console.log(`\n${CANVA_PRODUCTS.length} products → docs/canva-prompts/`);
}

main();
