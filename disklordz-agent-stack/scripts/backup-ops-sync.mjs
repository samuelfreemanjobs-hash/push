#!/usr/bin/env node
/**
 * Free GitHub Actions backup: refresh ops-snapshot if older than max age.
 */
import { readFile, stat } from "node:fs/promises";
import { spawn } from "node:child_process";
import { join } from "node:path";
import { repoRoot, snapshotPath } from "../lib/paths.mjs";
import { writeHermesOut } from "../lib/write.mjs";

const MAX_AGE_H = Number(process.env.DLZ_SNAPSHOT_MAX_AGE_H ?? 8);

let ageH = Infinity;
let meta = null;
try {
  const st = await stat(snapshotPath());
  ageH = (Date.now() - st.mtimeMs) / 36e5;
  meta = JSON.parse(await readFile(snapshotPath(), "utf8"));
} catch {
  /* missing */
}

const syncErrors = meta?.meta?.syncErrors ?? [];
const needsRun = ageH > MAX_AGE_H || syncErrors.length > 0;

let result = { needsRun, ageH, ran: false, exitCode: null };

if (needsRun) {
  const syncDir = join(repoRoot(), "disklordz-command-center");
  const syncScript = join(syncDir, "scripts", "sync-ops-snapshot.mjs");
  const intelScript = join(syncDir, "scripts", "sync-intelligence-snapshot.mjs");
  result.ran = true;
  result.exitCode = await new Promise((resolve) => {
    const child = spawn(process.execPath, [syncScript], {
      cwd: syncDir,
      stdio: "inherit",
      env: { ...process.env, DLZ_REPO_ROOT: repoRoot() },
    });
    child.on("close", (code) => {
      if (code !== 0) resolve(code ?? 1);
      else {
        const intel = spawn(process.execPath, [intelScript], {
          cwd: syncDir,
          stdio: "inherit",
          env: { ...process.env, DLZ_REPO_ROOT: repoRoot() },
        });
        intel.on("close", (c) => resolve(c ?? 1));
      }
    });
  });
}

await writeHermesOut(
  "events/backup-sync.json",
  { ...result, at: new Date().toISOString() },
  { json: true },
);

if (result.ran) {
  console.log("backup-sync exit", result.exitCode);
  process.exit(result.exitCode === 0 ? 0 : 1);
}
console.log("backup-sync skip (fresh snapshot)");