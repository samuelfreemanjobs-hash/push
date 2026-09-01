# MedFlow Operating System

Fresh-slate operating model for **MedFlow** — ambient scribe + billing code overlay for independent specialty practices in Southeast Michigan.

Replaces the legacy Freeman Intelligence OS multi-agent sprawl with a **small number of durable loops** tuned to healthcare's trust threshold, HIPAA requirements, and a solo founder who builds.

---

## Product (what we're building)

| Layer | Choice | Why |
|-------|--------|-----|
| **Wedge** | Ambient scribe + CPT/ICD-10 suggestions + physician approval | No FDA 510(k); human-in-the-loop before any submission |
| **ICP** | Independent specialty practices, 2–8 physicians, Washtenaw/Oakland/Wayne | Owner = buyer; weeks not quarters |
| **First specialties** | Psychiatry, urgent care, dermatology | High note burden or billing pressure; less enterprise lock-in |
| **Delivery** | Browser overlay (Chrome ext) over web EHRs | No native integration required for MVP |
| **Stack** | Deepgram/Whisper → Claude → Supabase (AWS BAA) → Stripe | ~85–90% margin at $250/provider/mo |
| **Moat (Y1–2)** | Local service, specialty templates, ship features by Thursday | Not "better AI" vs Nuance/Suki/Abridge |

**Skip for now:** Michigan Medicine, Henry Ford, Beaumont, Ascension — Year 3–4 buyers.

---

## Architecture (automation spine)

```
┌─────────────────────────────────────────────────────────────┐
│  SLACK (operator + future team)                              │
│  #executive-assistant  #medflow-product  #medflow-pipeline   │
│  #medflow-compliance   #shipped          #dev                │
└───────────────┬─────────────────────────────────────────────┘
                │ slash commands + notifications
┌───────────────▼─────────────────────────────────────────────┐
│  n8n (Railway) — schedules, webhooks, light routing          │
│  • EA daily dashboard (weekday 7 AM)                         │
│  • /ea commands                                              │
│  • (Phase 2) design-partner follow-up reminders              │
└───────────────┬─────────────────────────────────────────────┘
                │ x-n8n-secret
┌───────────────▼─────────────────────────────────────────────┐
│  push runtime (Railway) — business logic, not raw LLM chat   │
│  • EA dashboard from Supabase CRM                            │
│  • (Phase 2) encounter pipeline orchestration                │
│  • (Phase 3) Stripe webhooks, pilot → paid conversion        │
└───────────────┬─────────────────────────────────────────────┘
                │
┌───────────────▼─────────────────────────────────────────────┐
│  Supabase (HIPAA-eligible AWS, BAA) — single source of truth │
│  CRM: leads, opportunities, follow_ups, tasks, gates         │
│  Product: encounters, note_drafts, approvals (Phase 2)       │
└─────────────────────────────────────────────────────────────┘
```

**Rule:** n8n moves data and triggers. push runtime owns business rules. Supabase owns state. No Airtable.

---

## Salvaged from Freeman Intelligence OS

| Keep | Adapt for MedFlow |
|------|-------------------|
| Slack → n8n → Railway runtime | Same spine; fewer channels |
| Executive Assistant daily dashboard | Gates = 12-month playbook milestones |
| `#shipped` for done work | Linear issue → Done → Slack |
| Human approval before external impact | **Physician approves every note/code before submit** |
| Fail-closed on schema/compliance | HIPAA + note template validation = HALT |
| Operator gates on daily dashboard | BAA signed, SPARK applied, pilot live, etc. |
| Linear for builds (not CRM) | Product + compliance work only |
| Warm intro > cold outreach | SPARK, PitchMI, referrals — never spray physician offices |

| Discard | Why |
|---------|-----|
| 10+ agent Slack channels (ceo, sales, research, proposal…) | Solo founder; one product, one pipeline |
| Airtable CRM | Supabase replaces it |
| FlipReseller / freight / Etsy lanes | Different business |
| GitHub Actions 11-job daily agent team | Overkill; n8n + EA + focused build time |
| "Our AI is better" positioning | Lose on credibility; win on service + local |
| Enterprise health system pursuit | 18+ month procurement; burns runway |

