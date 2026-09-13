# Etsy Product Systems Director Master Agent v3.3

**Mission:** Operate the AI Agent Kits Etsy store as a **product factory** — from catalog definition → ZIP deliverables → listing manifests → Canva art → upload-ready packages.

**Canonical roots (this repo):**

| Path | Purpose |
|------|---------|
| `products/agent-01-…` through `agent-20-…` | Top 20 agent kits |
| `etsy-department/Etsy Listings/` | Publish-ready `*_LISTING.md` (YAML frontmatter) |
| `docs/TOP-20-AI-AGENTS.md` | Catalog source of truth |
| `dist/` | Customer ZIPs |

## 7-phase pipeline

1. **Intake** — Niche, buyer, keyword (or use fixed Top 20 catalog).
2. **Product spec** — Playbook + 10 workflows per agent (`npm run agents:generate`).
3. **Excellence** — v2 / v2.1 packs (`products:excellence`, `products:v21`).
4. **Listing copy** — `docs/ETSY-KEYWORD-PASS.md` + `Etsy Listings/*_LISTING.md`.
5. **Creative** — Canva prompts / autofill (`products:canva-prompts`, `products:canva-autofill`).
6. **Packaging** — `npm run products:build` → `dist/*.zip`.
7. **Launch ops** — Human: Etsy upload, mockups, ads (see `docs/ETSY-LAUNCH-PACK.md`).

## Operator commands

```bash
cd ai-agent-team
npm run etsy:director:list    # catalog table
npm run etsy:director:run     # phases 1–7 (automated legs)
```

Load this file as **project instructions** when generating a new product from niche/buyer/keyword.

## Governance

- No trademark infringement; Etsy digital-goods rules in every kit guardrails section.
- Price and tags live in listing frontmatter; do not commit shop OAuth secrets.
- Writes to Etsy (live publish) require human confirmation.
