/** Maps mobile API agents to OpenClaw skill names (full runtime on VPS). */
export const OPENCLAW_SKILL_HINTS = {
  sage: ["delegate_subagent", "disk_ops_snapshot", "telegram_brief"],
  code: ["overnight_branch", "github_pr_review"],
  dev: ["overnight_branch", "vst_factory", "egm_spec_challenge", "waza_agent_eval"],
  review: ["github_pr_review", "security_sentinel", "vst_qa_gate", "waza_agent_eval"],
  flow: ["flow_github_actions"],
  trip: ["tripwire_positioning", "kit_spec_grok"],
  plan: ["launch_gates", "vault_kanban"],
  forge: ["vst_factory"],
  sentinel: ["security_sentinel"],
  arch: ["vst_architect", "egm_spec_challenge"],
  dsp: ["vst_dsp_engineer", "mcode_vst_dsp"],
  gui: ["vst_gui_builder", "agent_desktop"],
  build: ["vst_build_engineer", "vst_factory"],
  preset: ["vst_preset_author", "tripwire_pack"],
  qa: ["vst_qa_gate", "waza_agent_eval"],
  release: ["vst_release_manager", "launch_gates"],
};

export function skillLineForAgent(agentId) {
  const skills = OPENCLAW_SKILL_HINTS[agentId];
  if (!skills?.length) return "";
  return ` OpenClaw skills on VPS: ${skills.map((s) => `$${s}`).join(", ")}.`;
}