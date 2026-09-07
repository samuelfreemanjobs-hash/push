# Micro SaaS Factory — Checklist

Use this after you deploy the starter. The boilerplate handles auth, billing, and deploy. **You** choose the one feature that makes people pay.

---

## Part 1: Choose your Micro SaaS idea

Complete this worksheet before you write product code. If you can't fill it in under 10 minutes, narrow the idea.

### Step 1 — One-sentence pitch

Fill in the blanks:

> I help **[specific person or role]** do **[one recurring task]** without **[their current painful method]**.

**Your pitch:**

> I help __________________________________________ do __________________________________________ without __________________________________________.

**Examples:**

- I help *indie Shopify owners* do *product description writing* without *hiring a copywriter*.
- I help *solo consultants* do *meeting follow-ups* without *rewriting notes manually*.
- I help *freelance designers* do *client feedback collection* without *scattered email threads*.

---

### Step 2 — Idea filter (score each 1–5)

| Question | 1 = weak, 5 = strong | Score |
|----------|----------------------|-------|
| Do I understand this customer's pain personally? | | |
| Does this pain happen **weekly** (not once a year)? | | |
| Would they pay **$19–49/mo** to fix it? | | |
| Can I build the core feature in **one weekend**? | | |
| Can I explain the product in **one sentence**? | | |
| Can I reach 100 potential customers without paid ads? | | |
| Does it work as a **simple web dashboard** (no mobile app required)? | | |

**Total:** _____ / 35

| Score | Verdict |
|-------|---------|
| **28–35** | Ship it. Start building today. |
| **20–27** | Narrow the niche or simplify the feature. |
| **Below 20** | Pick a different idea. Don't deploy code yet. |

---

### Step 3 — Pick your product type

Check the **one** type that best describes your idea:

- [ ] **AI wrapper** — input → AI → output (summaries, copy, drafts)
- [ ] **Generator / calculator** — replaces a spreadsheet or manual math
- [ ] **Workflow dashboard** — track, organize, or approve one type of work
- [ ] **Monitor / alert** — watch data and notify when something changes
- [ ] **Connector** — move or transform data between two tools
- [ ] **Vertical micro-tool** — one boring task for one industry
- [ ] **Builder tool** — tool for developers, founders, or creators like you

**Your type:** _________________________________

**The one thing inside the dashboard:** _________________________________

---

### Step 4 — Customer snapshot

| Field | Your answer |
|-------|-------------|
| **Who pays** (job title or role) | |
| **Where they hang out** (subreddit, Slack, LinkedIn, forum) | |
| **What they do today** (spreadsheet, freelancer, manual work, expensive tool) | |
| **What they get from you** (one concrete output) | |
| **Why monthly** (recurring data, history, alerts, AI credits, saved work) | |

---

### Step 5 — Scope lock (anti-bloat)

Your v1 includes **only** these. Everything else goes on the "later" list.

| In v1 (yes) | Out of v1 (no) |
|-------------|----------------|
| One dashboard feature | Second user role / admin panel |
| One paid plan | Usage-based pricing |
| Single-user or simple multi-user | Teams, seats, org billing |
| Manual onboarding | Complex integrations |
| You talking to 10 customers | Perfect UI polish |

**My v1 feature (one line):** _________________________________

**Explicitly NOT in v1:**

1. _________________________________
2. _________________________________
3. _________________________________

---

### Step 6 — Name and price

| Field | Your answer |
|-------|-------------|
| **Product name** | |
| **Domain** (check availability) | |
| **Monthly price** | $________ /mo |
| **Free trial or free tier?** | Yes / No — if yes: __________ |

**Price sanity check:**

- [ ] Similar tools charge at least this much (or more)
- [ ] One customer at this price covers your hosting + tools
- [ ] You'd pay this if you were the customer

---

### Step 7 — 10-minute validation (before you build)

Do this **before** adding your feature to the dashboard.

