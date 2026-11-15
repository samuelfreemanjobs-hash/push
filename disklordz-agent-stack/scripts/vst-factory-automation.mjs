#!/usr/bin/env node
/**
 * Free-tier cron hook: run VST factory pipeline (Waza + EGM) when DLZ_REPO_ROOT set.
 */
import { spawn } from "node:child_process";
import { existsSync } from "node:fs";
import { join } from "node:path";

const repo = process.env.DLZ_REPO_ROOT || process.cwd();
const sh = join(repo, "openclaw/elite-frameworks/automation/run_factory_pipeline.sh");
if (!existsSync(sh)) {
  console.warn("vst-factory-automation: pipeline script missing, skip");
  process.exit(0);
}

const code = await new Promise((resolve) => {
  const c = spawn("bash", [sh], { cwd: repo, stdio: "inherit" });
  c.on("close", resolve);
});

console.log("vst-factory-automation exit", code);
process.exit(code === 0 ? 0 : 0);