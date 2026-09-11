/** v2.1 — vertical input packs + 15s Loom scripts for all 20 agents */

const VERTICALS = {
  etsy: {
    label: 'Etsy digital seller',
    audience: 'Sells templates, printables, or digital downloads on Etsy',
    pain: 'Listings not ranking, inconsistent launches, overwhelmed by SEO and photos',
    offerExample: 'Agent kits + Excel command center',
    channel: 'Pinterest + Etsy search',
  },
  coach: {
    label: 'Coach / consultant',
    audience: 'Sells services, programs, or retainers to clients',
    pain: 'Inconsistent onboarding, vague proposals, content that does not convert',
    offerExample: '1:1 program or done-with-you implementation',
    channel: 'LinkedIn + email list',
  },
  local: {
    label: 'Local service business',
    audience: 'Shops, trades, or local B2B services with a geographic market',
    pain: 'Feast-or-famine leads, no follow-up system, weak direct mail',
    offerExample: 'Local lead gen + postcard campaigns',
    channel: 'Google Business Profile + direct mail',
  },
};

function verticalMd(agentTitle, workflow01Name, extraTips) {
  return (key) => {
    const v = VERTICALS[key];
    return `# Vertical pack — ${v.label}

**Agent:** ${agentTitle}

## Use this context in every workflow
- **Audience:** ${v.audience}
- **Core pain:** ${v.pain}
- **Example offer:** ${v.offerExample}
- **Primary channel:** ${v.channel}

## Suggested inputs for workflow 01 (${workflow01Name})
Paste into the workflow inputs before the agent brief:

\`\`\`
Niche: ${v.label}
Offer: ${v.offerExample}
Audience: ${v.audience}
Primary channel: ${v.channel}
Brand voice: direct, trustworthy, no hype
\`\`\`

## Niche tips
${extraTips[key]}

## Etsy listing angle (if you sell this agent)
Title lead with outcome for **${v.label.toLowerCase()}** buyers, not generic "AI prompts."
`;
  };
}

