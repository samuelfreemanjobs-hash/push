import { vaultConnectionStatus } from "../github/vaultOps.js";

export async function buildConnectionStatus() {
  const vault = await vaultConnectionStatus();
  const workflowDispatch = Boolean(
    process.env.GITHUB_DISPATCH_TOKEN?.trim(),
  );

  return {
    agent: {
      reachable: true,
      gemini: Boolean(process.env.GEMINI_API_KEY),
      ollama: Boolean(process.env.OLLAMA_BASE_URL),
    },
    vault,
    github: {
      workflowDispatch,
      reason: workflowDispatch
        ? undefined
        : "GITHUB_DISPATCH_TOKEN not set on ai-agent-team",
    },
    checkedAt: new Date().toISOString(),
  };
}