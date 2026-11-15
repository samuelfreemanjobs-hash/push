#!/usr/bin/env node
/**
 * Factory milestone M1 aggregate verify (CI / Linux OK).
 * See disklordz-ralph/FACTORY-MILESTONE.md
 */
import { existsSync } from "node:fs";
import { spawnSync } from "node:child_process";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const here = dirname(fileURLToPath(import.meta.url));
const repoRoot = join(here, "..", "..", "..");

function run(script, arg) {
  const r = spawnSync(process.execPath, [join(here, script), arg], {
    cwd: join(here, ".."),
    encoding: "utf8",
  });
  if (r.status !== 0) {
    process.stderr.write(r.stderr || r.stdout);
    return false;
  }
  return true;
}

function skillExists(name) {
  return existsSync(
    join(repoRoot, "openclaw/skills/shared", name, "SKILL.md"),
  );
}

let ok = true;

ok = run("validate-kit-spec.mjs", "handoff/kit-spec.template.yaml") && ok;
ok = run("emit-factory-work-order.mjs", "handoff/kit-spec.template.yaml") && ok;
ok =
  run("validate-qc-manifest.mjs", "handoff/qc-manifest.template.json") && ok;

for (const skill of ["kit_spec_grok", "tripwire_qc", "vst_factory"]) {
  if (!skillExists(skill)) {
    console.error("Missing skill:", skill);
    ok = false;
  }
}

if (!existsSync(join(repoRoot, "disklordz-ralph/FACTORY-MILESTONE.md"))) {
  console.error("Missing FACTORY-MILESTONE.md");
  ok = false;
}

if (ok) {
  console.log("Factory milestone M1 — automation spine OK (see Windows DLZ-VST-002 for pluginval)");
  process.exit(0);
}
process.exit(1);