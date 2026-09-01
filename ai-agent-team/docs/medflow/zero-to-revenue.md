# MedFlow — Zero-to-Revenue Field Guide (condensed)

**Position:** Solo founder, no healthcare network, can build. Southeast Michigan independent specialty practices.

**Build first:** Ambient scribe + billing code overlay. Browser-based. Physician approves everything before submit. No FDA 510(k).

**Do not pursue first:** Health systems (Michigan Medicine, Henry Ford, Beaumont, Ascension).

---

## Core equation

Background gap = first-customer problem, not fatal flaw.

```
Demo → SPARK → warm intro → 90-day free pilot → testimonial → case study → paid → referrals
```

---

## Tech stack (MVP)

| Component | Choice | Cost signal |
|-----------|--------|-------------|
| Transcription | Deepgram Nova-3 or Whisper | ~$0.07 / 15-min encounter |
| Notes + codes | Claude Haiku/Sonnet | $0.02–0.12 / encounter |
| EHR overlay | Chrome extension MV3 | No native EHR integration |
| Backend | Supabase on AWS (BAA) | HIPAA-eligible |
| Billing | Stripe subscriptions | $200–350/provider/mo |

At 3 providers/practice × $250/mo = **$750 MRR/practice**. Marginal API ~$3–6/day at 50 visits.

---

## GTM sequence

| When | Action |
|------|--------|
| Months 1–2 | Build MVP; HIPAA from day one; LinkedIn build trail |
| Month 2 | Apply Ann Arbor SPARK |
| Months 2–4 | 1 design partner (2–6 physicians); 90 days free; BAA signed |
| Months 3–6 | Iterate until daily habitual use |
| Months 5–6 | Convert to paid; apply PitchMI |
| Months 6–12 | Grow to 5–10 practices via referrals |

---

## Best first specialties

1. **Psychiatry** — long notes, low-tech, high scribe motivation
2. **Urgent care** — volume, billing pressure, owner watches margins
3. **Dermatology** — procedure-heavy CPT patterns
4. **Primary care independents** — familiar with ambient category

---

## Competitive answer

Not "our AI is better." Win on:
- Built for practices like theirs
- You answer the phone
- Feature they need by Thursday
- Local Southeast Michigan understanding

---

## Year 1 ARR scenarios ($250/provider, ~3 providers/practice)

| Scenario | Practices | ARR |
|----------|-----------|-----|
| Conservative | 3 | ~$27K |
| Realistic | 8 | ~$72K |
| Optimistic | 15 | ~$135K |

---

## Capital (filtered for solo, no U-M affiliation)

| Source | When | Notes |
|--------|------|-------|
| Ann Arbor SPARK | **Now** | Intros > money |
| PitchMI (MEDC) | Month 5–6 | Need working product + 1 paying customer |
| Michigan Rise | Month 6+ | Rolling, traction proof |
| Arboretum Ventures | Year 2–3 | $500K+ ARR target |

---

## Honest risks

- Market not empty (Nuance DAX, Suki, Abridge, Nabla) — compete on service
- Cold outreach ≈ 0% — need warm intros (SPARK, referrals)
- HIPAA is day-one — BAA template ~$500 before first pilot
