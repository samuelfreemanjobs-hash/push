/**
 * Writes public/intelligence-snapshot.json for Command Center Intelligence panel.
 * Merges repo opportunity stack + Ralph factory-m1 status + optional hermes-out tech-radar.
 */
import { readFile, writeFile, mkdir } from "node:fs/promises";
import { existsSync } from "node:fs";
import { homedir } from "node:os";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
import { OPPORTUNITIES } from "../../disklordz-agent-stack/lib/opportunities.mjs";
import { FACTORY_M1 } from "../../disklordz-agent-stack/lib/factory-milestone.mjs";
import { CATEGORIES, SOURCE } from "../../disklordz-agent-stack/lib/tech-radar.mjs";

const __dir = dirname(fileURLToPath(import.meta.url));
const ccRoot = join(__dir, "..");
const repoRoot = join(ccRoot, "..");
const outPath = join(ccRoot, "public", "intelligence-snapshot.json");

function hermesOut() {
  const fromEnv = process.env.OPENCLAW_STATE_DIR?.trim();
  if (fromEnv) return join(fromEnv, "hermes-out");
  return join(homedir(), ".openclaw", "hermes-out");
}

async function readJsonMaybe(path) {
  try {
    return JSON.parse(await readFile(path, "utf8"));
  } catch {
    return null;
  }
}

async function factoryMilestoneStatus() {
  const prd = await readJsonMaybe(join(repoRoot, "disklordz-ralph", "prd.json"));
  const stories = prd?.stories ?? [];
  const byId = new Map(stories.map((s) => [s.id, s]));
  return {
    ...FACTORY_M1,
    stories: FACTORY_M1.story_ids.map((id) => {
      const s = byId.get(id);
      return {
        id,
        title: s?.title ?? id,
        passes: Boolean(s?.passes),
        milestone: s?.milestone ?? null,
      };
    }),
    complete: FACTORY_M1.story_ids.every((id) => byId.get(id)?.passes),
  };
}

async function techRadarFromHermes() {
  const base = hermesOut();
  const intake = await readJsonMaybe(
    join(base, "research/tech-radar/intake.json"),
  );
  const inboxPath = join(base, "research/tech-radar/inbox/issue.md");
  const inboxPending = existsSync(inboxPath);
  return {
    generated_at: intake?.generated_at ?? null,
    feed_error: intake?.feed_error ?? null,
    latest_issue: intake?.latest_issue ?? null,
    recent_items: intake?.recent_items?.slice(0, 6) ?? [],
    inbox_pending: inboxPending,
    feed_url: intake?.feed_url ?? SOURCE.defaultFeedUrl,
  };
}

async function main() {
  const generatedAt = new Date().toISOString();
  const snapshot = {
    generatedAt,
    agent: {
      id: "radar",
      prefix: "RADAR",
      displayName: "Technology Intelligence",
      source: SOURCE.name,
      sourceUrl: SOURCE.site,
      skill: "tech_radar_signals",
      doc: "disklordz-agent-stack/TECH-RADAR.md",
    },
    categories: CATEGORIES.map(({ id, label, watchFor }) => ({
      id,
      label,
      watchFor,
    })),
    opportunities: OPPORTUNITIES,
    factoryMilestone: await factoryMilestoneStatus(),
    techRadar: await techRadarFromHermes(),
    quickPrompts: [
      "RADAR: review latest AI Dev Signals issue",
      "RADAR: screen for factory M1 — opportunities 1–3 only",
      "RADAR: adopt list for immediate opportunities 1–5",
    ],
  };

  await mkdir(dirname(outPath), { recursive: true });
  await writeFile(outPath, `${JSON.stringify(snapshot, null, 2)}\n`, "utf8");
  console.log("Wrote", outPath);
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});