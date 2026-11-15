# Impeccable (UI anti-slop)

[pbakaus/impeccable](https://github.com/pbakaus/impeccable) — design skill + hook for Cursor, Gemini CLI, Pi. Use on **storefront HTML** and **WebView faceplate mocks**, not on `TrapCoreDSP` C++.

## Install (Windows)

```powershell
cd C:\Dev\push
.\scripts\install-impeccable.ps1
```

Or: `npx impeccable install` → select **Cursor** → accept design hook → reload Cursor.

Update: `npx impeccable update`

## Context (already seeded)

| File | Role |
|------|------|
| `PRODUCT.md` | Audience, tiers, anti-references |
| `DESIGN.md` | Tokens — no purple-gradient slop |

Run `/impeccable init` only if you want to replace with interview output; otherwise refine these files in place.

## High-ROI commands

| Command | Use on DiskLordz |
|---------|------------------|
| `init` / `document` | Refresh PRODUCT/DESIGN from code |
| `shape` | Plan lite landing + pricing before HTML |
| `craft` | Build hero, tier table, audio preview stub |
| `distill` | Remove clutter from `storefront/plugin-faceplate` |
| `bolder` / `polish` | Hardware tactility pass |
| `audit` / `critique` | Pre-launch storefront + WebView mock |

## Targets in this repo

```
storefront/disklordz-lite/     # Lemon/Gumroad-style landing (primary)
storefront/plugin-faceplate/   # JUCE WebView HTML mock (future)
disklordz-command-center/src/ # Ops PWA — optional `document` pass
```

## Stack boundary (STACK-FROZEN)

- **Impeccable** → HTML/CSS/TSX polish
- **Cursor + Ponytail** → native JUCE editor (`PadGridComponent`) stays default for **v1 Lite ship**
- **WebView plugin UI** → after native drag-export passes FL smoke

## Gitignore

After install, merge Impeccable’s ephemeral paths from [upstream README](https://github.com/pbakaus/impeccable) into root `.gitignore` if the installer does not.