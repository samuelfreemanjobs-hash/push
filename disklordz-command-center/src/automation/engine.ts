import type { KanbanCard, LaunchGate, VaultPr, VstSku } from "../data/types";

export type AutomationSeverity = "critical" | "warn" | "info";

export type AutomationAction = {
  id: string;
  severity: AutomationSeverity;
  lane: string;
  title: string;
  detail: string;
};

export type DailyBrief = {
  generatedAt: string;
  northStar: string;
  actions: AutomationAction[];
  rituals: string[];
};

function cardById(cards: KanbanCard[], id: string) {
  return cards.find((c) => c.id === id);
}

export function buildDailyBrief(input: {
  cards: KanbanCard[];
  gates: LaunchGate[];
  vaultPrs: VaultPr[];
  vstFleet: VstSku[];
  wipLimit: number;
}): DailyBrief {
  const { cards, gates, vaultPrs, vstFleet, wipLimit } = input;
  const actions: AutomationAction[] = [];

  const pr126 = vaultPrs.find((p) => p.number === 126);
  if (pr126 && pr126.mergeable?.includes("conflict")) {
    actions.push({
      id: "merge-126",
      severity: "critical",
      lane: "FLOW",
      title: "Resolve vault PR #126 conflicts",
      detail: "Tripwire bundle blocked until #126 merges — enables INSTRUMENTS_PUSH_TOKEN path.",
    });
  } else if (pr126) {
    actions.push({
      id: "merge-126-ready",
      severity: "critical",
      lane: "FLOW",
      title: "Merge vault PR #126",
      detail: "Revenue handoff PR is next in queue.",
    });
  }

  const k012 = cardById(cards, "K-012");
  if (k012?.column === "in_progress") {
    actions.push({
      id: "tripwire-qc",
      severity: "critical",
      lane: "DRUM",
      title: "Founder QC tripwire-sample-v1",
      detail: k012.next ?? "Ears in DAW → approved → package ZIP.",
    });
  }

  const inProgress = cards.filter((c) => c.column === "in_progress");
  if (inProgress.length > wipLimit) {
    actions.push({
      id: "wip-overflow",
      severity: "warn",
      lane: "PM",
      title: `WIP over limit (${inProgress.length}/${wipLimit})`,
      detail: `Finish or park: ${inProgress.map((c) => c.id).join(", ")}`,
    });
  }

  const p0Ready = cards.filter(
    (c) => c.priority === "P0" && c.column === "ready",
  );
  for (const c of p0Ready.slice(0, 3)) {
    actions.push({
      id: `pull-${c.id}`,
      severity: "warn",
      lane: c.lane,
      title: `Pull ${c.id} when WIP allows`,
      detail: c.next ?? c.title,
    });
  }

  for (const g of gates.filter((x) => x.status === "blocked")) {
    actions.push({
      id: `gate-${g.id}`,
      severity: "warn",
      lane: "PLAN",
      title: `Gate ${g.id} blocked`,
      detail: g.title,
    });
  }

  const nightly = vstFleet.find((v) => v.name.includes("Nightly QA"));
  if (nightly?.ciStatus === "failure") {
    actions.push({
      id: "nightly-qa",
      severity: "warn",
      lane: "QA",
      title: "Instruments nightly QA failing",
      detail: `Last run ${nightly.ciDate ?? "unknown"} — triage before next factory batch.`,
    });
  }

  const blockedOps = cards.filter((c) => c.column === "blocked" && c.priority === "P0");
  for (const c of blockedOps) {
    actions.push({
      id: `blocked-${c.id}`,
      severity: "info",
      lane: c.lane,
      title: `${c.id} waiting on operator`,
      detail: c.blocker ?? "Unblock when secret or decision lands.",
    });
  }

  const rituals = [
    "Run business_health.py equivalent — fix first RED gate you can touch.",
    "Ship one revenue-adjacent commit (tripwire, checkout, or vault merge).",
    "Only escalate in #all-disklordz if blocked >24h.",
  ];

  return {
    generatedAt: new Date().toISOString(),
    northStar: "paid_tripwire_count_7d",
    actions: actions.slice(0, 12),
    rituals,
  };
}