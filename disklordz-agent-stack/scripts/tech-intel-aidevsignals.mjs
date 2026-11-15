#!/usr/bin/env node
/**
 * Ingest AI Dev Signals (Beehiiv RSS or pasted inbox) → hermes-out tech-radar intake.
 * Does not call paid LLMs; OpenClaw RADAR agent fills the report via tech_radar_signals skill.
 */
import { readFile } from "node:fs/promises";
import { join } from "node:path";
import {
  CATEGORIES,
  DISKLORDZ_PRODUCTS,
  SOURCE,
} from "../lib/tech-radar.mjs";
import { hermesOutDir } from "../lib/paths.mjs";
import { writeHermesOut } from "../lib/write.mjs";

const FEED_URL =
  process.env.AI_DEV_SIGNALS_FEED_URL ?? SOURCE.defaultFeedUrl;
const FETCH_TIMEOUT_MS = Number(process.env.DLZ_FETCH_TIMEOUT_MS ?? 15_000);

function decodeEntities(s) {
  return s
    .replace(/&amp;/g, "&")
    .replace(/&lt;/g, "<")
    .replace(/&gt;/g, ">")
    .replace(/&quot;/g, '"')
    .replace(/&#39;/g, "'");
}

/** Minimal RSS/Atom item parse — no XML dependency */
function parseFeedItems(xml) {
  const items = [];
  const itemRe = /<item>([\s\S]*?)<\/item>/gi;
  let m;
  while ((m = itemRe.exec(xml)) !== null) {
    const block = m[1];
    const title = block.match(/<title>(?:<!\[CDATA\[)?([\s\S]*?)(?:\]\]>)?<\/title>/i);
    const link = block.match(/<link>([\s\S]*?)<\/link>/i);
    const pub = block.match(/<pubDate>([\s\S]*?)<\/pubDate>/i);
    const guid = block.match(/<guid[^>]*>([\s\S]*?)<\/guid>/i);
    if (!title) continue;
    items.push({
      title: decodeEntities(title[1].trim()),
      link: link ? decodeEntities(link[1].trim()) : null,
      pubDate: pub ? pub[1].trim() : null,
      guid: guid ? decodeEntities(guid[1].trim()) : null,
    });
  }
  if (items.length > 0) return items;

  const entryRe = /<entry>([\s\S]*?)<\/entry>/gi;
  while ((m = entryRe.exec(xml)) !== null) {
    const block = m[1];
    const title = block.match(/<title[^>]*>([\s\S]*?)<\/title>/i);
    const link = block.match(/<link[^>]+href="([^"]+)"/i);
    const updated = block.match(/<updated>([\s\S]*?)<\/updated>/i);
    if (!title) continue;
    items.push({
      title: decodeEntities(title[1].trim()),
      link: link ? link[1].trim() : null,
      pubDate: updated ? updated[1].trim() : null,
      guid: null,
    });
  }
  return items;
}

async function fetchFeed(url) {
  const ctrl = new AbortController();
  const t = setTimeout(() => ctrl.abort(), FETCH_TIMEOUT_MS);
  try {
    const res = await fetch(url, {
      signal: ctrl.signal,
      headers: { Accept: "application/rss+xml, application/xml, text/xml, */*" },
    });
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    return await res.text();
  } finally {
    clearTimeout(t);
  }
}

async function readInbox() {
  const base = hermesOutDir();
  const candidates = [
    process.env.DLZ_TECH_INTEL_INBOX,
    join(base, "research/tech-radar/inbox/issue.md"),
    join(base, "research/tech-radar/inbox/issue.txt"),
  ].filter(Boolean);
  for (const p of candidates) {
    try {
      const body = await readFile(p, "utf8");
      if (body.trim()) return { path: p, body: body.trim() };
    } catch {
      /* try next */
    }
  }
  return null;
}

function categoryTableMd() {
  const rows = CATEGORIES.map(
    (c) => `| ${c.label} | ${c.watchFor} |`
  ).join("\n");
  return `| Category | What to watch for |\n|----------|-------------------|\n${rows}`;
}

function productsListMd() {
  return DISKLORDZ_PRODUCTS.filter((p) => p.id !== "none")
    .map((p) => `- **${p.label}** — ${p.examples}`)
    .join("\n");
}

