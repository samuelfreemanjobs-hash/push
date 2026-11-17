const OWNER = "samuelfreemanjobs-hash";
const REPO = "push";

export async function dispatchWorkflow({
  workflowFile,
  ref,
  inputs = {},
}) {
  const token = process.env.GITHUB_DISPATCH_TOKEN?.trim();
  if (!token) {
    throw new Error(
      "GITHUB_DISPATCH_TOKEN not set on ai-agent-team (PAT with workflow scope on push repo)",
    );
  }

  const url = `https://api.github.com/repos/${OWNER}/${REPO}/actions/workflows/${workflowFile}/dispatches`;
  const res = await fetch(url, {
    method: "POST",
    headers: {
      Accept: "application/vnd.github+json",
      Authorization: `Bearer ${token}`,
      "User-Agent": "disklordz-agent-team",
      "Content-Type": "application/json",
    },
    body: JSON.stringify({ ref, inputs }),
  });

  if (res.status === 204) {
    return { workflowFile, ref };
  }

  const text = await res.text();
  throw new Error(`GitHub dispatch ${res.status}: ${text}`);
}