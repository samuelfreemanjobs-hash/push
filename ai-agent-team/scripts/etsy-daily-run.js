#!/usr/bin/env node
/**
 * Cron-friendly CLI: market research → product → listing → marketing.
 * Usage: node scripts/etsy-daily-run.js [--publish]
 */
import dotenv from 'dotenv';
import { runEtsyPipeline } from '../src/services/etsy-pipeline.js';

dotenv.config();

const publish = process.argv.includes('--publish');
const niche = process.env.ETSY_NICHE;
const productType = process.env.ETSY_PRODUCT_TYPE || 'digital';

async function main() {
  const result = await runEtsyPipeline({
    niche,
    productType,
    publish,
    tokenBudget: 14000,
  });
  console.log(JSON.stringify(result, null, 2));
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});
