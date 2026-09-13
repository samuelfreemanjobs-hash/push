/**
 * Market demand agents 11–20 (pairs with top10-definitions 1–10).
 */
import { buildKit, workflow } from './kit-builder.mjs';
import { applyDeepening } from './deep-workflows-13-19.mjs';
import { AGENT_KITS_11_20 as LEGACY } from './top20-agents-11-20.mjs';

const legacy11 = LEGACY.find((k) => k.slug === 'agent-11-customer-support-reply-agent-kit');
const legacy20 = LEGACY.find((k) => k.slug === 'agent-20-coach-consultant-professional-agent-kit');
const legacyOps = LEGACY.find((k) => k.slug === 'agent-17-operations-sop-writer-agent-kit');

function sopKit() {
  const base = { ...legacyOps };
  base.slug = 'agent-12-sop-operations-doc-agent-kit';
  base.readme = base.readme.replace(/Operations SOP Writer/g, 'SOP & Operations Doc');
  base.profile = base.profile.replace(/Operations SOP Writer/g, 'SOP & Operations Doc');
  return base;
}

function wf(slug, title, job, inputs, brief, qc) {
  return { slug, title, job, inputs, brief, qc };
}

function deepKit(opts) {
  const { specialistRole, workflowDefs, ...rest } = opts;
  const defs = applyDeepening(workflowDefs, specialistRole).map((d) => ({
    ...d,
    briefIsFull: true,
  }));
  return buildKit({ ...rest, workflowDefs: defs });
}

const leadMagnet = deepKit({
  specialistRole: 'lead magnet and email list growth specialist',
  slug: 'agent-13-lead-magnet-opt-in-agent-kit',
  name: 'Lead Magnet & Opt-in Agent',
  tagline: 'Turn expertise into a downloadable lead magnet, landing copy, and 5-email nurture.',
  audience: 'Coaches, consultants, and digital product sellers who need list growth without a full marketing team.',
  outcome: 'Published lead magnet concept, landing page copy, and nurture sequence drafts.',
  kpi: 'Opt-in rate on landing page (target 25%+ warm traffic).',
  playbookFocus: 'Ship one lead magnet per quarter: problem → asset → landing → nurture → CTA to offer.',
  setupSteps: [
    'Open `05-implementation/setup-guide.md`',
    'Run workflow **01-offer-to-magnet-angle**',
    'Complete workflows 02–05 before any design work',
  ],
  workflowDefs: [
    wf('offer-to-magnet-angle', 'Offer to magnet angle', 'Define the lead magnet promise from your core offer.', ['Core offer and price', 'Audience pain', 'Delivery format preference (PDF/checklist/templates)'], 'Propose 3 lead magnet concepts with title, promise, outline, and which offer each magnet pre-sells. Score each 1–10 on clarity and speed to create.'),
    wf('magnet-outline', 'Magnet content outline', 'Structure the downloadable asset.', ['Chosen magnet title', 'Target reader'], 'Produce H2 outline with 5–9 sections, worksheet prompts, and one actionable checklist page. Include "time to complete" estimate.'),
    wf('landing-page-copy', 'Landing page copy', 'Write opt-in page.', ['Magnet title', 'Bullets of what they get', 'Brand tone'], 'Write hero, 3 benefit bullets, what\'s inside, who it\'s for, FAQ (3), and CTA button text. Keep mobile-scannable.'),
    wf('thank-you-delivery-email', 'Thank-you delivery email', 'Email with download + next step.', ['Download link', 'One upsell (soft)'], 'Write subject + body under 200 words with link, how to use magnet, and single next step.'),
    wf('nurture-five-email-sequence', '5-email nurture', 'Post opt-in nurture toward offer.', ['Offer summary', 'Magnet topic', 'Objections'], 'Write 5 emails: deliver value, story, case pattern, objection, pitch with deadline-free CTA.'),
    wf('social-teaser-pack', 'Social teaser pack', '10 posts to promote magnet.', ['Channels', 'Magnet hook'], '10 short posts (LinkedIn/IG/Pinterest variants) driving to landing page.'),
    wf('lead-magnet-title-ab', 'Title A/B variants', 'Test 5 opt-in titles.', ['Audience', 'Outcome'], '5 title/subtitle pairs under 60 chars for ads and landing hero tests.'),
    wf('opt-in-form-microcopy', 'Form microcopy', 'Reduce form friction.', ['Fields collected'], 'Microcopy for form, privacy line, and button. Explain why email is required.'),
    wf('segment-tag-plan', 'Segment tag plan', 'Tags for CRM/email tool.', ['Tool name'], 'Tag naming convention + 3 segments based on magnet topic and follow-up paths.'),
    wf('magnet-qc-checklist', 'Magnet QC checklist', 'Pre-launch QA.', ['Draft assets list'], '15-point QA: promise match, typos, mobile PDF, link works, CAN-SPAM basics, alignment with offer.'),
  ],
});

