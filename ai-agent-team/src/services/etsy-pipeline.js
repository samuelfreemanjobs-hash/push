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

function loadStoreConfig() {
  const configPath =
    process.env.ETSY_STORE_CONFIG ||
    path.join(__dirname, '../../config/etsy-store.yaml');
  if (!fs.existsSync(configPath)) {
    return null;
  }
  const raw = fs.readFileSync(configPath, 'utf8');
  const nicheMatch = raw.match(/niche:\s*["']?([^"'\n]+)/i);
  const productTypeMatch = raw.match(/product_type:\s*(\w+)/i);
  return {
    niche: nicheMatch?.[1]?.trim(),
    productType: productTypeMatch?.[1]?.trim(),
    rawPath: configPath,
  };
}

function buildDefaultTask(overrides = {}) {
  const cfg = loadStoreConfig();
  const niche = overrides.niche || cfg?.niche || 'printable planners for busy parents';
  const productType = overrides.productType || cfg?.productType || 'digital';
  return `Run the Etsy automation pipeline for niche: "${niche}". Product focus: ${productType}. Optimize for Etsy search and Pinterest discovery.`;
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
