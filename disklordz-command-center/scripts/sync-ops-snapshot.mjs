/**
 * Pulls DiskLordz ops state from GitHub (vault kanban, open PRs, Instruments CI)
 * and writes public/ops-snapshot.json for the Command Center app.
 *
 * Requires GITHUB_TOKEN or gh-authenticated `gh api` (workflow sets GITHUB_TOKEN).
 */
import { readFile, writeFile, mkdir } from "node:fs/promises";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
import yaml from "yaml";

const __dir = dirname(fileURLToPath(import.meta.url));
const root = join(__dir, "..");
const outPath = join(root, "public", "ops-snapshot.json");

const VAULT = "samuelfreemanjobs-hash/Revenue-Systems-Engineer-Master-Vault";
const INSTRUMENTS = "samuelfreemanjobs-hash/Instruments";

const VST_WORKFLOWS = [
  { name: "Drum-Crush", path: "vst/DrumCrush/", workflow: "drumcrush.yml" },
  { name: "Stem-Shield", path: "vst/StemShield/", workflow: "stemshield.yml" },
  { name: "Phonk-Tape", path: "vst/PhonkTape/", workflow: "phonktape.yml" },
  { name: "Grim-MIDI", path: "vst/GrimMIDI/", workflow: "grimmidi.yml" },
  { name: "808-Punisher", path: "vst/Punisher808/", workflow: "punisher808.yml" },
  { name: "MeterTap", path: "vst/MeterTap/", workflow: "metertap.yml" },
  { name: "JD Upgraded + WAVE-9090 + SP-1200", path: "root + Wave9090/", workflow: "build.yml" },
  { name: "Auto Sampler", path: "AutoSampler/", workflow: "autosampler.yml" },
  { name: "Nightly QA (fleet)", path: "repo-wide", workflow: "nightly-qa.yml" },
];

const TRIPWIRE_STEPS_TEMPLATE = [
  { id: "G1", title: "Generate 5 one-shots", owner: "DRUM" },
  { id: "G2", title: "Automated WAV QC + MANIFEST hashes", owner: "DRUM" },
  { id: "G3", title: "Founder ears → qc_status approved", owner: "Operator" },
  { id: "G4", title: "Package tripwire-sample-v1.zip", owner: "DRUM" },
  { id: "G5", title: "Copy ZIP + SAAS checkout to Instruments", owner: "SAAS / FLOW" },
  { id: "G6", title: "Production smoke: checkout → credits → download", owner: "Operator" },
];

function authToken() {
  return (
    process.env.DISKDLORDZ_OPS_TOKEN?.trim() ||
    process.env.GITHUB_TOKEN?.trim() ||
    ""
  );
}

async function github(path) {
  const token = authToken();
  const url = `https://api.github.com${path}`;
  const headers = {
    Accept: "application/vnd.github+json",
    "User-Agent": "disklordz-ops-sync",
  };
  if (token) headers.Authorization = `Bearer ${token}`;
  const res = await fetch(url, { headers });
  if (!res.ok) {
    throw new Error(`GitHub ${path}: ${res.status} ${await res.text()}`);
  }
  return res.json();
}

async function fetchRepoFile(repo, filePath) {
  const data = await github(`/repos/${repo}/contents/${filePath}`);
  if (Array.isArray(data)) throw new Error(`Expected file: ${filePath}`);
  const text = Buffer.from(data.content, data.encoding ?? "base64").toString(
    "utf8",
  );
  return text;
}

function mapKanbanCards(doc) {
  const cards = doc.cards ?? [];
  return cards.map((c) => ({
    id: c.id,
    column: c.column,
    title: c.title,
    lane: c.lane,
    priority: c.priority ?? "P2",
    owner: c.owner,
    next: c.next,
    blocker: c.blocker,
    link: c.link,
    note: c.note,
    completed: c.completed,
  }));
}

function inferLaunchGates(cards) {
  const byId = Object.fromEntries(cards.map((c) => [c.id, c]));
  const blocked = (id) => byId[id]?.column === "blocked";
  const done = (id) => byId[id]?.column === "done";

  return [
    {
      id: "G1",
      title: "Production secrets on Vercel",
      doc: "docs/disklordz/SAAS-LAUNCH-GAP-AUDIT.md",
      status: done("K-013") ? "done" : "pending",
    },
    {
      id: "G2",
      title: "Tripwire checkout + webhook on Instruments",
      doc: "factory-exports/saas/tripwire-checkout/APPLY.md",
      status: done("K-002") || done("K-011") ? "ready" : "pending",
    },
    {
      id: "G3",
      title: "QC'd WAV pack tripwire-sample-v1",
      status:
        done("K-012")
          ? "done"
          : byId["K-012"]?.column === "in_progress"
            ? "blocked"
            : "pending",
    },
    {
      id: "G4",
      title: "HTTPS production deploy",
      doc: "Instruments/disklordz/website/DEPLOY.md",
      status: "pending",
    },
    {
      id: "G5",
      title: "LAUNCH_CAMPAIGN_START set",
      status: blocked("K-033") ? "blocked" : "pending",
    },
    {
      id: "G6",
      title: "disklordz.slack.com bootstrap",
      doc: "docs/DISKLORDZ-SLACK-SETUP.md",
      status: blocked("K-030") ? "blocked" : "pending",
    },
  ];
}