export const V21_BY_SLUG = {
  'agent-01-etsy-listing-seo-agent-kit': {
    w01: 'keyword-research-matrix',
    tips: {
      etsy: '- Prioritize long-tail template keywords; audit tag overlap across your shop.',
      coach: '- Keywords around "client onboarding template" not "coaching prompts."',
      local: '- Use geo-modifiers only when you sell local-specific templates.',
    },
    loom: {
      hook: 'Your Etsy listings can rank without guessing tags.',
      beats: ['Show 06-scorecard.md', 'Open 07-artifacts shop-tag dedupe CSV', 'Flash acme-sample title'],
      cta: 'Search "[YOUR BRAND] Etsy SEO agent" — instant download.',
    },
  },
  'agent-02-social-content-machine-agent-kit': {
    w01: 'content-pillars',
    tips: {
      etsy: '- Pillars: new listings, maker tips, customer outcomes, behind-the-scenes.',
      coach: '- Pillars: client wins, framework posts, myth-busting, soft CTA to call.',
      local: '- Pillars: local proof, before/after, team, seasonal offers.',
    },
    loom: {
      hook: '30 days of posts—planned in one sitting.',
      beats: ['Calendar CSV artifact', 'Example week in acme-sample', 'Workflow 01 file'],
      cta: 'Download the Social Content Machine agent kit.',
    },
  },
  'agent-03-marketing-planner-agent-kit': {
    w01: 'weekly-plan',
    tips: {
      etsy: '- KPI focus: visits, conversion, favorites, email opt-ins from Etsy.',
      coach: '- KPI: calls booked, show rate, close rate.',
      local: '- KPI: calls, foot traffic, coupon redemptions.',
    },
    loom: {
      hook: 'One Excel command center for your marketing week.',
      beats: ['Open Marketing-Command-Center.xlsx Example Acme tab', 'weekly-marketing-review.md'],
      cta: 'Get the Marketing Planner agent + spreadsheet.',
    },
  },
  'agent-04-client-onboarding-agent-kit': {
    w01: 'welcome-email-draft',
    tips: {
      etsy: '- N/A for productized buyers; use for B2B template licensing only.',
      coach: '- Emphasize kickoff within 7 days and written scope.',
      local: '- Include site access / safety docs in intake list.',
    },
    loom: {
      hook: 'Onboard clients in 14 days—without dropped balls.',
      beats: ['Tracker CSV', 'SOP folder', '12 workflows list'],
      cta: 'Client Onboarding Agent Kit—instant download.',
    },
  },
  'agent-05-email-sequence-agent-kit': {
    w01: 'welcome-sequence',
    tips: {
      etsy: '- Sequence: delivery → tip → review ask → cross-sell bundle.',
      coach: '- Sequence: welcome → story → case → invite to apply.',
      local: '- Sequence: thank you → review → referral → seasonal offer.',
    },
    loom: {
      hook: 'Five emails that sell while you sleep.',
      beats: ['email-sequence-planner.csv', 'acme-sample email 1'],
      cta: 'Email Sequence Agent Kit on Etsy.',
    },
  },
  'agent-06-ad-copy-agent-kit': {
    w01: 'meta-primary-text',
    tips: {
      etsy: '- Drive to Etsy shop or lead magnet, not off-platform checkout if policy-sensitive.',
      coach: '- Lead with transformation; use proof, not income claims.',
      local: '- Geo-target ads; mention response time.',
    },
    loom: {
      hook: 'Ad angles that match your landing page.',
      beats: ['ad-compliance-checklist.md', 'sample Meta text in examples'],
      cta: 'Ad Copy Agent Kit—download today.',
    },
  },
  'agent-07-sales-proposal-discovery-agent-kit': {
    w01: 'discovery-prep',
    tips: {
      etsy: '- Use for wholesale/partner proposals, not $12 digital SKUs.',
      coach: '- Mandatory: goals, budget, decision date, stakeholders.',
      local: '- Include site visit / estimate line items.',
    },
    loom: {
      hook: 'Discovery calls that end in clear proposals.',
      beats: ['proposal-fill-in-template.md', 'acme executive summary'],
      cta: 'Sales Proposal Agent Kit for consultants.',
    },
  },
  'agent-08-copy-swipe-agent-kit': {
    w01: 'value-prop-one-liner',
    tips: {
      etsy: '- Swipe #1 for shop announcement; #33 for Etsy Ads.',
      coach: '- Swipe #41–47 for sales emails.',
      local: '- Swipe postcard copy + email #11 reminder.',
    },
    loom: {
      hook: '50 templates plus AI refinement workflows.',
      beats: ['50-business-copywriting-templates.md', 'copy-tracker.csv'],
      cta: 'Copy Swipe Agent Kit bundle.',
    },
  },
  'agent-09-pod-design-prompt-agent-kit': {
    w01: 'niche-research',
    tips: {
      etsy: '- Align POD prompts with your listing titles.',
      coach: '- Only if you sell merch; else skip vertical.',
      local: '- Local pride designs—verify trademark rules.',
    },
    loom: {
      hook: 'POD prompts locked to a repeatable style.',
      beats: ['pod-prompt-log.csv', 'example mug prompt'],
      cta: 'POD Design Prompt Agent Kit.',
    },
  },
  'agent-10-listing-mockup-photo-brief-agent-kit': {
    w01: 'listing-image-strategy',
    tips: {
      etsy: "- 10 images: hero mockup, what's included, size chart, FAQ slide.",
      coach: '- Use for course sales pages; less critical for pure services.',
      local: '- Before/after photos for trades.',
    },
    loom: {
      hook: 'Shot lists that fix weak Etsy thumbnails.',
      beats: ['listing-shot-list.csv', 'workflow 01 open'],
      cta: 'Listing Mockup Agent Kit.',
    },
  },
  'agent-11-customer-support-reply-agent-kit': {
    w01: 'ticket-triage-classifier',
    tips: {
      etsy: '- Map Etsy messages: file issue, custom request, refund.',
      coach: '- Boundary on scope creep in support threads.',
      local: '- Scheduling and emergency lines.',
    },
    loom: {
      hook: 'Support replies that sound human—not robotic.',
      beats: ['support-macro-library.csv', 'de-escalation sample'],
      cta: 'Customer Support Agent Kit.',
    },
  },
  'agent-12-sop-operations-doc-agent-kit': {
    w01: 'process-charter',
    tips: {
      etsy: '- SOP: weekly listing publish + SEO audit.',
      coach: '- SOP: session delivery + recap within 24h.',
      local: '- SOP: job site closeout + review ask.',
    },
    loom: {
      hook: 'SOPs your team can actually follow.',
      beats: ['sop-template.md', 'playbook excerpt'],
      cta: 'SOP & Operations Agent Kit.',
    },
  },
  'agent-13-lead-magnet-opt-in-agent-kit': {
    w01: 'offer-to-magnet-angle',
    tips: {
      etsy: '- Magnet: "Listing audit checklist" → sells SEO agent.',
      coach: '- Magnet: "Client onboarding scorecard" → sells program.',
      local: '- Magnet: "Seasonal promo planner" → sells service package.',
    },
    loom: {
      hook: 'A lead magnet that pre-sells your offer.',
      beats: ['lead-magnet-one-pager-outline.md', 'landing copy workflow'],
      cta: 'Lead Magnet Agent Kit.',
    },
  },
  'agent-14-direct-mail-campaign-agent-kit': {
    w01: 'campaign-brief',
    tips: {
      etsy: '- Mail QR to free Etsy SEO checklist.',
      coach: '- Mail to local offices with workshop invite.',
      local: '- Primary vertical—use postcard HTML templates.',
    },
    loom: {
      hook: '4x6 postcards with tracking—not guesswork.',
      beats: ['postcard HTML preview', 'campaign tracker CSV'],
      cta: 'Direct Mail Campaign Agent Kit.',
    },
  },
  'agent-15-brand-voice-messaging-agent-kit': {
    w01: 'positioning-canvas',
    tips: {
      etsy: '- Voice: helpful shop owner, not guru.',
      coach: '- Voice: calm expert, ethical boundaries.',
      local: '- Voice: neighborly pro, punctual, insured.',
    },
    loom: {
      hook: 'One messaging guide for every channel.',
      beats: ['brand-voice-card.md', 'positioning sample'],
      cta: 'Brand Voice Agent Kit.',
    },
  },
  'agent-16-blog-seo-article-agent-kit': {
    w01: 'keyword-intent-map',
    tips: {
      etsy: '- Articles that feed Pinterest pins to listings.',
      coach: '- Thought leadership → application CTA.',
      local: '- "City + service" local SEO posts.',
    },
    loom: {
      hook: 'Blog posts built for search intent.',
      beats: ['article-outline-template.md', 'meta pack sample'],
      cta: 'Blog SEO Article Agent Kit.',
    },
  },
  'agent-17-business-in-a-box-orchestrator-agent-kit': {
    w01: 'sku-audit-matrix',
    tips: {
      etsy: '- Score every listing SKU + bundle.',
      coach: '- Score offers: group, 1:1, DIY kit.',
      local: '- Score services by seasonality.',
    },
    loom: {
      hook: 'Run your shop like a portfolio—not random SKUs.',
      beats: ['sku-priority-matrix.csv', '90-day plan workflow'],
      cta: 'Business-in-a-Box Orchestrator.',
    },
  },
  'agent-18-ai-ops-admin-agent-kit': {
    w01: 'daily-standup-pack',
    tips: {
      etsy: '- Daily: messages, orders, one listing fix.',
      coach: '- Daily: client boundaries + content.',
      local: '- Daily: schedule + supplier follow-ups.',
    },
    loom: {
      hook: '50 ops workflows—routed in one map.',
      beats: ['agent-routing-map.md', '50-ai-ops file mention'],
      cta: 'AI Ops & Admin Agent Kit.',
    },
  },
  'agent-19-etsy-shop-analytics-agent-kit': {
    w01: 'stats-ingest-summary',
    tips: {
      etsy: '- Primary vertical; paste Etsy Stats weekly.',
      coach: '- Use if you also sell on Etsy; else skip.',
      local: '- Optional for local Etsy side hustle.',
    },
    loom: {
      hook: 'Turn Etsy Stats into three experiments—not overwhelm.',
      beats: ['etsy-stats-weekly-paste.csv', 'diagnosis sample'],
      cta: 'Etsy Shop Analytics Agent Kit.',
    },
  },
  'agent-20-coach-consultant-professional-agent-kit': {
    w01: 'icp-offer-clarity-pass',
    tips: {
      etsy: '- Secondary; use if coach sells digital kits.',
      coach: '- Primary vertical.',
      local: '- Business coaches serving local owners.',
    },
    loom: {
      hook: 'Packages, intake, and session recaps—systemized.',
      beats: ['coaching-intake-template.md', 'package sample'],
      cta: 'Coach & Consultant Professional Agent Kit.',
    },
  },
};

