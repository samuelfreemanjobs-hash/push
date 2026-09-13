import { CANVA_GLOBAL } from './canva-art-data.mjs';

export function baseImagePrompt(product) {
  const accentNote = `Accent color ${product.accent} for icons, underlines, and CTA button.`;
  return `${CANVA_GLOBAL.stylePrefix} ${accentNote} Visual theme: ${product.metaphor}. Product: ${product.name}. Mood: trustworthy, outcome-focused, not hypey AI glitter.`;
}

const COMPOSITIONS = {
  etsyHero:
    'Composition: centered device or document mock showing abstract workflow folders (playbook, workflows, scorecard). Bold empty headline band across top 20%. No tiny unreadable text in the artwork.',
  etsyInside:
    'Composition: isometric folder explosion with 5 labeled tabs (Playbook, Workflows, Examples, Scorecard, Artifacts). Clean icons only; labels can be generic. Light shadow.',
  etsyHow:
    'Composition: three steps left-to-right with arrows — (1) Paste workflow (2) Run your AI tool (3) Ship with scorecard. Minimal line icons.',
  etsyVerticals:
    'Composition: three equal columns with simple icons — shopping bag (Etsy seller), speech bubble (coach), map pin (local service). Soft column headers area at top.',
  pinterest:
    'Vertical composition: top 40% bold color block ACCENT, bottom 60% light workspace flat lay with laptop and checklist. Space for large title text in upper block.',
  salesHero:
    'Wide cinematic: left third text-safe solid #F7F5F0, right two thirds abstract product UI mock. Subtle gradient. Professional SaaS landing vibe.',
  thumb:
    'Composition: single large symbolic icon for THEME, very minimal background, high contrast, no small text in image. Designed to read at 500px.',
};

/**
 * Structured prompts + text fields for markdown docs, CSV bulk create, and Canva Connect autofill.
 */
export function buildProductCanvaAssets(product) {
  const base = baseImagePrompt(product);
  const brand = CANVA_GLOBAL.brandPlaceholder;

  const text = {
    product_name: product.name,
    slug: product.slug,
    price: product.price,
    headline: product.headline,
    subhead: product.subhead,
    bullet_1: product.bullets[0] ?? '',
    bullet_2: product.bullets[1] ?? '',
    bullet_3: product.bullets[2] ?? '',
    brand,
    badge: 'Instant Digital Download',
    inside_list: product.inside.join('\n'),
    accent_hex: product.accent,
    cta: 'Get instant access',
    trust_line: 'Commercial use license · Not a raw prompt dump',
    pin_title: product.headline,
    pin_subline: product.subhead,
    thumb_short: product.headline.split(' ').slice(0, 4).join(' '),
    magic_design_one_liner: `Create an Etsy digital product listing image set for "${product.name}". ${product.subhead}. Style: modern, ${product.accent} accent, off-white, navy text. Include: hero, what's inside checklist, 3-step how it works. Headline: "${product.headline}".`,
  };

  const pinterestComp = COMPOSITIONS.pinterest.replace('ACCENT', product.accent);
  const thumbComp = COMPOSITIONS.thumb.replace('THEME', product.metaphor);

  const images = {
    etsy_hero: `${base} ${COMPOSITIONS.etsyHero}`,
    etsy_inside: `${base} ${COMPOSITIONS.etsyInside}`,
    etsy_how: `${base} ${COMPOSITIONS.etsyHow}`,
    etsy_verticals: `${base} ${COMPOSITIONS.etsyVerticals}`,
    pinterest: `${base} ${pinterestComp}`,
    sales_hero: `${base} ${COMPOSITIONS.salesHero}`,
    thumb: `${base} ${thumbComp}`,
    feature_strip: `Wide feature section, 3 icons in a row on off-white background, accent ${product.accent}. Icons represent: ${product.bullets.join(', ')}. Short headings only; no paragraph text in generated art.`,
  };

  return { text, images, sizes: CANVA_GLOBAL.sizes };
}
