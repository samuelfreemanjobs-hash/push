# MetaGPT blueprint — VST product PRD (pre–Core)

Use **before** large Core/Suite feature work. Output feeds Ruflo memory and `prd.json` stories — not auto-commit.

```text
Act as MetaGPT Product Manager for DiskLordz.

Input product:
"Detroit Phonk Trap Maker" — hybrid analog-modeling drum VST3 for southern trap / Baton Rouge bounce. Windows-first, FL Studio priority. Lite free (factory kit only), Core $44 (drag WAV/MIDI, user import), Suite $89 (expansions).

Produce:
1. Competitive positioning vs generic 808 romplers (no iLok, pads-first UX)
2. User stories for Lite → Core conversion (drag to FL Playlist)
3. APVTS parameter table: Glide, 808 Clip, pad levels — max 12 user-facing controls in v1 Core
4. Success metrics: pluginval pass, trial-to-Core %, refund rate < 5%
5. Explicit non-goals for v1: iLok, cloud preset sync, mobile app

Constraints from engineering:
- Real-time: no alloc/lock in processBlock
- Ship tree: disklordz-ralph/vst/starter-cpp only
- Autoresearch wins merge from VoiceDSP.h only after human review

Format: markdown PRD + JSON backlog items { id, title, acceptance_criteria[] }.
Do not generate C++ code in this run.
```