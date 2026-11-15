# OpenMontage prompt templates — DiskLordz

Use these in **Cursor / Claude Code** with the OpenMontage project open at `marketing-studio/`.  
After `export_kit.py` (auto-publish) or `publish_to_openmontage.py`, assets live at **`assets/latest_release/`**.

**Brand:** hardware booth, gunmetal `#121216`, amber accents, synthesis-only drums, FL Studio–friendly drag workflow on Core+.  
**Never claim:** copyrighted samples, “AI-generated kits” vaporware tone, iLok.

---

## Template A — Automated drum kit promo (9:16, ~25s)

```text
Read assets/latest_release/release_brief.json and inspect the audio files in assets/latest_release/. Produce a high-energy, 25-second 9:16 vertical product promo video for this sample pack (DiskLordz, synthesis-only):

1. 0:00–0:05 (Solo hit showcase): Play hero_kick.wav, hero_snare.wav, and hero_hat.wav back-to-back with dark studio waveform/oscilloscope visuals and bold kinetic text: "24-Bit / 48 kHz · Analog DSP Drums".
2. 0:05–0:22 (Full beat drop): Transition into demo_beat.wav on the downbeat. Use audio transient detection to cut visual scenes, zoom pulses, and spec callouts (total sample count and velocity layers from release_brief.json, royalty-free / synthesis-only) strictly on kick and snare hits.
3. 0:22–0:25 (CTA): Clean product card with pack_name from release_brief.json and "Download Now — Link in Bio" as the beat tails out.

Pause after generating the storyboard so I can approve pacing before the final MP4. Run ffprobe loudness check before final export; peak must not clip.
```

---

## Template B — Commercial VST3 launch trailer (16:9, ~45s)

```text
Create a 45-second 16:9 widescreen launch trailer for **Detroit Phonk Trap Maker** (DiskLordz VST3), not a generic "DrumSynth" name.

Use assets/latest_release/demo_beat.wav as the background soundtrack unless a fresher plugin bounce exists at assets/latest_release/plugin_demo.wav.

Screen footage: assets/latest_release/plugin_ui_capture.mp4 (if missing, stop and ask me to add a capture from starter-cpp).

1. Hook (0:00–0:08): Sub-heavy kick hits synced to macro close-ups of the plugin UI; typography: "Zero Samples. Pure Circuit Math." (synthesis-only — no sampled loops).
2. Feature walkthrough (0:08–0:32): Cuts between BR 808 glide, soft clip, pad workflow; lower-thirds on snare transients: "Real-Time C++20 DSP", "VST3 · Ultra-Low CPU", "Lite free / Core drag WAV".
3. Outro (0:32–0:45): Full UI over final beat drop; end card: "Windows first · FL Studio friendly".

Run OpenMontage audio loudness and subtitle quality gates before final MP4 export. Pause at storyboard first.
```

Drop `plugin_ui_capture.mp4` into `assets/latest_release/` after recording `disklordz-ralph/vst/starter-cpp`.

---

## 1. Drum kit — 30s vertical Short (9:16, legacy paths)

```text
You are producing a promo for DiskLordz, a synthesis-only drum brand (southern trap / Baton Rouge bounce).

Context files (read first):
- assets/disklordz/{{PACK_FOLDER}}/brief.json
- assets/disklordz/{{PACK_FOLDER}}/manifest.json

Soundtrack: {{AUDIO_PATH}}  (use this exact file; do not replace with stock music)

Goal: 30-second 9:16 vertical MP4 for TikTok / Reels / YouTube Shorts.

Creative:
- Detect kick and snare transients; cut or scale typography on those hits (±50ms tolerance).
- Kinetic type: "24-bit / 48 kHz" · "Analog DSP Drums" · "{{PACK_TITLE}}"
- Visuals: dark waveform + rubber-pad aesthetic; subtle scanlines; NO purple gradients.
- End card: "DiskLordz" + "Link in bio" placeholder (no fake URL).

Workflow (OpenMontage checkpoints — STOP for my approval after each):
1. Brief + research (competitor pacing only — do not copy their audio)
2. Scene plan + storyboard stills
3. Draft render (720p) with ffprobe peak report
4. Final 1080×1920 after I approve

Quality gates before final render:
- ffprobe: soundtrack peak ≤ −0.1 dBFS
- No clipped audio; captions spell "48 kHz" correctly
- All claims match manifest.json sample_count and format fields
```

**Typical audio paths after handoff:**

- Loop: `assets/disklordz/{{PACK_FOLDER}}/best_kit.wav`
- Or single hit montage built from `clips/` folder

---

## 2. Drum kit — 45s beat visualizer (16:9 YouTube)

```text
Create a 45-second 16:9 YouTube beat-demo for "{{PACK_TITLE}}" by DiskLordz.

Read manifest.json in assets/disklordz/{{PACK_FOLDER}}/.

Audio: assets/disklordz/{{PACK_FOLDER}}/best_kit.wav

Structure:
- 0:00–0:03 — Logo sting (DiskLordz wordmark, amber on gunmetal)
- 0:03–0:35 — Full pattern; waveform + pad grid; hits flash on kick/snare
- 0:35–0:42 — Rapid grid of kit folders (Kicks / Snares / Hats) with layer names from manifest
- 0:42–0:45 — CTA: "Royalty-free · Synthesis only · 24-bit WAV"

Pause after storyboard. Use local/free assets first; ask before any paid API generation.

Run subtitle verification and loudness check; show me ffprobe output before final render.
```

---

## 3. VST launch — Detroit Phonk Trap Maker trailer (60s, 16:9)

```text
Produce a 60-second launch trailer for "Detroit Phonk Trap Maker" VST3 by DiskLordz (not TrapDrumEngine).

Inputs I will provide under assets/disklordz/vst-launch/:
- screen/*.mp4 or *.png — UI walkthrough (pads, BR 808 glide, soft clip)
- optional: demo_loop.wav (synthesis-only beat from our plugin)

Audience: FL Studio trap producers on Windows; tone: confident, booth-demo, not corporate SaaS.

Script beats:
1. Problem — "Generic 808 VSTs. No bounce."
2. Demo — pad hit → glide → drag WAV to playlist (Core+) — use supplied screen capture
3. Proof — "Low latency · No iLok · Lite free / Core $39"
4. CTA — storefront waitlist / download Lite

Deliverables: storyboard → VO script (optional Piper/local TTS) → 16:9 1080p + 9:16 cutdown.

Stop for approval after scene plan. Apply brand colors from brief.json.

Do not invent features not in brief.json feature_flags.
```

Prepare `assets/disklordz/vst-launch/brief.json` manually or copy from template in handoff script comments.

---

## 4. Coordinator one-liner (Ruflo / Claude Code)

```text
Read CLAUDE.md. Kit export is done. Run publish_to_openmontage.py, open ../marketing-studio, load assets/latest_release/AGENT_PROMPT.md, and execute Template A (or B for VST) with human approval at storyboard and draft render.
```