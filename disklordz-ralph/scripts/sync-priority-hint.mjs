#!/usr/bin/env node
import { readFile } from "node:fs/promises";
import { join, dirname } from "node:path";
import { fileURLToPath } from "node:url";

const repoRoot = join(dirname(fileURLToPath(import.meta.url)), "..", "..");
const prd = JSON.parse(
  await readFile(join(repoRoot, "disklordz-ralph", "prd.json"), "utf8"),
);
const snapPath = join(
  repoRoot,
  "disklordz-command-center/public/ops-snapshot.json",
);
const snapshot = JSON.parse(await readFile(snapPath, "utf8"));
const open = prd.stories.filter((s) => !s.passes && !s.actor);

const p0 = (snapshot.kanbanCards ?? []).filter(
  (c) => c.priority === "P0" && c.column !== "done",
);

console.log("Kanban P0 (snapshot, not done):");
for (const c of p0) {
  const linked = open.find((s) => s.kanban === c.id);
  console.log(
    `  ${c.id} [${c.column}] ${c.title}${linked ? ` → Ralph ${linked.id}` : ""}`,
  );
}
console.log("\nRalph open (agent):", open.map((s) => s.id).join(", ") || "(none)");