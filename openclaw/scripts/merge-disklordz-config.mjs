#!/usr/bin/env node
/**
 * Write ~/.openclaw/disklordz.merged.json5 from template (does not overwrite openclaw.json).
 */
import { readFile, writeFile, mkdir } from "node:fs/promises";
import { homedir } from "node:os";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const here = dirname(fileURLToPath(import.meta.url));
const repo =
  process.env.DLZ_REPO_ROOT?.trim() ||
  resolve(join(here, "..", ".."));
const template = await readFile(
  join(here, "..", "openclaw.disklordz.telegram.json5.example"),
  "utf8",
);
const outDir = process.env.OPENCLAW_STATE_DIR?.trim() || join(homedir(), ".openclaw");
await mkdir(outDir, { recursive: true });
const body = template.replaceAll("__REPO_ROOT__", repo.replace(/\\/g, "/"));
const outPath = join(outDir, "disklordz.merged.json5");
await writeFile(outPath, body, "utf8");
console.log("Wrote", outPath);
console.log(
  "Merge into openclaw.json via Control UI, or paste sections after `openclaw doctor` backup.",
);