const directMail = deepKit({
  specialistRole: 'direct mail and local response marketing specialist',
  slug: 'agent-14-direct-mail-campaign-agent-kit',
  name: 'Direct Mail Campaign Agent',
  tagline: 'Local postcard campaigns: offer, copy, checklist, and print-ready creative briefs.',
  audience: 'Local service businesses and B2B sellers using postcards to drive calls or scans.',
  outcome: 'One campaign with postcard copy, audience, tracking, and mail checklist.',
  kpi: 'Tracked leads or redemptions within 30 days of drop.',
  playbookFocus: 'One offer, one list, one CTA per campaign; measure with unique URL or code.',
  setupSteps: [
    'Open bundled postcard HTML templates in `03-templates/`',
    'Run workflow **01-campaign-brief**',
    'Complete print checklist before ordering mail',
  ],
  workflowDefs: [
    wf('campaign-brief', 'Campaign brief', 'Define single-offer mail campaign.', ['Offer', 'Geo/audience', 'Budget'], 'One-page brief: goal, offer, list, budget, timeline, success metric.'),
    wf('postcard-headline-angles', 'Headline angles', 'Write 5 postcard headlines.', ['Pain point', 'City/region'], '5 headlines ≤8 words + matching subheads for 4x6 front.'),
    wf('postcard-back-copy', 'Back copy', 'Mailing side copy + CTA.', ['CTA type: QR/call/URL'], 'Back copy with return address block notes, compliance line, and CTA.'),
    wf('list-selection-criteria', 'List selection', 'Who to mail.', ['Ideal customer'], 'Criteria for list purchase or house list; suppressions; expected quantity.'),
    wf('tracking-attribution', 'Tracking plan', 'Measure response.', ['Landing URL or code'], 'UTM/code plan, call tracking script, and spreadsheet columns for responses.'),
    wf('print-vendor-spec', 'Print spec handoff', 'Specs for printer.', ['Quantity'], '4x6 bleed, stock, coating, proof steps—buyer-ready bullet list.'),
    wf('ab-split-test', 'A/B test plan', 'Two creative variants.', ['Variable to test'], 'Hypothesis, A vs B copy, sample size rule of thumb, decision date.'),
    wf('follow-up-sequence', 'Follow-up if no response', 'Second touch ideas.', ['Days after drop'], 'Email or second mailer outline if response below threshold.'),
    wf('local-partnership-angle', 'Partnership mailer', 'Co-brand option.', ['Partner type'], 'Joint offer copy and split CTA for partner businesses.'),
    wf('campaign-retrospective', 'Campaign retrospective', 'After 30 days.', ['Results data'], 'What worked, CPL, scale/pause/kill recommendation.'),
  ],
});

