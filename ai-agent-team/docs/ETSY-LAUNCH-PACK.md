# Launch pack — business tools & AI outcome kits

Configured for `config/etsy-store.yaml`. Use this until `GEMINI_API_KEY` is set and `npm run etsy:daily` runs live.

## Shop positioning

**Promise:** Ready-to-run digital systems for founders — dashboards, agent workflows, and marketing kits sold as **outcomes**, not generic prompt lists.

**Primary buyer:** Solopreneurs, agency owners, and SMB operators who want implementation in under an hour.

## Flagship SKU ladder (recommended order)

| Priority | Product | Price | Outcome |
|----------|---------|-------|---------|
| 1 | AI Client Onboarding Agent Kit | $24.99 | New clients onboarded with SOPs + email prompts + checklist |
| 2 | Marketing Command Center (Excel) | $19.99 | Weekly KPIs, campaigns, and content calendar in one workbook |
| 3 | Postcard Direct-Mail Campaign Pack | $14.99 | Print-ready postcard + copy formulas + mailing checklist |
| 4 | Business-in-a-Box Starter (Bundle) | $59.99 | Onboarding kit + marketing dashboard + 50 copy templates |
| 5 | AI Ops Agent Library (50 workflows) | $34.99 | Repeatable agent briefs for sales, support, and admin |

## Listing #1 (paste into Etsy) — AI Client Onboarding Agent Kit

**Title (use keyword pass option A):**  
`Client Onboarding Template Kit | SOP AI Workflows Checklist | Freelancer Agency Digital Download`

Full SEO set for all 3 listings: **[ETSY-KEYWORD-PASS.md](ETSY-KEYWORD-PASS.md)**

**13 tags:** see keyword pass listing #1 (optimized for search intent, not generic “ai prompt kit” alone).

**Price:** $24.99  
**Quantity:** 999  
**Type:** Digital download

**Description:**

```markdown
Stop rewriting onboarding emails from scratch. This kit gives you a complete **client onboarding system** you can deploy this week—structured SOPs, copy-paste AI agent briefs, and a tracker so nothing falls through the cracks.

**Perfect for**
- Agency owners & freelancers
- Consultants productizing services
- SMB teams without an ops hire

**What's included (instant download)**
- Client Onboarding SOP (PDF + editable DOCX)
- 12 AI agent prompt workflows (welcome, intake, kickoff, handoff, follow-up)
- Onboarding checklist (spreadsheet)
- Email + message templates (plain text + markdown)
- 1-page implementation guide (15-minute setup)

**How it works**
1. Customize the SOP with your brand and tools.
2. Run each agent brief in your AI tool of choice for drafts you edit and send.
3. Track every client in the checklist until onboarding is complete.

**License**
Personal and commercial use for your business. No resale or redistribution of the files.

**FAQ**
*Do I need a paid AI subscription?* Any chat-based AI works; prompts are tool-agnostic.  
*Is this a Notion template?* Delivered as standard office files for maximum compatibility.  
*Refunds?* Digital items—contact me if files won't open and I'll fix it fast.

Questions? Message me before purchase—happy to help you pick the right kit.
```

**Mockup image prompts (for Canva/Midjourney):**

1. Clean laptop on desk showing checklist UI mock, headline "Onboard clients in 15 minutes"
2. Flat-lay: printed SOP, sticky notes, phone with message templates
3. Before/after graphic: chaotic inbox → organized onboarding pipeline

## 7-day marketing (LinkedIn + Pinterest)

| Day | Channel | Post angle |
|-----|---------|------------|
| Mon | LinkedIn | "I productized onboarding—what's inside an AI agent *outcome* kit" (carousel) |
| Tue | Pinterest | Pin: checklist mockup → link to listing |
| Wed | LinkedIn | Story: one client onboarded using prompt #3 |
| Thu | Instagram | Reel: 30-sec folder tour of download contents |
| Fri | Pinterest | Pin: postcard-style "welcome client" visual from kit |
| Sat | LinkedIn | Poll: biggest onboarding bottleneck |
| Sun | Email/Etsy | Etsy announcement + 10% launch coupon (optional) |

## Bundle rule

Always link Listing #1 from the $59.99 bundle description as the anchor; Etsy favors clear bundle value in the first 160 characters.

## Digital files (ready to upload)

Build customer ZIPs:

```bash
cd ai-agent-team
npm run products:build
```

Upload from `dist/`:

| ZIP | Listing |
|-----|---------|
| `ai-client-onboarding-agent-kit.zip` | AI Client Onboarding Agent Kit ($24.99) |
| `marketing-command-center-excel.zip` | Marketing Command Center ($19.99) |
| `postcard-direct-mail-pack.zip` | Postcard Direct-Mail Pack ($14.99) |
| `business-copywriting-templates.zip` | 50 Copywriting Templates (pair with SKU 4/5) |
| `ai-ops-agent-library.zip` | AI Ops Agent Library ($34.99) |
| `business-in-a-box-starter.zip` | Business-in-a-Box ($59.99) |

Source files live under `products/` for edits. Re-run `products:build` after changes.

## Next automated step

```bash
cp .env.example .env   # add GEMINI_API_KEY
npm run etsy:daily     # generates fresh research + next SKU listing JSON
```

With Etsy OAuth configured, add `--publish` to create a **draft** listing for human review before images are uploaded.
