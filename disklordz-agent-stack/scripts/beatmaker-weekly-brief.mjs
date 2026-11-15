#!/usr/bin/env node
/**
 * Weekly Beatmaker Content Brief — intake + MKT prompt (Monday 10:00).
 * Step 1 research is completed by MKT with web/browser tools; this script
 * loads catalog, research targets, and writes the fill-in brief scaffold.
 */
import { writeHermesOut } from "../lib/write.mjs";
import {
  CONCEPT_TYPES,
  VIDEO_BRIEF_SECTIONS,
  loadProductCatalog,
} from "../lib/beatmaker-brief.mjs";

function mondayDateLabel(d = new Date()) {
  return d.toISOString().slice(0, 10);
}

function catalogTableMd(catalog) {
  const rows = catalog.products.map((p) => {
    const presets = (p.presets_sounds ?? [])
      .map((s) => s.label)
      .join("; ");
    return `| \`${p.id}\` | ${p.type} | ${p.name} | ${(p.genres ?? []).join(", ")} | ${presets} |`;
  });
  return [
    "| id | type | name | genres | presets/sounds |",
    "|----|------|------|--------|----------------|",
    ...rows,
  ].join("\n");
}

function researchChecklistMd(catalog) {
  const r = catalog.research;
  return `### YouTube (past ${r.lookback_days} days)
${r.youtube_queries.map((q) => `- [ ] Search: \`${q}\` — capture 1 URL + 1 trend note`).join("\n")}

### TikTok / Reels hashtags
${r.tiktok_hashtags.map((h) => `- [ ] #${h}`).join("\n")}

### Reddit
${r.reddit_subs.map((s) => `- [ ] r/${s} — top pain or technique this week (cite URL)`).join("\n")}`;
}

function conceptShell(type, catalog) {
  const rules = catalog.matching_rules?.[type.id] ?? {};
  return `## ${type.label}

${type.description}

**Matching hint:** ${JSON.stringify(rules)}

**Catalog product id:** _(required — from table)_
**Sourced trend / pain:** _(URL + one sentence)_

${VIDEO_BRIEF_SECTIONS.map((s) => `### ${s}\n\n_(fill)_\n`).join("\n")}`;
}

function buildBriefMarkdown({ catalog, date, subject }) {
  const cta = catalog.offers.free_entry;
  return `# ${subject}

Generated: ${new Date().toISOString()}
Task: ${catalog.delivery.task_name}

You are **MKT** completing this brief for DiskLordz. Use **only** products/presets from the catalog table. Cite sources for trends.

## Step 1 — Live trend & pain research

${researchChecklistMd(catalog)}

Record findings in \`research-notes\` below before writing concepts.

## Step 2 — Product catalog (do not invent)

${catalogTableMd(catalog)}

**Default CTA offer:** ${cta.label} (\`${cta.id}\`)

## Step 3 & 4 — Three vertical video briefs (15–30s each)

${CONCEPT_TYPES.map((t) => conceptShell(t, catalog)).join("\n---\n\n")}

## Research notes (sourced)

| Source | Trend / pain | Product id candidate |
|--------|--------------|----------------------|
| | | |

## Delivery

- Subject: \`${subject}\`
- Telegram: ${catalog.delivery.channels.telegram.enabled ? "yes — `MKT:` brief, max 20 lines summary + attach full md" : "off"}
- Slack: ${catalog.delivery.channels.slack.enabled ? catalog.delivery.channels.slack.channel : "off"}
- Email: ${catalog.delivery.channels.email.enabled ? "on" : "off"}

---
_OpenClaw: \`MKT: complete Monday beatmaker brief from hermes-out intake\`_
`;
}

async function main() {
  const catalog = loadProductCatalog();
  const date = mondayDateLabel();
  const subject = catalog.delivery.subject_template.replace("{date}", date);

  const intake = {
    generated_at: new Date().toISOString(),
    date,
    subject,
    catalog_version: catalog.version,
    research: catalog.research,
    products: catalog.products.map((p) => ({
      id: p.id,
      type: p.type,
      name: p.name,
      genres: p.genres,
      solves: p.solves,
      presets_sounds: p.presets_sounds,
    })),
    concept_types: CONCEPT_TYPES,
    delivery: catalog.delivery,
  };

  const body = buildBriefMarkdown({ catalog, date, subject });

  const intakePath = await writeHermesOut(
    "research/beatmaker-brief/intake.json",
    intake,
    { json: true },
  );
  const briefPath = await writeHermesOut(
    `research/beatmaker-brief/BEATMAKER-BRIEF-${date}.md`,
    body,
  );
  await writeHermesOut(
    "research/beatmaker-brief/LATEST.md",
    body,
  );

  console.log("Wrote", intakePath);
  console.log("Wrote", briefPath);
  console.log("Subject:", subject);
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});