const brandVoice = deepKit({
  specialistRole: 'brand messaging and positioning specialist',
  slug: 'agent-15-brand-voice-messaging-agent-kit',
  name: 'Brand Voice & Messaging Agent',
  tagline: 'Positioning, messaging pillars, and voice rules for consistent copy across channels.',
  audience: 'Founders rebranding or launching a new offer who need messaging before ads and listings.',
  outcome: 'Messaging guide: positioning statement, pillars, voice chart, and sample rewrites.',
  kpi: 'Consistent voice across 3+ channels (site, Etsy, email) within 2 weeks.',
  playbookFocus: 'Messaging before creative: who it\'s for, problem, promise, proof, personality.',
  setupSteps: ['Gather 3 competitors and 3 brands you admire', 'Run workflow **01-positioning-canvas**', 'Apply voice rules in workflow 08 to an old post'],
  workflowDefs: [
    wf('positioning-canvas', 'Positioning canvas', 'Clarify category and difference.', ['Offer', 'Audience', 'Competitors'], 'Complete positioning: for [who], [product] is the [category] that [outcome] because [reason to believe].'),
    wf('messaging-pillars', 'Messaging pillars', '3–4 pillars with proof.', ['Benefits list'], 'Pillars with headline, proof points, and example sentence each.'),
    wf('voice-dimensions', 'Voice dimensions', 'Voice chart.', ['3 adjectives', '3 anti-adjectives'], 'We are / We are not table across 4 dimensions (tone, vocabulary, humor, authority).'),
    wf('tagline-options', 'Tagline options', '10 taglines.', ['Positioning'], '10 taglines ≤8 words with rationale each.'),
    wf('boilerplate-about', 'About boilerplate', 'Short and long about.', ['Founder story bullets'], '50-word and 150-word about [YOUR BRAND].'),
    wf('value-prop-hierarchy', 'Value prop hierarchy', 'Lead message per channel.', ['Channels'], 'Primary message for Etsy, email, LinkedIn, ads—same core, different lead.'),
    wf('audience-messaging-matrix', 'Audience matrix', 'Messages per segment.', ['2–3 segments'], 'Table: segment | pain | promise | proof | CTA.'),
    wf('voice-rewrite-sample', 'Voice rewrite', 'Rewrite generic copy.', ['Paste generic paragraph'], 'Rewrite in brand voice + annotate 3 changes.'),
    wf('forbidden-claims-list', 'Claims guardrails', 'Words to avoid.', ['Industry'], 'List banned phrases and compliant alternatives for your niche.'),
    wf('messaging-one-pager', 'Messaging one-pager', 'Single PDF outline.', ['All above'], 'Combine into 1-page messaging doc structure for designers/VA.'),
  ],
});

const blogSeo = deepKit({
  specialistRole: 'blog and SEO content specialist',
  slug: 'agent-16-blog-seo-article-agent-kit',
  name: 'Blog & SEO Article Agent',
  tagline: 'Keyword-led articles: outline, draft sections, meta, and internal links for organic traffic.',
  audience: 'Sellers and SMBs driving Etsy/blog traffic from Pinterest, Google, and newsletters.',
  outcome: 'Publish-ready article package with SEO metadata and FAQ schema suggestions.',
  kpi: 'Organic clicks/impressions growth on target URL (90-day window).',
  playbookFocus: 'One target keyword per article; intent-matched structure; human edit before publish.',
  setupSteps: ['Pick one keyword from Etsy or Google', 'Run **01-keyword-intent-map**', 'Draft with **04-section-drafts**'],
  workflowDefs: [
    wf('keyword-intent-map', 'Keyword intent map', 'Classify intent and angle.', ['Target keyword', 'Audience'], 'Intent type, SERP-style outline competitors use, angle differentiation for [YOUR BRAND].'),
    wf('article-outline', 'Article outline', 'H2/H3 structure.', ['Keyword', 'Word count target'], 'Outline with FAQ section and suggested internal links.'),
    wf('title-meta-pack', 'Title & meta pack', 'SEO title and description.', ['Keyword'], '3 title options ≤60 chars, meta ≤155 chars, slug suggestion.'),
    wf('section-drafts', 'Section drafts', 'Draft body from outline.', ['Outline'], 'Draft each H2 in scannable paragraphs with bullets where needed.'),
    wf('intro-hook', 'Intro hook', 'First 120 words.', ['Main promise'], 'Hook + problem + promise + preview bullets.'),
    wf('conclusion-cta', 'Conclusion CTA', 'Close with action.', ['Primary CTA'], 'Summary + single CTA aligned to lead magnet or listing.'),
    wf('faq-block', 'FAQ block', '5–8 FAQs.', ['Common objections'], 'Q&A for FAQ schema; plain language.'),
    wf('pinterest-pin-copy', 'Pinterest pin copy', 'Pin for article.', ['Article title'], 'Pin title, description, 5 hashtags for traffic to post.'),
    wf('newsletter-teaser', 'Newsletter teaser', 'Email to promote post.', ['Audience'], 'Subject + 100-word teaser linking to article.'),
    wf('content-refresh-audit', 'Content refresh audit', 'Update old post.', ['URL or paste'], 'What to add/update for freshness and keyword coverage.'),
  ],
});

