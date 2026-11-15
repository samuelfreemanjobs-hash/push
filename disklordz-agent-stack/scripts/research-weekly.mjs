#!/usr/bin/env node
/**
 * Free mode: no paid scrape APIs. Writes checklist + manual hooks for Grok/Hermes later.
 */
import { writeHermesOut } from "../lib/write.mjs";

const week = new Date().toISOString().slice(0, 10);

const youtube = `# YouTube competitor spy (free mode)

Week of ${week}

## Manual / Grok (no paid API in this script)

1. Open Grok Drum Kit Architect or Hermes when installed.
2. Channels to check: _(edit USER.md)_
3. Capture: title patterns, $9 pack positioning, gaps DiskLordz can own.

## Paste results

Save notes to \`research/youtube-notes.md\` or export kit-spec for OpenClaw \`DRUM:\`.
`;

const reddit = `# Reddit pain points (free mode)

Week of ${week}

## Subs

- r/trapproduction
- r/makinghiphop

## Free read

Use old.reddit.com or official app — no automation without API keys.

## Output for MKT pod

3 bullets: pain → tripwire angle → avoid repeating prior briefs.
`;

const revenue = `# Revenue opportunities (free mode)

Week of ${week}

## Compare

| Competitor SKU | Price | Pack size | Angle |
|----------------|-------|-----------|-------|

Fill manually or after Hermes research cron with web tools enabled.

## Tripwire

SKU \`tripwire-sample-v1\` — $9, +75 credits, 5 WAVs.
`;

for (const [name, body] of [
  ["research/youtube-weekly.md", youtube],
  ["research/reddit-pain.md", reddit],
  ["research/revenue-opportunities.md", revenue],
]) {
  const p = await writeHermesOut(name, body);
  console.log("Wrote", p);
}