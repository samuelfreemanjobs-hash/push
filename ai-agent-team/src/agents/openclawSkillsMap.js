/** Maps mobile API agents to OpenClaw skill names (full runtime on VPS). */
export const OPENCLAW_SKILL_HINTS = {
  sage: ["delegate_subagent", "disk_ops_snapshot", "telegram_brief"],
  code: ["overnight_branch", "github_pr_review"],
  dev: ["overnight_branch", "vst_factory"],
  review: ["github_pr_review", "security_sentinel"],
  flow: ["flow_github_actions"],
  trip: ["tripwire_positioning", "kit_spec_grok"],
  plan: ["launch_gates", "vault_kanban"],
  forge: ["vst_factory"],
  sentinel: ["security_sentinel"],
  radar: ["tech_radar_signals", "memory_vault"],
};

export function skillLineForAgent(agentId) {
  const skills = OPENCLAW_SKILL_HINTS[agentId];
  if (!skills?.length) return "";
  return ` OpenClaw skills on VPS: ${skills.map((s) => `$${s}`).join(", ")}.`;
}