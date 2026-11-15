#!/usr/bin/env node
/**
 * Emits a markdown factory work order from a validated kit-spec.yaml.
 * Usage: node scripts/emit-factory-work-order.mjs path/to/kit-spec.yaml
 */
import { readFileSync } from "node:fs";
import { parse as parseYaml } from "yaml";
import { spawnSync } from "node:child_process";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const kitPath = process.argv[2];
if (!kitPath) {
  console.error("Usage: node emit-factory-work-order.mjs <kit-spec.yaml>");
  process.exit(2);
}

const here = dirname(fileURLToPath(import.meta.url));
const validate = spawnSync(
  process.execPath,
  [join(here, "validate-kit-spec.mjs"), kitPath],
  { encoding: "utf8" },
);
if (validate.status !== 0) {
  process.stderr.write(validate.stderr || validate.stdout);
  process.exit(validate.status ?? 1);
}

const spec = parseYaml(readFileSync(kitPath, "utf8"));
const lines = [
  `# Factory work order — ${spec.kit_id} v${spec.version}`,
  "",
  `- **SKU:** ${spec.tripwire?.sku ?? "—"}`,
  `- **Genre / BPM:** ${spec.genre} @ ${spec.bpm}`,
  `- **Architect:** ${spec.architect?.system ?? "—"}`,
  `- **Founder ears (G3):** ${spec.founder_ears_required ? "required" : "—"}`,
  "",
  "## Sounds",
  "",
];

for (const s of spec.sounds ?? []) {
  lines.push(`- [ ] \`${s.id}\` (${s.role}) — ${s.description}`);
}

lines.push(
  "",
  "## QC pipeline",
  "",
  "- [ ] WAV format per `qc.format`",
  "- [ ] Run `validate-qc-manifest.mjs` after WAV export",
  "- [ ] `qc_status: approved` in manifest before pack",
  "- [ ] tripwire_pack skill → zip + manifest hash",
  "",
  "## Handoff",
  "",
  "- OpenClaw: `DRUM:` → drum-lead",
  "- Kanban: tripwire-sample-v1 / K-012",
  ""
);

process.stdout.write(lines.join("\n"));