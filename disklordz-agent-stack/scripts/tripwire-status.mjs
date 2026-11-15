#!/usr/bin/env node
import { readFile } from "node:fs/promises";
import { snapshotPath } from "../lib/paths.mjs";
import { writeHermesOut } from "../lib/write.mjs";

let snapshot;
try {
  snapshot = JSON.parse(await readFile(snapshotPath(), "utf8"));
} catch (e) {
  const md = `# Tripwire status\n\nSnapshot missing: ${e.message}\nRun: cd disklordz-command-center && npm run sync:ops\n`;
  console.log(await writeHermesOut("tripwire-status.md", md));
  process.exit(1);
}

const steps = snapshot.tripwireSteps ?? [];
const gates = snapshot.launchGates ?? [];
const p0 = (snapshot.kanbanCards ?? []).filter(
  (c) => c.priority === "P0" && c.column !== "done",
);

const lines = [
  "# Tripwire factory status",
  "",
  `Generated: ${new Date().toISOString()}`,
  "",
  "## Steps G1–G6",
  ...steps.map(
    (s) =>
      `- **${s.id}** ${s.title} (${s.owner}) — **${s.status}**${s.detail ? ` — ${s.detail}` : ""}`,
  ),
  "",
  "## Launch gates",
  ...gates.map((g) => `- ${g.id}: ${g.title} — **${g.status}**`),
  "",
  "## P0 kanban",
  ...p0.map((c) => `- [${c.column}] ${c.title}${c.blocker ? ` — blocked: ${c.blocker}` : ""}`),
  "",
  "## OpenClaw",
  "Telegram: `DRUM:` for kit-spec ingest · Hermes-free mode uses this file only.",
];

const path = await writeHermesOut("tripwire-status.md", lines.join("\n") + "\n");
console.log("Wrote", path);