function buildReviewPrompt({ issueTitle, issueUrl, inboxPath, feedError }) {
  const iso = new Date().toISOString();
  const date = iso.slice(0, 10);
  return `# Technology Intelligence — ${SOURCE.name}

Generated: ${iso}
Issue: ${issueTitle ?? "(paste inbox or fix RSS)"}
${issueUrl ? `URL: ${issueUrl}` : ""}
${inboxPath ? `Inbox: ${inboxPath}` : ""}
${feedError ? `\n> RSS note: ${feedError}\n` : ""}

You are **RADAR**, DiskLordz Technology Intelligence Agent. Do **not** archive every article. Produce a **short screening report** only for items that might change a product or workflow.

## Rubric

${categoryTableMd()}

## DiskLordz products (assign \`product_fit\`)

${productsListMd()}

## Per interesting item (0–6 entries total across the issue)

For each item worth keeping, output one block:

\`\`\`yaml
headline: ""
source_url: ""
category: audio_ai | coding_ai | dsp_research | local_models | agent_frameworks | commercial_opportunity
technology_summary: ""
licensing: ""
integration_difficulty: 1-5
expected_cost: ""
product_fit: vst_factory | tripwire_packs | command_center | agent_stack | none
improves: ""
verdict: adopt | watch | skip
next_action: null or one concrete step
\`\`\`

## Issue summary (required)

- **Top signal (one sentence):**
- **Adopt now (0–2):**
- **Watch list (0–3):**
- **Explicit skips:** what you ignored and why (brevity)

## Operator

Save completed report to \`~/.openclaw/hermes-out/research/tech-radar/report-${date}.md\`
Hand one line to SAGE morning brief if any \`verdict: adopt\`.
`;
}

async function main() {
  const generatedAt = new Date().toISOString();
  const inbox = await readInbox();
  let feedError = null;
  let items = [];
  try {
    const xml = await fetchFeed(FEED_URL);
    items = parseFeedItems(xml);
  } catch (e) {
    feedError = e instanceof Error ? e.message : String(e);
  }

  const latest = items[0] ?? null;
  const intake = {
    source: SOURCE,
    feed_url: FEED_URL,
    generated_at: generatedAt,
    feed_error: feedError,
    inbox_path: inbox?.path ?? null,
    latest_issue: latest,
    recent_items: items.slice(0, 8),
    categories: CATEGORIES.map(({ id, label, watchFor }) => ({
      id,
      label,
      watchFor,
    })),
  };

  const reviewBody = buildReviewPrompt({
    issueTitle: inbox ? "Pasted inbox issue" : latest?.title,
    issueUrl: latest?.link,
    inboxPath: inbox?.path,
    feedError,
  });

  const intakePath = await writeHermesOut(
    "research/tech-radar/intake.json",
    intake,
    { json: true }
  );
  const reviewPath = await writeHermesOut(
    `research/tech-radar/REVIEW-${generatedAt.slice(0, 10)}.md`,
    reviewBody
  );

  if (inbox) {
    await writeHermesOut("research/tech-radar/inbox/latest-body.md", inbox.body);
  }

  const stubPath = await writeHermesOut(
    "research/tech-radar/README.md",
    `# Technology Intelligence (AI Dev Signals)

Hermes-out path: \`${hermesOutDir()}/research/tech-radar/\`

## Daily flow

1. \`npm run tech-radar\` (this script) — RSS + optional inbox
2. OpenClaw: \`RADAR: review latest AI Dev Signals issue\`
3. Output: \`report-YYYY-MM-DD.md\` (screened items only)

## Paste fallback (no RSS)

Save the newsletter body to:

\`${join(hermesOutDir(), "research/tech-radar/inbox/issue.md")}\`

Or set \`DLZ_TECH_INTEL_INBOX\` to any file path.

## Source

[AI Dev Signals](${SOURCE.site}) — ${SOURCE.publisher}
`
  );

  console.log("Wrote", intakePath);
  console.log("Wrote", reviewPath);
  console.log("Wrote", stubPath);
  if (feedError && !inbox) {
    console.warn(
      "RSS fetch failed; paste issue to",
      join(hermesOutDir(), "research/tech-radar/inbox/issue.md")
    );
  }
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});