const bizBox = deepKit({
  specialistRole: 'multi-SKU digital product operator',
  slug: 'agent-17-business-in-a-box-orchestrator-agent-kit',
  name: 'Business-in-a-Box Orchestrator Agent',
  tagline: 'Launch and run multiple digital SKUs as a system: priorities, bundles, and weekly operator cadence.',
  audience: 'Etsy digital sellers with 3+ products who need an operator rhythm, not more random listings.',
  outcome: '90-day launch plan, SKU priority stack, and weekly orchestration checklist.',
  kpi: 'Revenue from top 3 SKUs as % of shop revenue ≥60%.',
  playbookFocus: 'Orchestrate onboarding, marketing, and listing agents—not replace them.',
  setupSteps: [
    'Inventory your SKUs (listings + zip products)',
    'Run **01-sku-audit-matrix**',
    'Schedule weekly orchestrator review (workflow 10)',
  ],
  workflowDefs: [
    wf('sku-audit-matrix', 'SKU audit matrix', 'Score each product.', ['SKU list with prices'], 'Table: SKU | demand | effort | margin | score | action (scale/fix/kill).'),
    wf('bundle-architecture', 'Bundle architecture', 'Design bundles.', ['Top SKUs'], '3 bundle concepts with anchor SKU, price ladder, and Etsy title angle.'),
    wf('ninety-day-launch-plan', '90-day launch plan', 'Quarterly roadmap.', ['Goal revenue'], 'Week-by-week: launch, ads, content, email, partnerships.'),
    wf('weekly-operator-agenda', 'Weekly operator agenda', '60-min meeting with yourself.', ['Last week metrics'], 'Agenda: metrics, blockers, one marketing bet, one listing fix.'),
    wf('agent-routing-map', 'Agent routing map', 'Which agent for which task.', ['Task backlog'], 'Map tasks to agent kits 01–20 in your library.'),
    wf('pricing-ladder-refresh', 'Pricing ladder', 'Review prices.', ['Competitors'], 'Good/better/best pricing with psychological anchors.'),
    wf('customer-journey-map', 'Customer journey', 'Path from discovery to repeat.', ['Channels'], 'Journey stages and which SKU serves each.'),
    wf('partnership-outreach', 'Partnership outreach', '1 collaboration pitch.', ['Complementary seller'], 'Short partnership email and offer structure.'),
    wf('metrics-dashboard-narrative', 'Metrics narrative', 'Explain the numbers.', ['Paste stats'], 'Plain-English summary + 3 levers for next week.'),
    wf('quarterly-retrospective', 'Quarterly retrospective', 'End of quarter review.', ['Revenue by SKU'], 'Keep/stop/start list and next quarter SKU bets.'),
  ],
});

