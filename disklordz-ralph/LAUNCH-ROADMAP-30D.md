# 30-day launch: Baton Rouge Trap & NOLA Bounce VST3 → first sales

Stack: **Cursor + Superpowers + Ponytail (full, `ponytail-juce.mdc`) + REA**, TrapCoreDSP, **OpenClaw + Firecrawl**, **AutoGPT**. Frozen: `STACK-FROZEN.md`.

---

## Week 1: Core DSP & Catch2 (Days 1–7)

**Goal:** Lock audio engine + RT safety before UI or marketing.

| Days | Work |
|------|------|
| **1–2** | Git + CMake, `AGENTS.md`, `.cursor/rules/rea-juce-superpowers.mdc` + `ponytail-juce.mdc` (**not** duplicate `ponytail.mdc`). First Catch2 baseline on Windows. |
| **3–5** | Superpowers TDD: `Trap808Voice`, `SoftClipper` (2× OS), hi-hat choke, `RollEngine` in `Modules/TrapCoreDSP/` — Ponytail **full** on implementation only. |
| **6–7** | Release `.vst3` + `.pdb`; `pluginval --strictness-level 5`; REA — zero `new`/locks in `processBlock`. |

**Exit:** `ctest -C Release` green + pluginval pass.

---

## Week 2: UI, drag-export, FL QA, kits (Days 8–14)

**Goal:** Beatmaker UX + Louisiana sound library.

| Days | Work |
|------|------|
| **8–10** | Minimal editor: Glide + 808 Clip macros, **Trap808Voice** + **SoftClipper** in `processBlock`, **DLZ-VST-007** — `starter-cpp/Source/` + pad drag (WAV / Shift+MIDI). Swing + Roll: next story after FL smoke. |
| **11–12** | FL Studio (primary) + Ableton smoke: note-off, pitch-bend 12 st, state save/load, CPU @ 512 buffer — `vst/docs/FL-STUDIO-QA.md`. |
| **13–14** | `LouisianaKitGenerator.exe` → 150+ WAVs + 40 MIDI rolls → `Assets/Releases/` + `kit_manifest.json`; encrypted `.blob` for in-plugin factory. |

**Exit:** Drag-export proven in FL Playlist; kit zip in `Assets/Releases/`.

---

## Week 3: Licensing, installer, storefront (Days 15–21)

**Goal:** Trustworthy Windows install + 3-tier funnel copy.

| Days | Work |
|------|------|
| **15–16** | Lite binary (1 kit, clip on, import/drag gated per `LICENSING.md`); plan Moonbase for **Core** only. |
| **17–18** | Inno Setup: `vst/installer/DetroitPhonkTrapMaker-Lite.iss` / `-Core.iss` → `Assets/Releases/installers/`; sign `.exe` when cert ready. |
| **19–21** | Lemon Squeezy / Gumroad / Shopify draft; OpenClaw `track_competitors.py`; AutoGPT staged copy in `docs/factory/staged/`. |

**Exit:** Signed or unsigned Lite installer downloadable; storefront draft approved.

---

## Week 4: Seeding, content, launch (Days 22–30)

**Goal:** Lite in producers’ hands → first Core sales.

| Days | Work |
|------|------|
| **22–24** | `find_producer_leads.py` → 40–50 leads; `pending_review` only. |
| **25–26** | Approve 4-sentence emails; Lite link + optional affiliate — compliance in `outreach-compliance.md`. |
| **27–28** | 60s FL cook-up (glide + clip + **drag MIDI roll**); Shorts/TikTok/Reels. |
| **29–30** | Waitlist launch; Core $39–49 promo window; AutoGPT triage install FAQ; log v1.1 in `shortcut:` ledger. |

**Exit:** ≥5 Lite installs or ≥1 Core sale; support paths documented.

---

## Quick links

| Deliverable | Path |
|-------------|------|
| Drag-export integration | `vst/starter-cpp/DLZ-VST-007-INTEGRATION.md` |
| Windows installer | `vst/installer/README.md` |
| Tiers & DRM | `vst/LICENSING.md` |
| OpenClaw crons | `openclaw/openclaw.disklordz.telegram.json5.example` |

**Daily rule:** One `prd.json` story; no new agent tools until Week 4 ends.