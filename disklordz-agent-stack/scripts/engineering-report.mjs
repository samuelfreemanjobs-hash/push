#!/usr/bin/env node
import { readFile } from "node:fs/promises";
import {
  INSTRUMENTS,
  PUSH,
  VAULT,
  lastPushCommit,
  lastWorkflowConclusion,
  listOpenPrs,
} from "../lib/github.mjs";
import { snapshotPath } from "../lib/paths.mjs";
import { writeHermesOut } from "../lib/write.mjs";

const generated_at = new Date().toISOString();
const blockers = [];

let snapshot = null;
try {
  snapshot = JSON.parse(await readFile(snapshotPath(), "utf8"));
} catch (e) {
  blockers.push(`ops-snapshot unreadable: ${e.message}`);
}

const vaultPrs = await listOpenPrs(VAULT, 15);
const ciInstruments = await lastWorkflowConclusion(INSTRUMENTS, "build.yml");
const pushHead = await lastPushCommit(PUSH);

const report = {
  generated_at,
  mode: "free-tier-disklordz-agent-stack",
  vault_prs: vaultPrs.map((p) => ({
    number: p.number,
    title: p.title,
    url: p.html_url,
    mergeable_state: p.mergeable_state,
  })),
  ci: {
    instruments_build: ciInstruments,
  },
  push_last_commit: pushHead,
  snapshot_meta: snapshot
    ? {
        kanban_cards: snapshot.kanbanCards?.length ?? 0,
        sync_errors: snapshot.meta?.syncErrors ?? [],
        generated_at: snapshot.meta?.generatedAt,
      }
    : null,
  blockers,
};

const path = await writeHermesOut("engineering-report.json", report, {
  json: true,
});
console.log("Wrote", path);