/**
 * DiskLordz proposed applications (not third-party products).
 * Used by RADAR, SAGE, and Ralph to prioritize tech adoption.
 */

/** @typedef {"immediate" | "next" | "later"} BuildPriority */

/**
 * @type {readonly {
 *   rank: number;
 *   id: string;
 *   title: string;
 *   businessBenefit: string;
 *   buildPriority: BuildPriority;
 *   lanes: string[];
 *   surfaces: string[];
 *   existingHooks: string[];
 * }[]}
 */
export const OPPORTUNITIES = [
  {
    rank: 1,
    id: "drum_sample_factory",
    title: "AI-assisted drum sample factory",
    businessBenefit: "Produce original drum kits faster",
    buildPriority: "immediate",
    lanes: ["DRUM", "H"],
    surfaces: ["tripwire_packs", "agent_stack"],
    existingHooks: [
      "openclaw/drum-kit-factory/",
      "grok-bridge kit-spec",
      "tripwire_pack / tripwire_qc skills",
    ],
  },
  {
    rank: 2,
    id: "juce_vst_factory",
    title: "Automated JUCE VST development system",
    businessBenefit: "Ship more reliable plugins",
    buildPriority: "immediate",
    lanes: ["VST", "FLOW"],
    surfaces: ["vst_factory", "agent_stack"],
    existingHooks: [
      "vst_factory skill",
      "disklordz-ralph DLZ-VST-*",
      "FORGE / Instruments CI",
    ],
  },
  {
    rank: 3,
    id: "audio_qc_pipeline",
    title: "Automated audio quality-control pipeline",
    businessBenefit: "Improve commercial sample consistency",
    buildPriority: "immediate",
    lanes: ["DRUM", "VST"],
    surfaces: ["tripwire_packs", "vst_factory"],
    existingHooks: [
      "tripwire_qc skill",
      "drum-qc workspace",
      "validate-kit-spec.mjs",
    ],
  },
  {
    rank: 4,
    id: "sample_tagging_search",
    title: "AI-powered sample tagging and search",
    businessBenefit: "Organize products and improve discovery",
    buildPriority: "immediate",
    lanes: ["SAAS", "BIZ"],
    surfaces: ["command_center", "tripwire_packs"],
    existingHooks: ["MANIFEST.yaml naming rules", "Command Center catalog (future)"],
  },
  {
    rank: 5,
    id: "marketing_launch_automation",
    title: "Marketing and launch automation",
    businessBenefit: "Reduce repetitive business work",
    buildPriority: "immediate",
    lanes: ["BIZ", "H", "OC"],
    surfaces: ["agent_stack", "command_center"],
    existingHooks: [
      "launch_gates skill",
      "research_consume",
      "hermes-out research stubs",
    ],
  },
  {
    rank: 6,
    id: "preset_sound_design_assistant",
    title: "AI-driven preset and sound design assistant",
    businessBenefit: "Differentiate synthesizers",
    buildPriority: "next",
    lanes: ["VST"],
    surfaces: ["vst_factory"],
    existingHooks: ["Detroit Phonk Trap Maker preset path (future)"],
  },
  {
    rank: 7,
    id: "drum_sample_saas",
    title: "AI drum sample SaaS",
    businessBenefit: "Create recurring revenue",
    buildPriority: "next",
    lanes: ["SAAS", "DRUM"],
    surfaces: ["command_center", "tripwire_packs"],
    existingHooks: ["Tripwire funnel", "Command Center PWA"],
  },
  {
    rank: 8,
    id: "product_recommendation_engine",
    title: "Smart product recommendation engine",
    businessBenefit: "Increase cross-selling and bundles",
    buildPriority: "next",
    lanes: ["SAAS", "BIZ"],
    surfaces: ["command_center"],
    existingHooks: ["biz-metrics.json", "revenue-opportunities stub"],
  },
  {
    rank: 9,
    id: "plugin_test_lab",
    title: "Automated plugin testing laboratory",
    businessBenefit: "Reduce bugs and support costs",
    buildPriority: "next",
    lanes: ["VST", "FLOW"],
    surfaces: ["vst_factory", "agent_stack"],
    existingHooks: ["pluginval in Ralph verify", "github_pr_review"],
  },
  {
    rank: 10,
    id: "nl_drum_kit_designer",
    title: "Natural-language drum kit designer",
    businessBenefit: "Offer a compelling SaaS interface",
    buildPriority: "next",
    lanes: ["SAAS", "DRUM"],
    surfaces: ["command_center", "tripwire_packs"],
    existingHooks: ["Grok Drum Kit Architect handoff", "kit_spec_grok"],
  },
  {
    rank: 11,
    id: "support_docs_assistant",
    title: "AI customer support and documentation assistant",
    businessBenefit: "Improve post-purchase service",
    buildPriority: "later",
    lanes: ["SAAS", "BIZ"],
    surfaces: ["command_center"],
    existingHooks: ["Telegram sage briefs (lightweight today)"],
  },
  {
    rank: 12,
    id: "market_intelligence_monitoring",
    title: "AI market-intelligence monitoring",
    businessBenefit: "Find product gaps and opportunities",
    buildPriority: "later",
    lanes: ["H", "BIZ"],
    surfaces: ["agent_stack"],
    existingHooks: [
      "tech_radar_signals / AI Dev Signals",
      "research-weekly.mjs",
      "revenue-opportunities stub",
    ],
  },
];

export const BUILD_PRIORITY_ORDER = ["immediate", "next", "later"];

/** Map newsletter category → opportunities most likely accelerated */
export const CATEGORY_OPPORTUNITY_HINTS = {
  audio_ai: [1, 3, 4, 6, 7, 10],
  coding_ai: [2, 9],
  dsp_research: [6, 2],
  local_models: [1, 2, 4, 7, 9, 11],
  agent_frameworks: [2, 5, 9, 12],
  commercial_opportunity: [5, 7, 8, 12],
};

export function opportunityById(id) {
  return OPPORTUNITIES.find((o) => o.id === id);
}

export function immediateOpportunities() {
  return OPPORTUNITIES.filter((o) => o.buildPriority === "immediate");
}