const aiOps = deepKit({
  specialistRole: 'AI-assisted business operations specialist',
  slug: 'agent-18-ai-ops-admin-agent-kit',
  name: 'AI Ops & Admin Agent',
  tagline: 'Daily/weekly business operations: inbox triage, scheduling narratives, finance summaries, and admin briefs.',
  audience: 'Solopreneurs drowning in admin who already use AI but lack structured ops workflows.',
  outcome: 'Repeatable admin workflows plus bundled 50-workflow library (see `03-templates/`).',
  kpi: 'Admin time reduced measurably (track hours/week).',
  playbookFocus: 'Use short agent runs for repeatable ops tasks; human approves anything external.',
  setupSteps: [
    'Open `03-templates/50-ai-ops-agent-workflows.md`',
    'Run workflow **01-daily-standup-pack** each morning',
    'Use workflow **05-inbox-triage** for support overflow',
  ],
  workflowDefs: [
    wf('daily-standup-pack', 'Daily standup pack', 'Plan the day.', ['Calendar', 'Top 3 goals'], 'Today\'s priorities, time blocks, and what to defer.'),
    wf('inbox-triage', 'Inbox triage', 'Sort messages.', ['Paste subject lines or summaries'], 'Categorize: reply today / delegate / archive; draft quick replies for urgent.'),
    wf('meeting-prep-brief', 'Meeting prep', 'Prep for call.', ['Attendee, purpose'], 'Agenda, questions, desired outcome, follow-up email draft.'),
    wf('weekly-finance-snapshot', 'Finance snapshot', 'Explain cash movement.', ['Numbers paste'], 'Narrative for owner: in, out, runway notes, one action.'),
    wf('contractor-task-brief', 'Contractor brief', 'Delegate task.', ['Deliverable'], 'Brief with context, acceptance criteria, deadline, examples.'),
    wf('sop-from-screen-recording', 'SOP from notes', 'Turn notes into SOP.', ['Messy notes'], 'Step-by-step SOP with roles and tools.'),
    wf('tool-stack-audit', 'Tool stack audit', 'Simplify tools.', ['Current tools'], 'Overlap, gaps, consolidate recommendation.'),
    wf('quarterly-goal-breakdown', 'Goal breakdown', 'OKR to tasks.', ['Quarter goal'], 'Monthly milestones and weekly habits.'),
    wf('travel-reset-plan', 'Away mode plan', 'Before vacation.', ['Dates'], 'Auto-replies, coverage, client expectations.'),
    wf('end-of-week-review', 'End of week review', 'Friday wrap.', ['Wins and misses'], '5 bullets + next week focus; optional team message.'),
  ],
});

const shopAnalytics = deepKit({
  specialistRole: 'Etsy shop analytics and growth specialist',
  slug: 'agent-19-etsy-shop-analytics-agent-kit',
  name: 'Etsy Shop Analytics Agent',
  tagline: 'Turn Etsy Stats into decisions: weekly diagnosis, listing fixes, and experiment backlog.',
  audience: 'Etsy digital sellers who have traffic but unclear priorities.',
  outcome: 'Weekly shop health report with prioritized experiments.',
  kpi: 'Conversion rate or revenue/week trend up after 4 weekly cycles.',
  playbookFocus: 'Metrics → diagnosis → one listing fix + one traffic experiment per week.',
  setupSteps: ['Export last 28 days Etsy Stats', 'Run **01-stats-ingest-summary**', 'Apply **08-experiment-backlog**'],
  workflowDefs: [
    wf('stats-ingest-summary', 'Stats ingest summary', 'Summarize metrics.', ['Paste Stats or CSV notes'], 'Traffic, conversion, favorites, orders, top listings, search terms if available.'),
    wf('listing-performance-rank', 'Listing rank', 'Stack rank listings.', ['Listing names + metrics'], 'Table sorted by revenue potential with "fix" hypothesis each.'),
    wf('ctr-diagnosis', 'CTR diagnosis', 'Low click listings.', ['Impressions/CTR data'], 'Why CTR may be low: thumbnail, title, price signal; 3 fixes each.'),
    wf('conversion-diagnosis', 'Conversion diagnosis', 'Low convert listings.', ['Views/orders'], 'Description, reviews, offer clarity, friction; 3 fixes.'),
    wf('search-term-opportunities', 'Search opportunities', 'Terms to target.', ['Search terms list'], 'Terms to add to tags/titles vs terms to ignore.'),
    wf('seasonal-prep', 'Seasonal prep', 'Next 6 weeks seasonality.', ['Category'], 'Seasonal tag/title tweaks and SKU ideas.'),
    wf('pricing-sensitivity-check', 'Pricing check', 'Price vs market.', ['Prices', 'Competitors'], 'Hold/raise/lower with bundle suggestion.'),
    wf('experiment-backlog', 'Experiment backlog', 'Prioritized tests.', ['Current issues'], '5 experiments: change, metric, duration, success threshold.'),
    wf('review-response-drafts', 'Review response drafts', 'Reply to recent reviews.', ['Paste reviews'], 'Thank-you and problem-solving replies; escalate negatives.'),
    wf('monthly-exec-summary', 'Monthly exec summary', 'Owner one-pager.', ['Month data'], 'Headline, wins, losses, next month bets.'),
  ],
});

export const AGENT_KITS_11_20 = [
  legacy11,
  sopKit(),
  leadMagnet,
  directMail,
  brandVoice,
  blogSeo,
  bizBox,
  aiOps,
  shopAnalytics,
  legacy20,
];
