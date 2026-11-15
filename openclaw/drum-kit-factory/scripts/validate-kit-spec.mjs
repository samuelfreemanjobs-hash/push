#!/usr/bin/env node
/**
 * Validates kit-spec.yaml from Grok Drum Kit Architect before OpenClaw ingest.
 * Usage: node scripts/validate-kit-spec.mjs path/to/kit-spec.yaml
 */
import { readFileSync } from "node:fs";
import { parse as parseYaml } from "yaml";

const path = process.argv[2];
if (!path) {
  console.error("Usage: node scripts/validate-kit-spec.mjs <kit-spec.yaml>");
  process.exit(2);
}

const raw = readFileSync(path, "utf8");
let spec;
try {
  spec = parseYaml(raw);
} catch (e) {
  console.error("YAML parse error:", e.message);
  process.exit(1);
}

const errors = [];

function req(field, pred, msg) {
  if (!pred) errors.push(msg ?? `missing or invalid: ${field}`);
}

req("kit_id", typeof spec.kit_id === "string" && spec.kit_id.length > 0);
req("version", Number.isInteger(spec.version) && spec.version >= 1);
req(
  "architect.system",
  spec.architect?.system === "grok-bot-drum-kit-architect",
  'architect.system must be "grok-bot-drum-kit-architect"',
);
req("genre", typeof spec.genre === "string");
req("bpm", typeof spec.bpm === "number" && spec.bpm >= 40 && spec.bpm <= 220);
req("sounds", Array.isArray(spec.sounds) && spec.sounds.length > 0);
req("qc.format", spec.qc?.format === "wav");
req("tripwire.sku", typeof spec.tripwire?.sku === "string");
req(
  "founder_ears_required",
  spec.founder_ears_required === true,
  "founder_ears_required must be true for tripwire",
);

if (Array.isArray(spec.sounds)) {
  spec.sounds.forEach((s, i) => {
    if (!s?.id || !s?.role || !s?.description) {
      errors.push(`sounds[${i}] needs id, role, description`);
    }
  });
}

if (errors.length) {
  console.error("Invalid kit-spec:");
  errors.forEach((e) => console.error(" -", e));
  process.exit(1);
}

console.log("OK", spec.kit_id, "v" + spec.version, "→", spec.tripwire?.sku);
process.exit(0);