const AGENT_TITLES = {
  'agent-01-etsy-listing-seo-agent-kit': 'Etsy Listing SEO',
  'agent-02-social-content-machine-agent-kit': 'Social Content Machine',
  'agent-03-marketing-planner-agent-kit': 'Marketing Planner',
  'agent-04-client-onboarding-agent-kit': 'Client Onboarding',
  'agent-05-email-sequence-agent-kit': 'Email Sequence',
  'agent-06-ad-copy-agent-kit': 'Ad Copy',
  'agent-07-sales-proposal-discovery-agent-kit': 'Sales Proposal & Discovery',
  'agent-08-copy-swipe-agent-kit': 'Copy Swipe File',
  'agent-09-pod-design-prompt-agent-kit': 'POD Design Prompt',
  'agent-10-listing-mockup-photo-brief-agent-kit': 'Listing Mockup & Photo Brief',
  'agent-11-customer-support-reply-agent-kit': 'Customer Support Reply',
  'agent-12-sop-operations-doc-agent-kit': 'SOP & Operations Doc',
  'agent-13-lead-magnet-opt-in-agent-kit': 'Lead Magnet & Opt-in',
  'agent-14-direct-mail-campaign-agent-kit': 'Direct Mail Campaign',
  'agent-15-brand-voice-messaging-agent-kit': 'Brand Voice & Messaging',
  'agent-16-blog-seo-article-agent-kit': 'Blog & SEO Article',
  'agent-17-business-in-a-box-orchestrator-agent-kit': 'Business-in-a-Box Orchestrator',
  'agent-18-ai-ops-admin-agent-kit': 'AI Ops & Admin',
  'agent-19-etsy-shop-analytics-agent-kit': 'Etsy Shop Analytics',
  'agent-20-coach-consultant-professional-agent-kit': 'Coach & Consultant Professional',
};

export function getV21Pack(slug) {
  const cfg = V21_BY_SLUG[slug];
  const title = AGENT_TITLES[slug];
  if (!cfg || !title) return null;
  const vGen = verticalMd(title, cfg.w01, cfg.tips);
  return {
    verticals: {
      'etsy-digital-seller.md': vGen('etsy'),
      'coach-consultant.md': vGen('coach'),
      'local-service-business.md': vGen('local'),
    },
    loom: `# 15-second Loom script — ${title}

**Total length:** ~15 seconds at calm pace.

| Sec | Visual | Script |
|-----|--------|--------|
| 0–3 | Face or screen | "${cfg.loom.hook}" |
| 3–10 | ${cfg.loom.beats.join(' → ')} | "Inside: scorecard, examples, and fillable templates—not a prompt dump." |
| 10–15 | Etsy shop or logo | "${cfg.loom.cta}" |

## Recording tips
- 1080p screen recording; zoom text to 125%.
- Export vertical cut for Pinterest if needed.
`,
  };
}
