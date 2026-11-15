# Gemini CLI (`@google/gemini-cli`)

Google’s terminal agent for **bulk audits, asset organization, and multimodal analysis** on your Windows machine. It does **not** replace Cursor (C++/JUCE), OpenClaw (cron intel), or pi-coding-agent unless you choose to.

Upstream: [google-gemini/gemini-cli](https://github.com/google-gemini/gemini-cli).

## Install (Windows PowerShell)

```powershell
npm install -g @google/gemini-cli
# or one-shot:
npx @google/gemini-cli
```

From repo root:

```powershell
cd C:\Dev\push
gemini
```

Complete browser sign-in on first run (free tier quotas on Flash/Pro — see Google docs). API-key flow is optional for CI; prefer sign-in for solo operator use.

**Project instructions:** root `GEMINI.md` includes `@AGENTS.md` so repo rules load automatically.

## Verify

```powershell
cd C:\Dev\push
gemini -p "Summarize STACK-FROZEN.md in 5 bullets"
```

## Best roles in DiskLordz

### 1. Whole-catalog audit (1M context)

Point at a release folder before zip/store:

- Naming: `[Style]_[Type]_[Key]_[Index].wav`
- Duplicate hashes, missing root-note suffixes on 808s
- MIDI map vs `kit_manifest.json`

Template: `docs/launch/prompts/gemini-kit-audit.md`

### 2. Multimodal reference analysis

Attach reference audio under `Assets/References/` (create operator-side):

```text
Analyze @Assets/References/nola_bounce_ref.wav — 808 decay ms, hat density, swing %. Compare to docs in vst/TRAP-PHONK-ARCHITECTURE.md; suggest Catch2-safe parameter targets only (no C++ patch unless asked).
```

### 3. Batch rename / README / license text

Standardize folders:

```text
01_808s_Tuned, 02_BooBass_Plucks, 03_NOLA_Chants, 04_Triggerman_MIDIs
```

Output: `README_License.txt` in the kit folder — operator reviews before ship.

### 4. SEO from manifest

Template: `docs/launch/prompts/gemini-storefront-seo.md`

## Boundaries (STACK-FROZEN)

| Gemini CLI | Cursor + Superpowers + REA |
|------------|----------------------------|
| Renames, tracklists, copy, audits | `TrapCoreDSP`, pluginval, REA |
| Read DSP headers, suggest tweaks | Implement + Catch2 tests |
| `docs/launch/*` drafts | `Source/` production code |

## Coexistence

| Tool | Role |
|------|------|
| **Cursor** | JUCE editor, drag-export code (already in `starter-cpp/Source/`) |
| **Gemini CLI** | Kit QA + marketing text from folders |
| **OpenClaw** | Scheduled Firecrawl / outreach |
| **Claude-Mem** | Local Cursor memory (`docs/dev/claude-mem.md`) |

## Wrapper script

```powershell
.\scripts\gemini-from-repo.ps1
```

Opens `gemini` with cwd = repo root and prints active `GEMINI.md`.