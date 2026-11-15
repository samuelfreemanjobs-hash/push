# OpenMontage prompt templates — DiskLordz

Use these in **Cursor / Claude Code** with the OpenMontage project open at `marketing-studio/`.  
Replace `{{PACK_TITLE}}`, `{{AUDIO_PATH}}`, and paths after running `handoff-pack-to-openmontage.ps1`.

**Brand:** hardware booth, gunmetal `#121216`, amber accents, synthesis-only drums, FL Studio–friendly drag workflow on Core+.  
**Never claim:** copyrighted samples, “AI-generated kits” vaporware tone, iLok.

---

## 1. Drum kit — 30s vertical Short (9:16)

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
Read CLAUDE.md. Kit export is done. Run scripts/handoff-pack-to-openmontage.ps1, open ../marketing-studio, load assets/disklordz/*/AGENT_PROMPT.md, and execute template "Drum kit — 30s vertical Short" with human approval at storyboard and draft render.
```