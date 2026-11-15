/** Factory milestone M1 — links opportunity ranks 1–3 to Ralph story ids */

export const FACTORY_M1 = {
  id: "factory-m1",
  title: "Factory milestone M1 — drum factory + VST automation + QC pipeline",
  opportunity_ids: [
    "drum_sample_factory",
    "juce_vst_factory",
    "audio_qc_pipeline",
  ],
  doc: "disklordz-ralph/FACTORY-MILESTONE.md",
  verifyCmd:
    "node openclaw/drum-kit-factory/scripts/verify-factory-milestone.mjs",
  story_ids: [
    "DLZ-FACTORY-M1-1",
    "DLZ-FACTORY-M1-2",
    "DLZ-FACTORY-M1-3",
    "DLZ-FACTORY-M1",
    "DLZ-DRUM-002",
    "DLZ-VST-002",
  ],
};