---

## Slack channel map (minimal)

| Channel | Purpose |
|---------|---------|
| `#executive-assistant` | Daily dashboard, `/ea` commands, streak |
| `#medflow-product` | MVP build, clinical feedback, edge cases |
| `#medflow-pipeline` | Design partners, SPARK intros, pilot status |
| `#medflow-compliance` | BAA, HIPAA, incident response — nothing ships without this |
| `#dev` | PRs, deploys, Railway/n8n |
| `#shipped` | Linear Done → celebrate + case study material |

---

## Twelve-month playbook → operator gates

Gates surface on the daily EA dashboard from Supabase `operator_gates`. Complete → mark `done`.

### Months 1–2 — Build + SPARK
- [P0] HIPAA infra live (AWS BAA + Supabase encryption documented)
- [P0] BAA template from healthcare attorney (~$500)
- [P0] Core loop MVP: audio → transcript → SOAP draft → CPT suggestions → approval UI
- [P0] Apply Ann Arbor SPARK (SPARK Central Innovation Center)
- [P1] LinkedIn build-in-public cadence (2×/week minimum)
- [P1] n8n EA daily loop live on Railway

### Months 2–4 — Design partner
- [P0] Warm intro to 1 pilot clinic (via SPARK, not cold)
- [P0] BAA signed with design partner before any PHI
- [P1] Chrome extension overlay prototype (Manifest V3)
- [P1] Specialty note template v1 (psych / urgent care / derm — pick one)

### Months 3–6 — Product-market fit signal
- [P0] Design partner uses tool daily without prompting
- [P0] Note formatting correct for their specialty template
- [P1] Weekly feedback calls logged in Supabase

### Months 5–6 — Revenue proof
- [P0] Design partner converts to paid ($250/provider/mo)
- [P0] Apply PitchMI (MEDC)
- [P1] Written testimonial + case study published

### Months 6–12 — Scale on referrals
- [P0] 5 paying practices ($45K+ ARR trajectory)
- [P1] First customer refers 1+ peer practice
- [P1] Michigan Rise pre-seed application (with traction proof)

---

## CRM model (Supabase)

| Table | MedFlow use |
|-------|-------------|
| `leads` | Practice contacts from SPARK, events, referrals |
| `opportunities` | Pilot → paid pipeline ($750/mo avg practice) |
| `follow_ups` | SPARK follow-up, pilot check-ins, BAA status |
| `tasks` | Build tasks; `revenue_task=true` for top daily item |
| `operator_gates` | Playbook milestones (above) |
| `goals` | Monthly ARR targets, pilot count |
| `dashboard_runs` | EA streak / accountability |

**Phase 2 tables:** `encounters`, `note_drafts`, `code_suggestions`, `physician_approvals`

---

## Compliance non-negotiables

1. **BAA before PHI** — no recordings until signed
2. **HIPAA-eligible cloud** — Supabase on AWS with BAA from day one
3. **Human approval** — physician reviews every note and code suggestion before submission
4. **Incident response doc** — written before first pilot
5. **"BAA-ready"** — selling point on every demo

---

## What success looks like (Month 12)

| Metric | Target |
|--------|--------|
| Paying practices | 5–10 |
| ARR | $45–90K (realistic path to $72K) |
| Case study | 1 strong written testimonial |
| Capital | SPARK engaged + grant app in progress |
| Product | Habitual daily use at design partner + 4+ referrals |

Year 1 goal is **traction proof**, not replacement salary.

---

## Phase 2 build priorities (product)

1. Encounter capture API (Web Audio → Deepgram/Whisper)
2. Note + code generation (Claude, structured output schema)
3. Physician approval queue (HITL UI — salvage `#proposal-reviews` pattern)
4. Chrome extension overlay
5. Stripe subscription ($250/provider/mo)

---

## Related docs

- `docs/medflow/zero-to-revenue.md` — condensed field guide
- `docs/medflow/wedge.md` — product wedge + competitive answer
- `config/ea-dashboard.yaml` — daily dashboard copy
- `integrations/n8n/README.md` — automation setup
