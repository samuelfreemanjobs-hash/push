#!/usr/bin/env node
import { spawn } from "node:child_process";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";

const dir = dirname(fileURLToPath(import.meta.url));
const scripts = [
  "backup-ops-sync.mjs",
  "engineering-report.mjs",
  "tripwire-status.mjs",
  "vst-factory-automation.mjs",
];

for (const name of scripts) {
  const code = await new Promise((resolve) => {
    const c = spawn(process.execPath, [join(dir, name)], { stdio: "inherit" });
    c.on("close", resolve);
  });
  if (code !== 0) console.warn(name, "exited", code);
}

async function run(name) {
  return new Promise((resolve) => {
    const c = spawn(process.execPath, [join(dir, name)], { stdio: "inherit" });
    c.on("close", resolve);
  });
}

const day = new Date().getUTCDay();
if (day === 5) await run("biz-metrics.mjs");
if (day === 1) await run("research-weekly.mjs");