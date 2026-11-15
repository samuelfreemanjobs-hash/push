import { readFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
import { parse as parseYaml } from "yaml";

const __dir = dirname(fileURLToPath(import.meta.url));

export function loadProductCatalog() {
  const path = join(__dir, "..", "catalog", "product-catalog.yaml");
  return parseYaml(readFileSync(path, "utf8"));
}

export const CONCEPT_TYPES = [
  {
    id: "trend_jacking",
    label: "Concept 1 (Trend-Jacking)",
    description: "Recreate a trending beat style or artist sound.",
  },
  {
    id: "problem_solution",
    label: "Concept 2 (Problem/Solution)",
    description: "Fix a common producer headache in under 15 seconds.",
  },
  {
    id: "sound_design_cookup",
    label: "Concept 3 (Sound Design / Cookup)",
    description: "Fast from-scratch melody or drum bounce with minimal sounds.",
  },
];

export const VIDEO_BRIEF_SECTIONS = [
  "Title & Target Angle",
  "Featured Product & Specific Preset/Sound",
  "0:00–0:03 Hook (On-Screen Text + Spoken Line)",
  "0:03–0:20 DAW Screen Action & Audio Demo",
  "0:20–0:25 Call to Action (CTA)",
  "Post Caption & 5–8 Niche Hashtags",
  "Pinned Comment CTA",
  "Screen Recording Cues (resolution, zoom, click targets)",
  "Audio Hooks (what plays at 0:00 — finished beat vs dry loop)",
];