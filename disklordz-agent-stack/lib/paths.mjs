import { homedir } from "node:os";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const packageRoot = join(
  dirname(fileURLToPath(import.meta.url)),
  "..",
);

function expand(p) {
  return p.startsWith("~") ? join(homedir(), p.slice(1)) : p;
}

export function hermesOutDir() {
  if (process.env.DLZ_HERMES_OUT?.trim()) {
    return expand(process.env.DLZ_HERMES_OUT.trim());
  }
  const state =
    process.env.OPENCLAW_STATE_DIR?.trim() || join(homedir(), ".openclaw");
  return join(expand(state), "hermes-out");
}

export function repoRoot() {
  const fromEnv = process.env.DLZ_REPO_ROOT?.trim();
  if (fromEnv) return expand(fromEnv);
  return join(packageRoot, "..");
}

export function snapshotPath() {
  const custom = process.env.DLZ_OPS_SNAPSHOT?.trim();
  if (custom) return expand(custom);
  return join(
    repoRoot(),
    "disklordz-command-center",
    "public",
    "ops-snapshot.json",
  );
}