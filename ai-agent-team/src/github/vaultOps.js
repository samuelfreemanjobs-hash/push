import yaml from "yaml";

const VAULT = "samuelfreemanjobs-hash/Revenue-Systems-Engineer-Master-Vault";
const INSTRUMENTS = "samuelfreemanjobs-hash/Instruments";

function vaultToken() {
  return (
    process.env.DISKDLORDZ_OPS_TOKEN?.trim() ||
    process.env.GITHUB_DISPATCH_TOKEN?.trim() ||
    ""
  );
}

async function githubGet(path) {
  const token = vaultToken();
  if (!token) {
    throw new Error("DISKDLORDZ_OPS_TOKEN not configured on ai-agent-team");
  }
  const res = await fetch(`https://api.github.com${path}`, {
    headers: {
      Accept: "application/vnd.github+json",
      Authorization: `Bearer ${token}`,
      "User-Agent": "disklordz-ai-agent-team",
    },
  });
  if (!res.ok) {
    const text = await res.text();
    throw new Error(`GitHub ${res.status}: ${text}`);
  }
  return res.json();
}

async function fetchRepoFile(filePath) {
  const data = await githubGet(`/repos/${VAULT}/contents/${filePath}`);
  return Buffer.from(data.content, data.encoding ?? "base64").toString("utf8");
}

function mapKanbanCards(doc) {
  return (doc.cards ?? []).map((c) => ({
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

export async function vaultConnectionStatus() {
  const token = vaultToken();
  if (!token) {
    return { connected: false, reason: "DISKDLORDZ_OPS_TOKEN not set" };
  }
  try {
    const text = await fetchRepoFile("agents/disklordz/kanban.yaml");
    const doc = yaml.parse(text);
    return {
      connected: true,
      cardCount: (doc.cards ?? []).length,
    };
  } catch (e) {
    return {
      connected: false,
      reason: e instanceof Error ? e.message : String(e),
    };
  }
}

export async function pullVaultOpsFragment() {
  const errors = [];
  let cards = [];
  let wipLimit = 2;
  try {
    const kanbanYaml = await fetchRepoFile("agents/disklordz/kanban.yaml");
    const doc = yaml.parse(kanbanYaml);
    cards = mapKanbanCards(doc);
    wipLimit = doc.wip_limits?.in_progress ?? 2;
  } catch (e) {
    errors.push(String(e));
  }

  let vaultPrs = [];
  try {
    const pulls = await githubGet(
      `/repos/${VAULT}/pulls?state=open&per_page=30`,
    );
    vaultPrs = pulls
      .filter((p) =>
        /disklordz|tripwire|drum|vault agent|slack team/i.test(p.title),
      )
      .slice(0, 12)
      .map((p) => ({
        number: p.number,
        title: p.title,
        phase: "Vault",
        mergeable: p.mergeable_state ?? "open",
        url: p.html_url,
      }));
  } catch (e) {
    errors.push(String(e));
  }

  return {
    generatedAt: new Date().toISOString(),
    syncErrors: errors,
    kanban: { wipLimit, cards },
    vaultPrs,
    sources: {
      kanban: `https://github.com/${VAULT}/blob/main/agents/disklordz/kanban.yaml`,
      instrumentsMain: `https://github.com/${INSTRUMENTS}/tree/main`,
    },
  };
}