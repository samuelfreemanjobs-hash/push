#!/usr/bin/env node
/**
 * Merge disklordz.vst-factory.merged.json5 into ~/.openclaw/openclaw.json (with .bak).
 */
import { readFile, writeFile, copyFile, mkdir } from "node:fs/promises";
import { existsSync } from "node:fs";
import { homedir } from "node:os";
import { join } from "node:path";
import JSON5 from "json5";

const state = process.env.OPENCLAW_STATE_DIR?.trim() || join(homedir(), ".openclaw");
const configPath = join(state, "openclaw.json");
const patchPath = join(state, "disklordz.vst-factory.merged.json5");

function deepMerge(target, source) {
  if (source === null || typeof source !== "object" || Array.isArray(source)) {
    return source ?? target;
  }
  const out = { ...target };
  for (const key of Object.keys(source)) {
    const sv = source[key];
    const tv = out[key];
    if (
      sv &&
      typeof sv === "object" &&
      !Array.isArray(sv) &&
      tv &&
      typeof tv === "object" &&
      !Array.isArray(tv)
    ) {
      out[key] = deepMerge(tv, sv);
    } else if (sv !== undefined) {
      out[key] = sv;
    }
  }
  return out;
}

await mkdir(state, { recursive: true });
if (!existsSync(patchPath)) {
  console.error("Missing", patchPath, "— run merge-vst-factory-config.mjs first");
  process.exit(1);
}

const patch = JSON5.parse(await readFile(patchPath, "utf8"));
let base = {};
if (existsSync(configPath)) {
  const raw = await readFile(configPath, "utf8");
  base = raw.trim() ? JSON5.parse(raw) : {};
  await copyFile(configPath, `${configPath}.bak.${Date.now()}`);
}

const merged = deepMerge(base, patch);
await writeFile(configPath, `${JSON.stringify(merged, null, 2)}\n`, "utf8");
console.log("Wrote", configPath);