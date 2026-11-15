const VAULT = "samuelfreemanjobs-hash/Revenue-Systems-Engineer-Master-Vault";
const INSTRUMENTS = "samuelfreemanjobs-hash/Instruments";
const PUSH = "samuelfreemanjobs-hash/push";

function token() {
  return (
    process.env.DISKDLORDZ_OPS_TOKEN?.trim() ||
    process.env.GITHUB_TOKEN?.trim() ||
    ""
  );
}

export async function github(path) {
  const url = `https://api.github.com${path}`;
  const headers = {
    Accept: "application/vnd.github+json",
    "User-Agent": "disklordz-agent-stack",
  };
  if (token()) headers.Authorization = `Bearer ${token()}`;
  const res = await fetch(url, { headers });
  if (!res.ok) {
    const body = await res.text();
    throw new Error(`GitHub ${path}: ${res.status} ${body.slice(0, 120)}`);
  }
  return res.json();
}

export async function openPullRequestCount(repo) {
  try {
    const prs = await github(
      `/repos/${repo}/pulls?state=open&per_page=1`,
    );
    return Array.isArray(prs) ? prs.length : 0;
  } catch {
    return null;
  }
}

export async function listOpenPrs(repo, limit = 10) {
  try {
    return await github(
      `/repos/${repo}/pulls?state=open&per_page=${limit}&sort=updated`,
    );
  } catch {
    return [];
  }
}

export async function lastWorkflowConclusion(repo, workflowFile) {
  try {
    const runs = await github(
      `/repos/${repo}/actions/workflows/${workflowFile}/runs?per_page=1`,
    );
    const run = runs.workflow_runs?.[0];
    if (!run) return { status: "unknown" };
    return {
      status: run.conclusion ?? run.status,
      name: run.name,
      url: run.html_url,
      updated: run.updated_at,
    };
  } catch (e) {
    return { status: "error", error: String(e.message) };
  }
}

export async function lastPushCommit(repo) {
  try {
    const meta = await github(`/repos/${repo}/commits?per_page=1`);
    const c = meta[0];
    return c
      ? { sha: c.sha.slice(0, 7), date: c.commit?.author?.date, message: c.commit?.message?.split("\n")[0] }
      : null;
  } catch {
    return null;
  }
}

export { VAULT, INSTRUMENTS, PUSH };