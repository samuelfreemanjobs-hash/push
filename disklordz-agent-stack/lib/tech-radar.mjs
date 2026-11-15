/** DiskLordz Technology Intelligence — categories, products, report shape */

export const SOURCE = {
  id: "ai_dev_signals",
  name: "AI Dev Signals",
  site: "https://www.aidevsignals.com/",
  publisher: "Marktechpost",
  defaultFeedUrl:
    "https://rss.beehiiv.com/feeds/f5e63dd4-5653-4f09-83e2-321a8b1ba526.xml",
};

/** What to watch for in each newsletter issue */
export const CATEGORIES = [
  {
    id: "audio_ai",
    label: "Audio AI",
    watchFor:
      "Sound generation, recognition, separation, tagging, voice, music models",
    keywords: [
      "audio",
      "voice",
      "speech",
      "music",
      "sound",
      "transcribe",
      "tts",
      "asr",
      "stem",
      "separation",
    ],
  },
  {
    id: "coding_ai",
    label: "Coding AI",
    watchFor: "C++, build systems, IDE agents, codegen, SWE-bench, compilers",
    keywords: [
      "code",
      "coding",
      "cpp",
      "c++",
      "cmake",
      "compiler",
      "swe-bench",
      "ide",
      "cursor",
      "copilot",
      "kernel",
      "torch.compile",
    ],
  },
  {
    id: "dsp_research",
    label: "DSP research",
    watchFor:
      "Synthesis, filtering, physical modeling, real-time audio algorithms",
    keywords: [
      "dsp",
      "synthesis",
      "filter",
      "physical model",
      "waveform",
      "neural audio",
      "differentiable",
    ],
  },
  {
    id: "local_models",
    label: "Local models",
    watchFor: "CPU/GPU inference, quantization, edge, on-device, Ollama-class",
    keywords: [
      "quantize",
      "quantization",
      "gguf",
      "on-device",
      "edge",
      "jetson",
      "local",
      "inference",
      "ollama",
      "vllm",
    ],
  },
  {
    id: "agent_frameworks",
    label: "Agent frameworks",
    watchFor: "Testing, orchestration, MCP, tool use, reliability, automation",
    keywords: [
      "agent",
      "agentic",
      "mcp",
      "orchestr",
      "workflow",
      "tool use",
      "rewind",
      "sandbox",
    ],
  },
  {
    id: "commercial_opportunity",
    label: "Commercial opportunity",
    watchFor: "Tech that could become a marketable plugin or SaaS feature",
    keywords: ["api", "launch", "sdk", "plugin", "saas", "pricing", "open source"],
  },
];

/** Existing DiskLordz surfaces — map findings here, not a raw article archive */
export const DISKLORDZ_PRODUCTS = [
  {
    id: "vst_factory",
    label: "VST plugins (JUCE factory / Instruments)",
    examples: "Detroit Phonk Trap Maker, future DSP plugins",
  },
  {
    id: "tripwire_packs",
    label: "Tripwire $9 drum packs",
    examples: "tripwire-sample-v1, Grok kit-spec pipeline",
  },
  {
    id: "command_center",
    label: "DiskLordz Command Center (ops PWA)",
    examples: "Kanban, gates, agent lanes",
  },
  {
    id: "agent_stack",
    label: "Agent stack (OpenClaw + hermes-out cron)",
    examples: "SAGE briefs, overnight dev, research stubs",
  },
  {
    id: "none",
    label: "No current product fit",
    examples: "Watch only or skip",
  },
];

/** JSON schema for one screened item (LLM output) */
export const reportItemSchema = {
  headline: "string — newsletter item title",
  source_url: "string — primary link",
  category: "one of CATEGORIES[].id",
  technology_summary: "2–4 sentences, plain language",
  licensing: "e.g. MIT, Apache-2.0, proprietary API, unknown",
  integration_difficulty: "1 (trivial) – 5 (major R&D)",
  expected_cost: "e.g. $0 self-host, API $/1M tokens, GPU hourly",
  product_fit: "DISKLORDZ_PRODUCTS[].id",
  improves: "what gets better if adopted",
  verdict: "adopt | watch | skip",
  next_action: "optional single concrete step or null",
};

export const INTEGRATION_LABELS = {
  1: "drop-in library or REST",
  2: "wrapper + config",
  3: "new subsystem",
  4: "research spike required",
  5: "not viable near-term",
};