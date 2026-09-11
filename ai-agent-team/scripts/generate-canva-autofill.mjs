#!/usr/bin/env node
/**
 * Export Canva Bulk Create CSV + Connect API autofill job JSON per SKU.
 */
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import { CANVA_PRODUCTS } from './agents/canva-art-data.mjs';
import { buildProductCanvaAssets } from './agents/canva-prompt-lib.mjs';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const root = path.join(__dirname, '..');
const outRoot = path.join(root, 'dist', 'canva-autofill');
const bulkDir = path.join(root, 'dist', 'canva-bulk');

const CSV_COLUMNS = [
  'slug',
  'product_name',
  'headline',
  'subhead',
  'bullet_1',
  'bullet_2',
  'bullet_3',
  'price',
  'brand',
  'accent_hex',
  'inside_list',
  'badge',
  'prompt_etsy_hero',
  'prompt_etsy_inside',
  'prompt_etsy_how',
  'prompt_etsy_verticals',
  'prompt_pinterest',
  'prompt_sales_hero',
  'prompt_thumb',
];

function csvEscape(value) {
  const s = String(value ?? '').replace(/\r?\n/g, ' ').replace(/"/g, '""');
  return `"${s}"`;
}

function buildRow(p) {
  const { text, images } = buildProductCanvaAssets(p);
  return {
    slug: p.slug,
    product_name: text.product_name,
    headline: text.headline,
    subhead: text.subhead,
    bullet_1: text.bullet_1,
    bullet_2: text.bullet_2,
    bullet_3: text.bullet_3,
    price: text.price,
    brand: text.brand,
    accent_hex: text.accent_hex,
    inside_list: text.inside_list,
    badge: text.badge,
    prompt_etsy_hero: images.etsy_hero,
    prompt_etsy_inside: images.etsy_inside,
    prompt_etsy_how: images.etsy_how,
    prompt_etsy_verticals: images.etsy_verticals,
    prompt_pinterest: images.pinterest,
    prompt_sales_hero: images.sales_hero,
    prompt_thumb: images.thumb,
  };
}

const ASSET_KEYS = [
  'etsy_hero',
  'etsy_inside',
  'etsy_how',
  'etsy_verticals',
  'pinterest',
  'sales_hero',
  'thumb',
];

function main() {
  fs.mkdirSync(outRoot, { recursive: true });
  fs.mkdirSync(bulkDir, { recursive: true });

  const manifest = {
    generated_at: new Date().toISOString(),
    product_count: CANVA_PRODUCTS.length,
    brand_template_config: 'config/canva-brand-templates.json (copy from .example.json)',
    assets: ASSET_KEYS,
    products: [],
  };

  const csvLines = [CSV_COLUMNS.join(',')];

  for (const p of CANVA_PRODUCTS) {
    const row = buildRow(p);
    csvLines.push(CSV_COLUMNS.map((col) => csvEscape(row[col])).join(','));
    const { text, images } = buildProductCanvaAssets(p);

    const productEntry = { slug: p.slug, name: p.name, jobs: [] };
    for (const asset of ASSET_KEYS) {
      const job = {
        slug: p.slug,
        asset,
        title: `${p.name} — ${asset.replace(/_/g, ' ')}`,
        brand_template_key: asset,
        fields: { ...text },
        image_prompt: images[asset],
        export: { format: 'png', width: asset === 'pinterest' ? 1000 : 2000, height: asset === 'pinterest' ? 1500 : 2000 },
      };
      productEntry.jobs.push(job);
      const jobDir = path.join(outRoot, 'jobs', p.slug);
      fs.mkdirSync(jobDir, { recursive: true });
      fs.writeFileSync(path.join(jobDir, `${asset}.json`), JSON.stringify(job, null, 2));
    }
    manifest.products.push(productEntry);
  }

  fs.writeFileSync(path.join(outRoot, 'manifest.json'), JSON.stringify(manifest, null, 2));
  fs.writeFileSync(path.join(bulkDir, 'bulk-create.csv'), csvLines.join('\n') + '\n');

  console.log('Wrote', path.relative(root, path.join(bulkDir, 'bulk-create.csv')));
  console.log('Wrote', path.relative(root, path.join(outRoot, 'manifest.json')));
  console.log(`${CANVA_PRODUCTS.length} products × ${ASSET_KEYS.length} jobs → dist/canva-autofill/jobs/`);
}

main();
