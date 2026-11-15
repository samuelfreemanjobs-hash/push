#!/usr/bin/env node
import { readFile } from "node:fs/promises";
import { spawn } from "node:child_process";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const ralphRoot = join(dirname(fileURLToPath(import.meta.url)), "..");
const repoRoot = resolve(ralphRoot, "..");
const id = process.argv[2];

if (!id) {
  console.error("Usage: node verify-story.mjs <story-id>");
  process.exit(2);
}

const prd = JSON.parse(await readFile(join(ralphRoot, "prd.json"), "utf8"));
const story = prd.stories.find((s) => s.id === id);
if (!story) {
  console.error("Unknown story", id);
  process.exit(2);
}

const verify = story.verify ?? {};
if (verify.platform === "windows" && process.platform !== "win32") {
  console.warn(
    `SKIP ${id}: Windows-only verification (run on founder PC in ${story.workspace ?? "VST workspace"})`,
  );
  process.exit(0);
}

const cwd = verify.cwd
  ? join(repoRoot, verify.cwd)
  : story.workspace && process.platform === "win32"
    ? story.workspace
    : repoRoot;

const env = { ...process.env, ...(verify.env ?? {}), DLZ_REPO_ROOT: repoRoot };
const shell = Boolean(verify.shell);
const cmd = verify.cmd;
if (!cmd) {
  console.error("No verify.cmd on story", id);
  process.exit(2);
}

console.log(`[verify] ${id}: ${cmd} (cwd=${cwd})`);

const child = shell
  ? spawn(cmd, { cwd, env, shell: true, stdio: "inherit" })
  : spawn(cmd, { cwd, env, shell: true, stdio: "inherit" });

const code = await new Promise((res) => child.on("close", res));
process.exit(code === 0 ? 0 : 1);