- [ ] Wrote my one-sentence pitch in a notes app
- [ ] Googled "[problem] tool" — found competitors (competition = validated demand)
- [ ] Posted or DMed **3 people** in my target niche with: *"Would you pay $X/mo for [one sentence]?"*
- [ ] Got at least **1 non-polite** response ("yes I'd pay" or "I use X but hate Y")
- [ ] Listed **3 places** I'll announce launch (community, social, email)

**If zero real interest after 3 conversations → narrow niche or change idea.**

---

### Stuck? Pick from these seeds

Choose one and customize the bracketed part:

1. AI [meeting notes → action items] for [consultants]
2. [Keyword] rank tracker for [local business type]
3. Client [content / feedback] approval board for [freelancers]
4. [Shopify / Etsy] [listing copy / tags] generator
5. [Job / lead / application] tracker for [role]
6. Competitor [price / feature] monitor for [industry]
7. [Invoice / payment] reminder dashboard for [solo agencies]
8. Niche prompt library for [Cursor users in X industry]
9. Waitlist + changelog for [indie product launches]
10. Form → [PDF report / summary] for [coaches / agencies]

**I chose seed #____ customized as:** _________________________________

---

### Cursor: your first build prompt

After this worksheet is done, open Cursor and paste:

```
I'm building on the Micro SaaS Factory Starter. My product:

- Customer: [from Step 4]
- One-sentence pitch: [from Step 1]
- Core dashboard feature: [from Step 5]
- Product type: [from Step 3]

Add the v1 feature to the dashboard. Keep auth and billing untouched.
Use Supabase for data. Match existing components and patterns.
Do not add: [from Step 5 "NOT in v1"]
```

---

## Part 2: First customer in 7 days

Deploy the starter first (`SETUP.md`). **Then** complete this checklist.

### Day 1 — Deploy + customize shell

- [ ] Starter deployed to production URL
- [ ] Product name, tagline, and pricing updated in `src/config/product.ts`
- [ ] Lemon Squeezy test checkout works end-to-end
- [ ] Idea worksheet (Part 1) completed

### Day 2 — Build v1 feature

- [ ] Core dashboard feature works for you (happy path)
- [ ] Data saves to Supabase
- [ ] Logged-out users see marketing page; logged-in users see dashboard

### Day 3 — Polish minimum

- [ ] Landing page explains **who it's for** and **what they get**
- [ ] One screenshot or 30-second Loom on the landing page
- [ ] "Manage billing" works in Settings

### Day 4 — Soft launch

- [ ] Shared with 5 people in your niche (DM, Slack, email — not public yet)
- [ ] Fixed the top 3 pieces of confusion they reported
- [ ] At least 1 person said they'd use or pay for it

### Day 5 — Public launch

- [ ] Posted in 1 community where your ICP lives
- [ ] Posted on 1 social channel with demo screenshot or video
- [ ] Email or DM to anyone who said "keep me posted"

### Day 6 — Follow up

- [ ] Replied to every comment and question
- [ ] Asked 3 interested people for a 15-minute call or async feedback
- [ ] Added FAQ item for the #1 question you got

### Day 7 — First dollar

- [ ] Turned off test mode / confirmed live billing
- [ ] Sent personal checkout link to 5 warm leads
- [ ] **Goal:** 1 paying customer OR 3 committed trials with follow-up date

---

## When you're stuck

| Problem | Fix |
|---------|-----|
| Idea too big | Cut until one dashboard screen does one thing |
| No one responds | Narrow the niche — "freelancers" → "freelance UX designers" |
| People like it but won't pay | Raise specificity or switch to a pain that costs them money today |
| Building forever | Ship ugly v1; your first customer funds v2 |
| Auth/billing broken | Stop feature work; finish `SETUP.md` first |

---

## What's next after your first customer

1. Fix what they complain about (not what you imagine)
2. Add one feature they asked for — only one
3. Automate ops with the **Claude Agent Workforce Kit** (support, onboarding, research)

**You don't need a bigger idea. You need one paying customer.**
