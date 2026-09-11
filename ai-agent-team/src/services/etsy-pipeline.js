import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import { orchestrate } from '../agents/orchestrator.js';
import { runResearch } from '../agents/research.js';
import { runSEO } from '../agents/seo.js';
import { runSocial } from '../agents/social.js';
import { runAnalytics } from '../agents/analytics.js';
import { runEtsyProduct } from '../agents/etsy-product.js';
import { runEtsyListing } from '../agents/etsy-listing.js';
import { TokenBudget } from '../utils/tokenBudget.js';
import { publishDraftListing } from '../etsy/publish.js';
import { createEtsyClientFromEnv } from '../etsy/client.js';

const __dirname = path.dirname(fileURLToPath(import.meta.url));

function parseYamlScalar(raw, key) {
  const m = raw.match(new RegExp(`${key}:\\s*["']?([^"'\\n]+)`, 'i'));
  return m?.[1]?.trim();
}

function parseYamlList(raw, key) {
  const block = raw.match(new RegExp(`${key}:\\s*\\n((?:  - .+\\n)+)`, 'i'));
  if (!block) return [];
  return block[1]
    .split('\n')
    .map((line) => line.replace(/^\s*-\s*/, '').trim())
    .filter(Boolean);
}

export function loadStoreConfig() {
  const configPath =
    process.env.ETSY_STORE_CONFIG ||
    path.join(__dirname, '../../config/etsy-store.yaml');
  if (!fs.existsSync(configPath)) {
    return null;
  }
  const raw = fs.readFileSync(configPath, 'utf8');
  return {
    niche: parseYamlScalar(raw, 'niche'),
    productType: parseYamlScalar(raw, 'product_type'),
    positioning: parseYamlScalar(raw, 'positioning'),
    buyer: parseYamlScalar(raw, 'buyer'),
    priceBandUsd: parseYamlScalar(raw, 'price_band_usd'),
    brandVoice: parseYamlScalar(raw, 'brand_voice'),
    catalogFocus: parseYamlList(raw, 'catalog_focus'),
    rawPath: configPath,
  };
}

function buildDefaultTask(overrides = {}) {
  const cfg = loadStoreConfig();
  const niche =
    overrides.niche ||
    cfg?.niche ||
    'business tools and AI agent outcome kits';
  const productType = overrides.productType || cfg?.productType || 'digital';
  const catalog = cfg?.catalogFocus?.length
    ? `Catalog lines: ${cfg.catalogFocus.join('; ')}.`
    : '';
  const positioning = cfg?.positioning
    ? `Positioning: ${cfg.positioning}.`
    : 'Sell outcomes, not vague prompts.';
  const buyer = cfg?.buyer ? `Target buyer: ${cfg.buyer}.` : '';
  const price = cfg?.priceBandUsd ? `Price band USD: ${cfg.priceBandUsd}.` : '';

  return [
    `Run the Etsy automation pipeline for niche: "${niche}".`,
    `Product focus: ${productType} instant downloads.`,
    positioning,
    buyer,
    price,
    catalog,
    'Optimize for Etsy search, LinkedIn discovery, and Pinterest saves.',
    'Avoid trademarked tool names in titles; use generic descriptors (spreadsheet dashboard, prompt library, SOP pack).',
  ]
    .filter(Boolean)
    .join(' ');
}

export async function runEtsyPipeline({
  task,
  niche,
  productType,
  publish = false,
  tokenBudget = 12000,
  userId,
  persist,
}) {
  const effectiveTask = task || buildDefaultTask({ niche, productType });
  const budget = new TokenBudget(tokenBudget);
  const results = {};

  const routing = await orchestrate(effectiveTask, budget);
  const shouldPublish = publish || routing.publish === true;

  if (routing.agents.includes('research')) {
    results.research = await runResearch(effectiveTask, budget);
  }

  let productPlan = null;
  if (routing.agents.includes('product')) {
    productPlan = await runEtsyProduct(effectiveTask, results.research, budget);
    results.product = productPlan;
  }

  if (routing.agents.includes('listing')) {
    const idx = productPlan?.recommended ?? 0;
    const selected =
      productPlan?.products?.[idx] || productPlan?.products?.[0] || null;
    results.listing = await runEtsyListing(
      `Create listing for: ${selected?.name || effectiveTask}`,
      selected || productPlan,
      budget,
    );
  }

  if (routing.agents.includes('seo')) {
    const listingTitle = results.listing?.title || effectiveTask;
    results.seo = await runSEO(
      `Refine Etsy title and 13 tags for: ${listingTitle}. Return JSON with title and tags.`,
      budget,
    );
  }

  if (routing.agents.includes('social')) {
    const ctx = JSON.stringify(
      { listing: results.listing, seo: results.seo },
      null,
      2,
    );
    results.social = await runSocial(
      '7-day Pinterest + Instagram plan to drive Etsy visits',
      ctx,
      budget,
    );
  }

  if (routing.agents.includes('analytics')) {
    results.analytics = await runAnalytics(
      'Baseline KPIs for a new Etsy shop in this niche (traffic, conversion, AOV)',
      budget,
    );
  }

  let publishResult = null;
  if (shouldPublish && results.listing && !results.listing.parseError) {
    try {
      publishResult = await publishDraftListing(results.listing);
      results.publish = publishResult;
    } catch (err) {
      results.publish = { error: err.message, details: err.details };
    }
  }

  const tokenSummary = budget.summary();

  if (persist && userId) {
    await persist({
      userId,
      task: effectiveTask,
      routing: routing.agents,
      results,
      tokenSummary,
      published: !!publishResult?.created,
    });
  }

  return {
    success: true,
    task: effectiveTask,
    plan: routing.plan,
    agentsUsed: routing.agents,
    publishAttempted: shouldPublish,
    results,
    tokenUsage: tokenSummary,
  };
}

export async function fetchShopSnapshot() {
  const client = createEtsyClientFromEnv();
  const shop = await client.getShop();
  const listings = await client.getActiveListings(10);
  const weekAgo = Math.floor(Date.now() / 1000) - 7 * 86400;
  const receipts = await client.getReceipts({ minCreated: weekAgo, limit: 25 });

  return {
    shop: shop.results ?? shop,
    activeListings: listings.results ?? listings,
    recentReceipts: receipts.results ?? receipts,
  };
}
