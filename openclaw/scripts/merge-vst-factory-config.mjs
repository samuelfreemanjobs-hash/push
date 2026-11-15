#!/usr/bin/env node
/**
 * Write ~/.openclaw/disklordz.vst-factory.merged.json5 from template.
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
  join(here, "..", "openclaw.disklordz.vst-factory.json5.example"),
  "utf8",
);
const outDir = process.env.OPENCLAW_STATE_DIR?.trim() || join(homedir(), ".openclaw");
await mkdir(outDir, { recursive: true });
const skillsDir = join(repo.replace(/\\/g, "/"), "openclaw/skills/shared");
const body = template
  .replaceAll("__REPO_ROOT__", repo.replace(/\\/g, "/"))
  .replaceAll("__SKILLS_SHARED__", skillsDir);
const outPath = join(outDir, "disklordz.vst-factory.merged.json5");
await writeFile(outPath, body, "utf8");
console.log("Wrote", outPath);
console.log("Skills dir:", skillsDir);
console.log(
  "Merge skills + agents.entries + automation.jobs into ~/.openclaw/openclaw.json (backup first).",
);