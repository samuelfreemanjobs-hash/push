#!/usr/bin/env node
import { readFile } from "node:fs/promises";
import { readdir } from "node:fs/promises";
import { join } from "node:path";
import {
  INSTRUMENTS,
  VAULT,
  listOpenPrs,
  lastWorkflowConclusion,
} from "../lib/github.mjs";
import { hermesOutDir, snapshotPath } from "../lib/paths.mjs";
import { writeHermesOut } from "../lib/write.mjs";

let snapshot = null;
try {
  snapshot = JSON.parse(await readFile(snapshotPath(), "utf8"));
} catch {
  /* optional */
}

const vaultPrs = await listOpenPrs(VAULT, 50);
const stale = vaultPrs.filter((p) => {
  const updated = new Date(p.updated_at).getTime();
  return Date.now() - updated > 7 * 864e5;
});

const vstGreen =
  snapshot?.vstFleet?.filter((v) => v.ciStatus === "success").length ?? null;
const vstTotal = snapshot?.vstFleet?.length ?? null;

const outDir = hermesOutDir();
let researchFiles = [];
try {
  researchFiles = await readdir(join(outDir, "research"));
} catch {
  researchFiles = [];
}

const metrics = {
  generated_at: new Date().toISOString(),
  mode: "free-tier",
  factory: {
    vst_ci_green: vstGreen,
    vst_sku_count: vstTotal,
    instruments_build: await lastWorkflowConclusion(INSTRUMENTS, "build.yml"),
  },
  launch: {
    gates: snapshot?.launchGates ?? [],
    tripwire_step_active: snapshot?.tripwireSteps?.find((s) => s.status === "active"),
  },
  vault: {
    open_pr_count: vaultPrs.length,
    stale_pr_count: stale.length,
    p0_prs: vaultPrs.filter((p) => /P0|tripwire|vault/i.test(p.title)).map((p) => p.number),
  },
  business: {
    stripe_mrr_usd: null,
    note: "Add STRIPE_SECRET_KEY read-only later; free mode uses GitHub + snapshot only",
    tripwire_metric: snapshot?.kpis?.[0] ?? null,
  },
  research_artifacts: researchFiles.map((f) => `research/${f}`),
};

const path = await writeHermesOut("biz-metrics.json", metrics, { json: true });
console.log("Wrote", path);