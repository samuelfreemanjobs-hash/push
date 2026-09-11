#!/usr/bin/env node
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import { CANVA_GLOBAL, CANVA_PRODUCTS } from './agents/canva-art-data.mjs';
import { baseImagePrompt, buildProductCanvaAssets } from './agents/canva-prompt-lib.mjs';

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
  const { text, images } = buildProductCanvaAssets(p);
  const baseImage = baseImagePrompt(p);

  const heroOverlay = `- **Headline:** ${text.headline}
- **Subhead:** ${text.subhead}
- **Badge:** ${text.badge}
- **Footer small:** ${text.brand}`;

  const galleryOverlay = `- **Title:** What's Inside
${p.inside.map((i) => `- ${i}`).join('\n')}
- **Corner badge:** v2.1 · Scorecard + Artifacts`;

  const pinterestOverlay = `- **Pin title:** ${text.pin_title}
- **Subline:** ${text.pin_subline}
- **CTA strip:** Shop ${text.brand} on Etsy`;

  const salesOverlay = `- **H1:** ${text.headline}
- **H2:** ${text.subhead}
- **3 bullets:** ${p.bullets.join(' · ')}
- **CTA button text:** ${text.cta}
- **Trust line:** ${text.trust_line}`;

  const thumbOverlay = `- **Large 3–4 words only:** ${text.thumb_short}
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
      images.etsy_hero,
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
      images.etsy_inside,
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
      images.etsy_how,
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
      images.etsy_verticals,
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
      images.pinterest,
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
      images.sales_hero,
      salesOverlay,
    ),
    '',
    '## 7. Sales page — Feature strip',
    '',
    `**Size:** ${CANVA_GLOBAL.sizes.salesWide}

**Layout prompt (Canva Magic Design):**

> ${images.feature_strip}

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
      images.thumb,
      thumbOverlay,
    ),
    '',
    '## 9. Magic Design one-liner (optional)',
    '',
    'Paste into **Canva Magic Design** with template search *Etsy digital product* or *ebook mockup*:',
    '',
    `\`\`\`
${text.magic_design_one_liner}
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
    'npm run products:canva-prompts    # markdown prompts',
    'npm run products:canva-autofill   # CSV + API job JSON',
    'npm run products:canva:submit     # dry-run Canva Connect',
    '```',
    '',
    '**Full automation guide:** [CANVA-AUTOMATION.md](./CANVA-AUTOMATION.md)',
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
