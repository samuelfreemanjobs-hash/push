import { buildScorecard } from './excellence-scorecard.mjs';

const ACME = 'Acme Digital Studio (fictional sample — replace with your business)';

function ex(intro, body) {
  return `# Example output — ${ACME}

${intro}

---

${body}

---

*This sample shows what “good” looks like. Run the matching workflow with your real inputs.*
`;
}

export const EXCELLENCE_V2 = {
  'agent-01-etsy-listing-seo-agent-kit': {
    scorecard: buildScorecard('Etsy Listing SEO', [
      { name: 'Primary keyword fit', weight: 25, hint: 'Primary phrase matches product + buyer intent' },
      { name: 'Title clarity & front-load', weight: 20, hint: '≤140 chars; keyword in first 40 chars' },
      { name: '13 unique tags', weight: 20, hint: 'No shop-wide duplicates; ≤20 chars each' },
      { name: 'Description converts', weight: 20, hint: 'Hook, bullets, license, FAQ' },
      { name: 'Image SEO plan', weight: 15, hint: 'Alt text drafted for every image' },
    ]),
    example: ex(
      'Sample listing SEO package for a digital planner SKU.',
      `**Primary phrase:** budget planner printable pdf

**Title:** Budget Planner Printable PDF | Monthly Expense Tracker | A5 Letter Digital Download

**Tags (13):** budget planner, expense tracker, printable pdf, monthly budget, digital download, finance planner, money organizer, savings tracker, paycheck budget, family budget, instant download, spreadsheet alternative, minimalist planner

**Audit score:** 88/100 — strengthen FAQ on refund policy.`,
    ),
    artifact: {
      name: '07-artifacts/shop-tag-dedupe-tracker.csv',
      content: `listing_id,listing_title,tag,duplicate_of_listing,action
101,Budget Planner PDF,budget planner,,keep
102,Expense Tracker,budget planner,101,replace tag with expense tracker
103,Marketing Kit,small business,,keep
`,
    },
  },
  'agent-02-social-content-machine-agent-kit': {
    scorecard: buildScorecard('Social Content Machine', [
      { name: 'Pillar alignment', weight: 25, hint: 'Each post ties to 1 of 3–5 pillars' },
      { name: 'Platform-native tone', weight: 25, hint: 'LinkedIn ≠ Instagram caption length' },
      { name: 'CTA clarity', weight: 20, hint: 'One CTA per post' },
      { name: '30-day coverage', weight: 20, hint: 'No empty weekdays you care about' },
      { name: 'Repurpose plan', weight: 10, hint: 'At least 4 posts repurposed from 1 idea' },
    ]),
    example: ex(
      'Week 1 snippet from a 30-day calendar for a bookkeeping consultant.',
      `| Day | Channel | Hook | CTA |
| Mon | LinkedIn | Most founders don't have a tax problem—they have a visibility problem. | Comment "checklist" |
| Wed | Instagram | 3 numbers to check every Friday (carousel slide 1) | Link in bio |
| Fri | Email | Weekly money snapshot template tease | Download magnet |`,
    ),
    artifact: {
      name: '07-artifacts/30-day-content-calendar-template.csv',
      content: `date,channel,pillar,hook_draft,format,cta,status
2026-01-06,LinkedIn,Education,Problem-aware hook here,carousel,Book call,Draft
2026-01-07,Instagram,Proof,Client result (anonymized),reel,DM keyword,Draft
`,
    },
  },
  'agent-03-marketing-planner-agent-kit': {
    scorecard: buildScorecard('Marketing Planner', [
      { name: 'KPIs defined', weight: 25, hint: 'Each metric has goal + owner' },
      { name: 'Weekly plan realistic', weight: 25, hint: 'One primary offer + channel focus' },
      { name: 'Calendar populated', weight: 25, hint: '12 weeks of slots or intentional blanks' },
      { name: 'Campaign ROI tracked', weight: 15, hint: 'Spend/leads/revenue columns used' },
      { name: 'Lead sources reviewed', weight: 10, hint: 'Top 3 sources have next action' },
    ]),
    example: ex(
      'Sample dashboard narrative for Week 2.',
      `**Focus offer:** Marketing Command Center Excel  
**Channel:** Pinterest + Etsy  
**KPI:** 12 email opt-ins (goal 15) — add pin linking to lead magnet workflow 13.`,
    ),
    artifact: {
      name: '07-artifacts/weekly-marketing-review.md',
      content: `# Weekly marketing review — [YOUR BRAND]

**Week of:** __________

| Metric | Goal | Actual | Notes |
|--------|------|--------|-------|
| Leads | | | |
| Revenue | | | |
| Email subs | | | |

**Wins:**
**Misses:**
**One bet for next week:**
`,
    },
  },
  'agent-04-client-onboarding-agent-kit': {
    scorecard: buildScorecard('Client Onboarding', [
      { name: 'Welcome within 2h SLA', weight: 20, hint: 'Timestamp in tracker' },
      { name: 'Kickoff scheduled ≤7 days', weight: 20, hint: 'Date in tracker' },
      { name: 'Scope in writing', weight: 25, hint: 'Recap email sent' },
      { name: 'Portal/assets delivered', weight: 20, hint: 'Checklist complete' },
      { name: 'First value ≤14 days', weight: 15, hint: 'Milestone dated' },
    ]),
    example: ex(
      'Sample tracker row after kickoff.',
      `Client: River & Co. Consulting | Kickoff: 2026-01-12 | Status: Plan locked | Next: deliver audit PDF by 1/19`,
    ),
    artifact: {
      name: '07-artifacts/onboarding-milestone-checklist.md',
      content: `# Onboarding milestone checklist — [Client Name]

- [ ] Welcome sent (date: ___)
- [ ] Intake received
- [ ] Kickoff completed
- [ ] Recap confirmed in writing
- [ ] First deliverable scheduled
- [ ] Onboarding complete
`,
    },
  },
  'agent-05-email-sequence-agent-kit': {
    scorecard: buildScorecard('Email Sequence', [
      { name: 'Subject line clarity', weight: 25, hint: '≤50 chars; no spam triggers' },
      { name: 'Single CTA per email', weight: 25, hint: 'One primary action' },
      { name: 'Sequence logic', weight: 25, hint: 'Each email has a reason to exist' },
      { name: 'Voice match', weight: 15, hint: 'Matches brand voice card' },
      { name: 'Compliance', weight: 10, hint: 'Physical address / unsubscribe if broadcast' },
    ]),
    example: ex(
      'Email 1 of welcome sequence for a template shop.',
      `**Subject:** Your download is ready (+ one tip)

Hi Maya — thanks for grabbing the Marketing Command Center. Start on the **Dashboard** tab and enter last week's numbers before you edit campaigns. If anything won't open on mobile, reply and we'll help.`,
    ),
    artifact: {
      name: '07-artifacts/email-sequence-planner.csv',
      content: `email_num,name,trigger_day,subject,primary_cta,status
1,Welcome,0,Your download is ready,Open file,Draft
2,Quick win,2,The one tab most sellers skip,Watch 2-min tip,Draft
3,Case pattern,5,How Alex booked 3 calls,Book onboarding kit,Draft
`,
    },
  },
  'agent-06-ad-copy-agent-kit': {
    scorecard: buildScorecard('Ad Copy', [
      { name: 'Angle distinct', weight: 25, hint: 'Pain / aspiration / proof differ' },
      { name: 'Character limits', weight: 20, hint: 'Headlines within platform caps' },
      { name: 'Offer match', weight: 25, hint: 'Ad matches landing page promise' },
      { name: 'Compliance pass', weight: 20, hint: 'No prohibited claims' },
      { name: 'Test plan', weight: 10, hint: 'Hypothesis + kill criteria' },
    ]),
    example: ex(
      'Meta primary text variant (pain angle).',
      `Still rebuilding client onboarding from scratch every sale? The Acme kit gives you SOP + emails + tracker—deploy this week. Instant download.`,
    ),
    artifact: {
      name: '07-artifacts/ad-compliance-checklist.md',
      content: `# Ad compliance checklist — [YOUR BRAND]

- [ ] No income guarantees
- [ ] No "cure" or medical claims
- [ ] Landing page matches ad promise
- [ ] Testimonials have permission
- [ ] Special category rules checked (if applicable)
`,
    },
  },
  'agent-07-sales-proposal-discovery-agent-kit': {
    scorecard: buildScorecard('Sales Proposal & Discovery', [
      { name: 'Discovery captured', weight: 25, hint: 'Goals, constraints, decision process' },
      { name: 'Scope in/out', weight: 25, hint: 'Explicit exclusions' },
      { name: 'Pricing clear', weight: 20, hint: 'Line items or tiers' },
      { name: 'Timeline dated', weight: 20, hint: 'Milestones with dates' },
      { name: 'Next step', weight: 10, hint: 'Single approval action' },
    ]),
    example: ex(
      'Executive summary excerpt.',
      `River & Co. needs a repeatable client onboarding system in 21 days. Phase 1: SOP + templates ($2,400). Phase 2: team training call ($600). Out of scope: CRM migration.`,
    ),
    artifact: {
      name: '07-artifacts/proposal-fill-in-template.md',
      content: `# Proposal — [Client]

## Executive summary
[Outcome in one paragraph]

## Scope
**In:** 
**Out:** 

## Timeline
| Milestone | Date |
|-----------|------|

## Investment
| Item | Amount |

## Next step
Reply "approved" by [date] to begin.
`,
    },
  },
  'agent-08-copy-swipe-agent-kit': {
    scorecard: buildScorecard('Copy Swipe File', [
      { name: 'Placeholder replaced', weight: 30, hint: 'No [brackets] left' },
      { name: 'Offer clear', weight: 25, hint: 'Reader knows what they get' },
      { name: 'CTA specific', weight: 20, hint: 'Verb + outcome' },
      { name: 'Channel fit', weight: 15, hint: 'Email vs social vs ad' },
      { name: 'Legal tone', weight: 10, hint: 'No false guarantees' },
    ]),
    example: ex(
      'Filled value prop (template #1).',
      `We help Etsy digital sellers launch listings 3× faster with SEO + mockup agent kits—without hiring a copywriter.`,
    ),
    artifact: {
      name: '07-artifacts/copy-tracker.csv',
      content: `template_id,channel,draft_status,used_on_date,result_notes
1,Homepage,Done,, 
33,Meta ad,Draft,, 
`,
    },
  },
  'agent-09-pod-design-prompt-agent-kit': {
    scorecard: buildScorecard('POD Design Prompt', [
      { name: 'Niche specific', weight: 25, hint: 'Not generic "cool design"' },
      { name: 'Style lock', weight: 25, hint: 'Repeatable visual rules' },
      { name: 'Print safe', weight: 20, hint: 'Contrast / text size noted' },
      { name: 'Listing tie-in', weight: 20, hint: 'Title angle matches design' },
      { name: 'QC pass', weight: 10, hint: 'Artifact checklist complete' },
    ]),
    example: ex(
      'POD prompt snippet for minimalist nurse mug.',
      `White ceramic mug mockup, minimalist line art stethoscope heart, soft sage accent, studio lighting, no text, print-on-demand safe margins`,
    ),
    artifact: {
      name: '07-artifacts/pod-prompt-log.csv',
      content: `niche,product_type,prompt_id,style_lock,listing_title_draft,tested_yes_no
nurse gifts,mug,P-001,minimal line sage,,no
`,
    },
  },
  'agent-10-listing-mockup-photo-brief-agent-kit': {
    scorecard: buildScorecard('Listing Mockup & Photo Brief', [
      { name: 'Shot list complete', weight: 25, hint: 'Covers hero + detail + scale' },
      { name: 'Brand consistent', weight: 25, hint: 'Colors/fonts match shop' },
      { name: 'Alt text drafted', weight: 20, hint: 'Every image' },
      { name: 'Thumbnail test', weight: 15, hint: '2 variants planned' },
      { name: 'QC before upload', weight: 15, hint: 'Workflow 10 complete' },
    ]),
    example: ex(
      'Shot 1 brief for digital download listing.',
      `**Shot 1 (Hero):** Laptop on desk showing PDF planner page; overlay text "Instant Download" bottom third; warm daylight.`,
    ),
    artifact: {
      name: '07-artifacts/listing-shot-list.csv',
      content: `image_num,shot_type,scene,props,text_overlay,alt_text_draft,done
1,Hero,laptop desk,planner pdf,Instant Download,,no
2,Detail,close-up page,,,,
`,
    },
  },
  'agent-11-customer-support-reply-agent-kit': {
    scorecard: buildScorecard('Customer Support Reply', [
      { name: 'Empathy first', weight: 25, hint: 'Acknowledge before solving' },
      { name: 'Specific answer', weight: 25, hint: 'References their issue' },
      { name: 'Correct policy', weight: 25, hint: 'Refund/shipping accurate' },
      { name: 'Tone match', weight: 15, hint: 'Calm for angry tickets' },
      { name: 'Macro updated', weight: 10, hint: 'Reusable if repeat issue' },
    ]),
    example: ex(
      'De-escalation reply excerpt.',
      `I'm sorry the file didn't open on your phone—that's frustrating. The kit is optimized for desktop PDF/Excel; here's the direct link again plus steps for Google Sheets import.`,
    ),
    artifact: {
      name: '07-artifacts/support-macro-library.csv',
      content: `category,severity,macro_name,body_shortcut
billing,P2,invoice_reminder,Hi {name}—invoice {id} due {date}: {link}
technical,P3,file_wont_open,Try desktop + link {url}; reply with screenshot
`,
    },
  },
  'agent-12-sop-operations-doc-agent-kit': {
    scorecard: buildScorecard('SOP & Operations Doc', [
      { name: 'Trigger defined', weight: 20, hint: 'When SOP starts' },
      { name: 'Steps testable', weight: 25, hint: 'Someone else can follow' },
      { name: 'Roles named', weight: 20, hint: 'RACI or owner per step' },
      { name: 'Quality gate', weight: 20, hint: 'Checklist before done' },
      { name: 'Version/date', weight: 15, hint: 'Header metadata' },
    ]),
    example: ex(
      'SOP step excerpt: weekly listing publish.',
      `**Step 4:** Run listing audit scorecard (Agent 01 workflow 08). Publish only if ≥85/100.`,
    ),
    artifact: {
      name: '07-artifacts/sop-template.md',
      content: `# SOP — [Process Name] | v1.0 | Owner: [Name]

**Trigger:** 
**Goal:** 

## Steps
1. 
2. 

## Quality checklist
- [ ] 

## Revision log
| Date | Change |
|------|--------|
`,
    },
  },
  'agent-13-lead-magnet-opt-in-agent-kit': {
    scorecard: buildScorecard('Lead Magnet & Opt-in', [
      { name: 'Promise specific', weight: 30, hint: 'Outcome in title' },
      { name: 'Landing clarity', weight: 25, hint: 'Above-fold benefit' },
      { name: 'Nurture logic', weight: 25, hint: '5 emails build to offer' },
      { name: 'Delivery works', weight: 10, hint: 'Test link' },
      { name: 'GDPR/CAN-SPAM', weight: 10, hint: 'Consent language' },
    ]),
    example: ex(
      'Lead magnet title + promise.',
      `**Title:** Etsy Listing SEO Cheat Sheet (1 page)  
**Promise:** Audit any listing in 10 minutes using the scorecard.`,
    ),
    artifact: {
      name: '07-artifacts/lead-magnet-one-pager-outline.md',
      content: `# [Lead Magnet Title]

## Page 1 — The problem
## Page 2 — The framework (3 steps)
## Page 3 — Checklist
## CTA — Work with [YOUR BRAND]
`,
    },
  },
  'agent-14-direct-mail-campaign-agent-kit': {
    scorecard: buildScorecard('Direct Mail Campaign', [
      { name: 'Single offer', weight: 30, hint: 'One CTA only' },
      { name: 'Copy fits 4x6', weight: 25, hint: 'Readable at arm\'s length' },
      { name: 'Tracking plan', weight: 20, hint: 'URL or code' },
      { name: 'List quality', weight: 15, hint: 'Audience defined' },
      { name: 'Print spec met', weight: 10, hint: 'Bleed/safe zone' },
    ]),
    example: ex(
      'Postcard headline sample.',
      `**Headline:** Local shops: still guessing which Etsy tags work?`,
    ),
    artifact: {
      name: '07-artifacts/direct-mail-campaign-tracker.csv',
      content: `campaign_name,drop_date,pieces_mailed,cost,leads,revenue,notes
Q1 local B2B,,,0,0,0,
`,
    },
  },
  'agent-15-brand-voice-messaging-agent-kit': {
    scorecard: buildScorecard('Brand Voice & Messaging', [
      { name: 'Positioning crisp', weight: 30, hint: 'One sentence for who/what/why' },
      { name: 'Pillars provable', weight: 25, hint: 'Each has proof' },
      { name: 'Voice consistent', weight: 25, hint: 'We are / we are not' },
      { name: 'Channel variants', weight: 10, hint: 'Etsy vs email lead' },
      { name: 'No jargon', weight: 10, hint: '12-year-old test' },
    ]),
    example: ex(
      'Positioning sentence.',
      `For Etsy digital sellers, Acme kits are the fastest way to ship listing-ready systems—not prompt dumps.`,
    ),
    artifact: {
      name: '07-artifacts/brand-voice-card.md',
      content: `# Brand voice card — [YOUR BRAND]

**We are:** direct, helpful, operator-minded  
**We are not:** hypey, vague, guru-ish  

| Dimension | Do | Don't |
|-----------|----|-------|
| Tone | | |
| Vocabulary | | |

**Banned phrases:** 
**Signature CTA:** 
`,
    },
  },
  'agent-16-blog-seo-article-agent-kit': {
    scorecard: buildScorecard('Blog & SEO Article', [
      { name: 'Intent match', weight: 25, hint: 'Answers search intent' },
      { name: 'Structure scannable', weight: 25, hint: 'H2s + bullets' },
      { name: 'Meta optimized', weight: 20, hint: 'Title/meta length' },
      { name: 'Internal links', weight: 15, hint: '2+ relevant links' },
      { name: 'FAQ useful', weight: 15, hint: 'Real buyer questions' },
    ]),
    example: ex(
      'Meta pack sample.',
      `**Title:** Etsy SEO for Digital Downloads: 2026 Checklist (58 chars)  
**Meta:** Audit titles, tags, and images before you publish your next template listing. (78 chars)`,
    ),
    artifact: {
      name: '07-artifacts/article-outline-template.md',
      content: `# [H1 Title]

**Target keyword:** 
**Intent:** 

## Introduction
## [H2 section]
## FAQ
## CTA
`,
    },
  },
  'agent-17-business-in-a-box-orchestrator-agent-kit': {
    scorecard: buildScorecard('Business-in-a-Box Orchestrator', [
      { name: 'SKU scored', weight: 30, hint: 'Every product in matrix' },
      { name: 'Bundle logic', weight: 25, hint: 'Anchor + upsell clear' },
      { name: '90-day plan', weight: 25, hint: 'Weekly actions' },
      { name: 'Metrics reviewed', weight: 10, hint: 'Weekly ritual' },
      { name: 'Kill decisions', weight: 10, hint: 'At least 1 SKU deprioritized' },
    ]),
    example: ex(
      'SKU score excerpt.',
      `Onboarding Kit: demand 9, effort 4, score 8.5 → **scale** with Pinterest pins.`,
    ),
    artifact: {
      name: '07-artifacts/sku-priority-matrix.csv',
      content: `sku,demand_1_10,effort_1_10,margin_1_10,total_score,action
Onboarding Kit,9,4,8,8.5,scale
Postcard Pack,5,3,6,6.0,maintain
`,
    },
  },
  'agent-18-ai-ops-admin-agent-kit': {
    scorecard: buildScorecard('AI Ops & Admin', [
      { name: 'Task routed', weight: 25, hint: 'Correct agent/workflow chosen' },
      { name: 'Human review', weight: 25, hint: 'External sends approved' },
      { name: 'Time saved', weight: 20, hint: 'Note minutes saved' },
      { name: 'No hallucinated facts', weight: 20, hint: 'Assumptions flagged' },
      { name: 'Logged', weight: 10, hint: 'Output filed with date' },
    ]),
    example: ex(
      'Daily standup pack sample.',
      `**Today:** finish Agent 19 stats paste; publish 1 listing; defer partnership email to Friday.`,
    ),
    artifact: {
      name: '07-artifacts/agent-routing-map.md',
      content: `# Which agent kit for which task?

| Task | Kit # |
|------|-------|
| New Etsy listing SEO | 01 |
| Weekly shop stats | 19 |
| New client signed | 04 |
| Lead magnet launch | 13 |
`,
    },
  },
  'agent-19-etsy-shop-analytics-agent-kit': {
    scorecard: buildScorecard('Etsy Shop Analytics', [
      { name: 'Data complete', weight: 25, hint: '28-day window' },
      { name: 'Diagnosis specific', weight: 25, hint: 'CTR vs conversion separated' },
      { name: '3 experiments max', weight: 25, hint: 'Focused backlog' },
      { name: 'Listing-level actions', weight: 15, hint: 'Named SKUs' },
      { name: 'Review loop scheduled', weight: 10, hint: 'Next Friday cal block' },
    ]),
    example: ex(
      'Weekly diagnosis snippet.',
      `CTR down 12% shop-wide — hero thumbnails on listings #4 and #7; test brighter mockups.`,
    ),
    artifact: {
      name: '07-artifacts/etsy-stats-weekly-paste.csv',
      content: `metric,this_week,last_week,notes
visits,1200,1350,
orders,18,22,
conversion_pct,1.5,1.6,
top_listing,Onboarding Kit,,
`,
    },
  },
  'agent-20-coach-consultant-professional-agent-kit': {
    scorecard: buildScorecard('Coach & Consultant Professional', [
      { name: 'ICP specific', weight: 25, hint: 'Who + pain + budget' },
      { name: 'Package clear', weight: 25, hint: 'Deliverables + boundaries' },
      { name: 'Intake thorough', weight: 20, hint: 'Required questions' },
      { name: 'Session recap sent', weight: 20, hint: 'Actions dated' },
      { name: 'Renewal path', weight: 10, hint: 'Next engagement proposed' },
    ]),
    example: ex(
      'Package one-liner.',
      `**6-week Operations Accelerator:** weekly call + async Voxer + SOP templates—$3,200.`,
    ),
    artifact: {
      name: '07-artifacts/coaching-intake-template.md',
      content: `# Client intake — [YOUR BRAND]

1. What outcome do you want in 90 days?
2. What have you tried?
3. Budget range:
4. Decision date:
5. Success metric:
`,
    },
  },
};

export const CHANGELOG_V2 = `# Changelog

## 2.0.0 — Excellence pass
- Added \`04-examples/\` with Acme sample outputs
- Added \`06-scorecard.md\` (100-point rubric)
- Added \`07-artifacts/\` fillable templates
- Added VERSION.txt

## 1.0.0 — Initial release
- Playbook, 10 workflows, setup guide
`;

export const VERSION = '2.0.0';
