#!/usr/bin/env node
/**
 * Emit OpenClaw agent entries from vst-factory-pods.yaml (stdout JSON5 snippet).
 * Usage: node openclaw/scripts/apply-vst-factory-agents.mjs
 */
import { readFile } from "node:fs/promises";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";
import { parse as parseYaml } from "yaml";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");
const podsPath = join(root, "agents", "vst-factory-pods.yaml");
const raw = await readFile(podsPath, "utf8");
const data = parseYaml(raw);
const entries = {};
for (const [id, pod] of Object.entries(data.pods ?? {})) {
  entries[id] = {
    name: id.split("-").pop()?.toUpperCase() ?? id,
    workspace: `~/.openclaw/${pod.workspace}`,
    skills: pod.skills ?? [],
  };
}
const out = {
  agents: { entries },
  delegation_order: data.delegation_order,
  pipeline: data.orchestration?.pipeline,
};
console.log("// Paste under agents in openclaw.json — from vst-factory-pods.yaml\n");
console.log(JSON.stringify(out, null, 2));