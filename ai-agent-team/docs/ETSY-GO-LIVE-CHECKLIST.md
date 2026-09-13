# Etsy go-live checklist — three flagship SKUs

**Scope:** Ship only these listings first. No new products until all three are live with images and downloads attached.

| # | Product | Price | ZIP | SEO & copy |
|---|---------|------:|-----|------------|
| 1 | AI Client Onboarding Agent Kit | $24.99 | `dist/ai-client-onboarding-agent-kit.zip` | [Keyword pass — Listing 1](ETSY-KEYWORD-PASS.md#listing-1--ai-client-onboarding-agent-kit-2499) · [Launch pack — Listing #1 paste copy](ETSY-LAUNCH-PACK.md#listing-1-paste-into-etsy--ai-client-onboarding-agent-kit) |
| 2 | Marketing Command Center (Excel) | $19.99 | `dist/marketing-command-center-excel.zip` | [Keyword pass — Listing 2](ETSY-KEYWORD-PASS.md#listing-2--marketing-command-center-excel-1999) |
| 3 | Etsy Listing SEO Agent Kit (Agent 01) | $19.99 | `dist/agent-01-etsy-listing-seo-agent-kit.zip` | [Keyword pass — Agent Kit 1](ETSY-KEYWORD-PASS.md#ai-agent-kit-1--etsy-listing-seo-agent-3299) · stub: `etsy-department/Etsy Listings/agent-01-etsy-listing-seo-agent-kit_LISTING.md` |

**Shop positioning & ladder context:** [ETSY-LAUNCH-PACK.md](ETSY-LAUNCH-PACK.md)  
**Automation / OAuth (optional drafts):** [ETSY-AUTOMATION.md](ETSY-AUTOMATION.md)

---

## Phase 0 — One-time shop & secrets

- [ ] Etsy shop open, policies filled (returns for digital, privacy, about).
- [ ] Payment and tax settings complete for your region.
- [ ] Brand name consistent across shop title, banner, and listing footers (`[YOUR BRAND]` in keyword pass).
- [ ] **Build deliverables locally** (ZIPs are not in git):

```bash
cd ai-agent-team
npm install
npm run products:build
# optional: full agent ZIPs + listing stubs
npm run etsy:director:run
```

- [ ] Confirm each ZIP opens and matches what the description promises (file list spot-check).
- [ ] *(Optional API path)* Etsy OAuth + `ETSY_SHOP_ID` per [ETSY-AUTOMATION.md](ETSY-AUTOMATION.md) — use **draft** publish only; images/files still need seller UI or upload API.

---

## Phase 1 — Shared listing standards (all three)

For **each** SKU:

- [ ] **Title:** Use recommended option **A** from keyword pass (≤140 chars).
- [ ] **13 tags:** Copy from keyword pass tables (each tag ≤20 chars).
- [ ] **Price / quantity:** As in table above · quantity **999** · type **Digital download**.
- [ ] **Description:** Opener from keyword pass + body adapted from launch pack (Listing 1) or agent stub; include license, FAQ, and “message before purchase” line.
- [ ] **Images:** 5–8 mockups (Canva). Different **hero text** per SKU so listings do not cannibalize — see [cross-listing strategy](ETSY-KEYWORD-PASS.md#cross-listing-strategy-avoid-cannibalization).
- [ ] **Digital file:** Attach the matching ZIP from `dist/`.
- [ ] **Preview:** Mobile snippet (~160 chars) reads clearly; no trademarked AI tool names in title.
- [ ] **Save as draft** → proofread → **Active** when ready.

---

## Phase 2 — SKU 1: AI Client Onboarding Agent Kit

- [ ] Title/tags from [Listing 1](ETSY-KEYWORD-PASS.md#listing-1--ai-client-onboarding-agent-kit-2499).
- [ ] Paste description from [Launch pack — Listing #1](ETSY-LAUNCH-PACK.md#listing-1-paste-into-etsy--ai-client-onboarding-agent-kit).
- [ ] Mockup prompts in launch pack (laptop checklist, flat-lay SOP, before/after pipeline).
- [ ] Upload `ai-client-onboarding-agent-kit.zip`.
- [ ] Pin this listing as **shop featured** #1 if Etsy allows only one hero — this is the anchor SKU.

---

## Phase 3 — SKU 2: Marketing Command Center Excel

- [ ] Title/tags from [Listing 2](ETSY-KEYWORD-PASS.md#listing-2--marketing-command-center-excel-1999).
- [ ] Description: keyword pass opener + bullets for dashboard, 12-week calendar, campaigns, lead sources (mirror structure of Listing 1 FAQ/license).
- [ ] Hero image emphasizes **planner / calendar / KPI**, not onboarding.
- [ ] Upload `marketing-command-center-excel.zip`.
- [ ] In description, **do not** lead with onboarding SOP language (avoids competing with SKU 1).

---

## Phase 4 — SKU 3: Etsy Listing SEO Agent Kit

- [ ] Title/tags from [Agent Kit 1](ETSY-KEYWORD-PASS.md#ai-agent-kit-1--etsy-listing-seo-agent-3299) (price on shop: **$19.99** per product matrix; keyword pass shows a higher tier — use $19.99 unless you intentionally reprice).
- [ ] Expand stub copy in `etsy-department/Etsy Listings/agent-01-etsy-listing-seo-agent-kit_LISTING.md` with deliverables from `products/agent-01-etsy-listing-seo-agent-kit/`.
- [ ] Hero image: Etsy search / tags / title optimization (seller audience).
- [ ] Upload `agent-01-etsy-listing-seo-agent-kit.zip`.
- [ ] Tags filled (stub `tags: []` is intentional until publish — copy from keyword pass).

---

## Phase 5 — Go-live verification

- [ ] Open each listing in a logged-out browser; buy-path shows digital download.
- [ ] Test purchase with a secondary account or Etsy’s preview flow if available.
- [ ] Download link works; file size reasonable.
- [ ] Shop sections: group under **Business tools** / **AI workflows** (or one section for all three).
- [ ] Optional: 7-day social cadence from [launch pack](ETSY-LAUNCH-PACK.md#7-day-marketing-linkedin--pinterest) — only after listings are **Active**.

---

## Phase 6 — After first three (explicitly out of scope here)

Do **not** add SKUs 4–6 or agents 02–20 until:

1. All three checkboxes above are done, and  
2. You have at least one sale or one week of impressions per listing to judge SEO.

Then use `npm run etsy:daily` (with `GEMINI_API_KEY`) for the next researched SKU — see launch pack **Next automated step**.

---

## Quick reference — files on disk

| Path | Role |
|------|------|
| `products/ai-client-onboarding-agent-kit/` | Source for SKU 1 |
| `products/marketing-command-center-excel/` | Source for SKU 2 |
| `products/agent-01-etsy-listing-seo-agent-kit/` | Source for SKU 3 |
| `dist/*.zip` | Customer downloads (build locally) |
| `config/etsy-store.yaml` | Niche / store config (copy from `.example` if missing) |

**Done definition:** Three active digital listings, each with 13 tags, 5+ images, ZIP attached, and policies live.