function inferTripwireSteps(cards, vaultPr126) {
  const k012 = cards.find((c) => c.id === "K-012");
  const merge126 = vaultPr126?.merged_at != null;

  const stepStatus = (idx) => {
    switch (idx) {
      case 0:
      case 1:
        return "done";
      case 2:
        return k012?.column === "in_progress" ? "active" : k012?.column === "done" ? "done" : "queued";
      case 3:
        return k012?.column === "done" ? "done" : "queued";
      case 4:
        return merge126 ? "done" : k012?.column === "done" ? "active" : "queued";
      case 5:
        return "queued";
      default:
        return "queued";
    }
  };

  return TRIPWIRE_STEPS_TEMPLATE.map((s, i) => ({
    ...s,
    status: stepStatus(i),
    detail:
      i === 4 && !merge126
        ? "Vault PR #126 + INSTRUMENTS_PUSH_TOKEN"
        : i === 2
          ? "Listen in DAW; update MANIFEST.yaml"
          : undefined,
  }));
}

function prPhase(pr) {
  const n = pr.number;
  if (n === 126) return "P0 Revenue — merge first";
  if (n === 138 || n === 145) return "P1 Ops";
  if (n === 124) return "P1 Docs";
  if (n === 143) return "P3 — defer";
  return "Vault";
}

function mergeableLabel(pr) {
  if (pr.draft) return "draft";
  if (pr.mergeable === false) return "conflicts";
  if (pr.mergeable === true) return "clean";
  return pr.mergeable_state ?? "open";
}

async function workflowCiStatus(repo, workflowFile) {
  try {
    const runs = await github(
      `/repos/${repo}/actions/workflows/${workflowFile}/runs?branch=main&per_page=1`,
    );
    const run = runs.workflow_runs?.[0];
    if (!run) return { ciStatus: "unknown" };
    const conclusion = run.conclusion ?? run.status;
    let ciStatus = "unknown";
    if (conclusion === "success") ciStatus = "success";
    else if (conclusion === "failure" || conclusion === "cancelled")
      ciStatus = "failure";
    else if (conclusion === "stale") ciStatus = "stale";
    const ciDate = run.updated_at?.slice(0, 10);
    return { ciStatus, ciDate };
  } catch {
    return { ciStatus: "unknown" };
  }
}

async function main() {
  const errors = [];
  let kanbanYaml;
  try {
    kanbanYaml = await fetchRepoFile(VAULT, "agents/disklordz/kanban.yaml");
  } catch (e) {
    errors.push(String(e));
    kanbanYaml = null;
  }

  if (!kanbanYaml) {
    try {
      kanbanYaml = await readFile(join(__dir, "fallback-kanban.yaml"), "utf8");
      errors.push("Vault kanban API unavailable — used scripts/fallback-kanban.yaml");
    } catch {
      /* no local fallback */
    }
  }

  let cards = [];
  let wipLimit = 2;
  if (kanbanYaml) {
    const doc = yaml.parse(kanbanYaml);
    cards = mapKanbanCards(doc);
    wipLimit = doc.wip_limits?.in_progress ?? 2;
  }

  let vaultPrs = [];
  let pr126 = null;
  try {
    const pulls = await github(
      `/repos/${VAULT}/pulls?state=open&per_page=50&sort=updated`,
    );
    vaultPrs = pulls
      .filter(
        (p) =>
          /disklordz|tripwire|drum|vault agent|slack team/i.test(p.title) ||
          [126, 138, 124, 145, 143].includes(p.number),
      )
      .sort((a, b) => {
        const rank = (n) =>
          n === 126 ? 0 : n === 124 ? 1 : n === 138 ? 2 : n === 145 ? 3 : 9;
        return rank(a.number) - rank(b.number);
      })
      .map((p) => ({
        number: p.number,
        title: p.title,
        phase: prPhase(p),
        mergeable: mergeableLabel(p),
        url: p.html_url,
      }));
    pr126 = pulls.find((p) => p.number === 126);
  } catch (e) {
    errors.push(String(e));
    try {
      const raw = await readFile(join(__dir, "fallback-vault-prs.json"), "utf8");
      vaultPrs = JSON.parse(raw);
    } catch {
      /* empty */
    }
  }

  const vstFleet = [];
  for (const sku of VST_WORKFLOWS) {
    const ci = await workflowCiStatus(INSTRUMENTS, sku.workflow);
    vstFleet.push({
      name: sku.name,
      path: sku.path,
      workflow: sku.workflow,
      ...ci,
    });
  }

  vstFleet.push(
    {
      name: "Lane Bus",
      path: "plugins/DisklordzLaneBus/",
      workflow: "—",
      ciStatus: "unknown",
    },
    {
      name: "CHORD ENGINE (DPF)",
      path: "DiskLordz-Audio repo",
      workflow: "disklordz-audio-windows.yml",
      ciStatus: "stale",
      ciDate: "2026-11-08",
    },
  );

  const launchGates = inferLaunchGates(cards);
  const tripwireSteps = inferTripwireSteps(cards, pr126);

  const snapshot = {
    generatedAt: new Date().toISOString(),
    sources: {
      kanban: `https://github.com/${VAULT}/blob/main/agents/disklordz/kanban.yaml`,
      businessOs: `https://github.com/${VAULT}/blob/main/agents/disklordz/business-os.yaml`,
      instrumentsMain: `https://github.com/${INSTRUMENTS}/tree/main`,
    },
    syncErrors: errors,
    northStar: {
      metric: "paid_tripwire_count_7d",
      label: "North star (business-os.yaml)",
      supporting: [
        "pro_mrr_usd",
        "generation_batches_7d",
        "github_main_green",
        "slack_bootstrap_complete",
      ],
    },
    kanban: { wipLimit, cards },
    launchGates,
    tripwireSteps,
    vaultPrs,
    vstFleet,
  };

  await mkdir(dirname(outPath), { recursive: true });
  await writeFile(outPath, `${JSON.stringify(snapshot, null, 2)}\n`, "utf8");
  console.log(`Wrote ${outPath} (${cards.length} kanban cards, ${vaultPrs.length} PRs)`);
  if (errors.length) console.warn("Partial sync:", errors.join("; "));
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});