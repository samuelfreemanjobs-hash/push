/**
 * Top 10 AI Agent Kit definitions (kits 01–03, 05–10; kit 04 copied separately).
 * Consumed by scripts/generate-top10-agents.mjs
 */

function workflow(slug, title, job, inputs, brief, qc) {
  return { slug, title, job, inputs, brief, qc };
}

export const AGENT_KITS = [
  {
    slug: 'agent-01-etsy-listing-seo-agent-kit',
    readme: `# Etsy Listing SEO Agent Kit — [YOUR BRAND]

Turn one product idea into Etsy-ready listings that rank and convert.

## What you get
- **AGENT-PROFILE.md** — role, inputs, outputs, KPIs
- **01-playbook/** — weekly SEO operating rhythm
- **02-workflows/** — 10 copy-paste agent briefs (research → publish)
- **05-implementation/** — 15-minute setup

## Who this is for
Digital product sellers, POD shops, and template creators who want consistent Etsy search traffic without guessing tags and titles.

## Quick start
1. Open \`05-implementation/setup-guide.md\`
2. Run workflow **01-keyword-research-matrix** for your next SKU
3. Ship title, 13 tags, and description from workflows 02–04
4. Re-run **08-listing-audit-scorecard** before every publish

Replace every \`[YOUR BRAND]\` placeholder before you run workflows.
`,
    profile: `# Agent profile — Etsy Listing SEO

**Role:** Etsy search strategist for [YOUR BRAND].

**Outcome:** Each listing has a researched keyword spine, compliant title, 13 high-intent tags, and a description that converts browsers into buyers.

**Inputs:** Product type, niche, price point, competitor shop URLs, existing listing text (if any).

**Outputs:** Keyword matrix, optimized title variants, tag sets, SEO description, alt-text pack, audit scorecard.

**KPIs:** Impressions and click-through rate (Etsy Stats), conversion rate, favorites per listing, rank for 3 target phrases (manual spot-check weekly).

**Guardrails:** No trademark infringement, no misleading claims, follow Etsy house rules for digital goods and repetition across listings.
`,
    playbook: `# Etsy Listing SEO — Playbook (SOP)

## Purpose
Run a repeatable weekly loop so every [YOUR BRAND] listing earns qualified Etsy search impressions and converts with buyer-clear copy.

## Roles
- **Owner:** picks SKUs to optimize, approves final title/tags, publishes.
- **SEO agent (AI):** research, drafts, audits via workflows in \`02-workflows/\`.

## Weekly rhythm (≈90 minutes)
**Monday — Pick targets (15 min):** Choose 1–3 listings (new or underperforming). Export Etsy Stats for last 28 days: impressions, CTR, conversion, favorites. Note top query terms if visible.

**Tuesday — Research (25 min):** Run workflow 01 for each SKU. Save primary phrase, 3 secondary phrases, and 5 long-tail variants. Record competitor URLs you studied.

**Wednesday — Draft assets (30 min):** Run workflows 02–04. Produce 2 title options, one 13-tag set, one description. Pick hero keywords for the first 40 characters of title.

**Thursday — Multimedia SEO (15 min):** Run workflows 09–10. Alt text for every image; align shop section and attributes with keyword spine.

**Friday — Audit & publish (15 min):** Run workflow 08. Score must be ≥85/100 before publish. Log changes in your tracker (Notion/Sheet).

## New listing launch checklist
- Keyword matrix approved
- Title ≤140 characters, front-loaded primary phrase
- Exactly 13 tags, mix of broad + long-tail, no duplicates across shop
- Description: hook, what's included, how to use, license, FAQ
- 5+ images with alt text
- Category/attributes set

## Monthly refresh
Run workflow 07 for top 5 listings and seasonal SKUs. Update tags when Etsy trends shift (holidays, back-to-school, Q4 gifting).

## Quality bar
Reject keyword stuffing, duplicate tags shop-wide, or titles that read like comma-separated tags. Every change must be legible to a human buyer first, algorithm second.

## Escalation
If a listing violates Etsy policies or receives IP notice, pause optimization and fix compliance before resuming SEO work.
`,
    setup: `# 15-minute setup — Etsy Listing SEO Agent

## Minutes 0–3: Brand context
Create a note titled **[YOUR BRAND] Etsy SEO context** with: shop URL, product categories, average price, 3 competitor shops, tone (friendly/expert/minimal), and words you never use.

## Minutes 3–6: Tooling
Pin your AI tool with **AGENT-PROFILE.md** in project instructions. Create a folder per listing: \`sku-name/research.md\`, \`drafts.md\`, \`published-snapshot.md\`.

## Minutes 6–10: Baseline metrics
Open Etsy Stats → Search terms / Listing stats. Copy last 28 days for your worst CTR listing into the SKU folder. This becomes the “before” for workflow 08.

## Minutes 10–13: First workflow
Open \`02-workflows/01-keyword-research-matrix.md\`. Paste inputs for one real product. Save output to \`research.md\`.

## Minutes 13–15: Schedule the loop
Block 90 minutes weekly on your calendar labeled “Etsy SEO loop.” Add checklist: research → draft → audit → publish.

You are ready. Run workflow 02 on the same SKU next.
`,
    workflows: [
      workflow(
        'keyword-research-matrix',
        'Keyword research matrix',
        'Build a prioritized Etsy search keyword plan for one listing.',
        [
          'Product name and one-sentence benefit',
          'Niche/audience (e.g., wedding planners, Cricut users)',
          'Price and product type (digital download / POD / physical)',
          '3 competitor listing URLs or titles',
          'Season or event (optional)',
        ],
        `You are an Etsy SEO researcher for [YOUR BRAND]. Produce a keyword matrix for ONE listing.

Study the product, niche, price, and competitors. Infer buyer intent (gift, business use, hobby, urgent need). Map phrases shoppers actually type into Etsy search—not generic Google SEO terms unless they clearly apply to Etsy.

Deliver:
1) **Primary phrase** (2–4 words) — highest intent + fit for this product.
2) **Secondary phrases** (3) — related, non-redundant.
3) **Long-tail phrases** (5) — specific, lower competition.
4) **Negative keywords** (5) — searches that would attract wrong clicks.
5) **Tag candidates** (20) — raw material for later 13-tag curation.
6) **Title angle** — one sentence describing how to front-load the primary phrase naturally.

Present as a markdown table: Phrase | Intent (transactional/informational) | Placement (title/tag/desc) | Notes.

Be specific to digital/POD if applicable. Do not invent trademarked brand names. Flag any phrase that risks policy issues (medical claims, guaranteed income).`,
        [
          'Primary phrase appears in title recommendation',
          'No duplicate phrases across tag candidates',
          'At least 3 long-tail phrases under 20 characters',
          'Competitor insights referenced explicitly',
        ],
      ),
      workflow(
        'title-optimizer',
        'Listing title optimizer',
        'Generate Etsy-compliant titles that front-load high-intent keywords.',
        [
          'Approved keyword matrix (or primary + 3 secondary phrases)',
          'Product format (PDF, Canva, PNG bundle, etc.)',
          'Key deliverable count (e.g., 12 templates)',
          'Words to avoid',
        ],
        `You are an Etsy listing title specialist for [YOUR BRAND]. Write **3 title variants** for one product.

Rules: max **140 characters** each; readable to humans; primary keyword in the first **40 characters** when possible; include format and quantity if it differentiates; no ALL CAPS; no keyword spam or repeated commas; no unverifiable superlatives (#1, best ever).

For each variant provide: Title | Char count | Primary phrase placement | Why this wins (1 sentence).

End with a **recommended pick** and a **mobile preview** (first 55 characters shown). If digital, clarify instant download only when accurate.`,
        [
          'All variants ≤140 characters',
          'Primary phrase in first half of recommended title',
          'No trademark or Etsy-prohibited terms',
          'Readable aloud in one breath',
        ],
      ),
      workflow(
        'tag-set-generator',
        '13-tag set generator',
        'Curate exactly 13 Etsy tags from research without shop-wide duplication.',
        [
          'Keyword matrix or tag candidates',
          'List of tags already used on other active listings (paste)',
          'Primary category (Etsy taxonomy)',
        ],
        `You are an Etsy tags strategist for [YOUR BRAND]. Create **exactly 13 tags** for one listing.

Each tag max **20 characters**. Mix: 3 broad niche tags, 5 mid-intent, 5 long-tail. No plurals/singular duplicates, no repeating the same word stem across tags, no cross-listing duplicates from the provided "already used" list.

Output:
- Tag list (numbered 1–13)
- For each tag: role (broad/mid/long) + buyer intent
- **Swap suggestions** if any tag is weak
- One-line warning if tags are too similar to title (that's OK in moderation)

Optimize for Etsy search 2024+ practice: phrases buyers use, not single generic words like "gift" alone.`,
        [
          'Exactly 13 tags, each ≤20 characters',
          'No duplicates with other listings list',
          'Includes at least 4 multi-word tags where allowed',
        ],
      ),
      workflow(
        'description-seo-writer',
        'SEO description writer',
        'Write a scannable listing description that ranks and converts.',
        [
          'Final or draft title',
          '13 tags',
          "Bullet list of what's included",
          'License/refund policy snippets',
          'Brand voice adjectives (3)',
        ],
        `You are an Etsy conversion copywriter for [YOUR BRAND]. Write a full **listing description** in markdown.

Structure:
1) **Hook** (2 sentences) — outcome for buyer, include primary phrase naturally.
2) **What's included** — bullet list with quantities/formats/sizes.
3) **How to use** — numbered steps for digital/POD.
4) **Who it's for** — 3 bullets.
5) **FAQ** — 4 Q&As (commercial use, software needed, refunds, customization).
6) **SEO footer** — 2 sentences weaving secondary phrases without stuffing.

Tone: match voice adjectives. Short paragraphs. No HTML required. Add [CUSTOMIZE: …] placeholders only where seller must insert facts.`,
        [
          'Primary phrase in first 100 words',
          "What's included matches seller bullet input",
          'FAQ addresses digital download expectations',
          'No guaranteed results or income claims',
        ],
      ),
      workflow(
        'category-taxonomy-picker',
        'Category & attribute picker',
        'Recommend Etsy category path and attributes aligned to keywords.',
        [
          'Product type and materials/format',
          'Keyword matrix',
          'Current category (if any)',
        ],
        `You are an Etsy taxonomy advisor for [YOUR BRAND]. Recommend the best **category path** and **attributes** for one listing.

List 2 plausible Etsy category paths (breadcrumb style). Pick one winner with rationale tied to buyer intent and competition. Suggest required attributes (occasion, style, file type, etc.) and optional ones that improve filter visibility.

If digital: note file type, editable/non-editable, software compatibility. Flag mismatch risks (wrong category = lower quality score).

Output sections: Recommended path | Alternative | Attributes table | Keyword alignment notes.`,
        [
          'Recommendation matches digital vs physical reality',
          'Attributes are consistent with description',
          'Rationale cites buyer intent',
        ],
      ),
      workflow(
        'competitor-gap-analysis',
        'Competitor gap analysis',
        'Find positioning gaps versus 3 competitor listings.',
        [
          'Your draft title/tags/description',
          '3 competitor listing URLs or pasted copy',
          'Your unique differentiator (1–2 sentences)',
        ],
        `You are a competitive intelligence analyst for Etsy shops. Compare [YOUR BRAND]'s draft listing against three competitors.

For each competitor extract: title pattern, apparent keyword focus, promise, proof elements (reviews, bestseller), weaknesses (vague deliverables, missing FAQs).

Then produce:
- **Gap table** — what competitors say vs what we should say
- **5 positioning hooks** only we can claim (truthful)
- **3 tag/title tweaks** to exploit gaps
- **Risk** — where we are behind and need better creative or offer

Stay factual; do not copy phrasing verbatim.`,
        [
          'All 3 competitors addressed',
          'Differentiator appears in recommendations',
          'No plagiarized sentences',
        ],
      ),
      workflow(
        'seasonal-keyword-refresh',
        'Seasonal keyword refresh',
        'Refresh tags and title hooks for seasonal demand windows.',
        [
          'Listing URL or current title/tags',
          "Target season/event (e.g., Mother's Day, Q4)",
          'Lead time in weeks',
        ],
        `You are an Etsy seasonal SEO planner for [YOUR BRAND]. Refresh one listing for an upcoming season.

Propose **timeline**: when to publish changes (weeks before peak). Suggest **seasonal title variant** (≤140 chars) and **5 seasonal tag swaps** (still 13 total—show full new set). Add **2 description inserts** (banner sentence + FAQ).

Include **revert plan** after season ends. Note Etsy policy on misleading seasonal claims if product is evergreen.`,
        [
          'Full 13-tag set shown after swaps',
          'Timeline realistic for lead time input',
          'Revert plan included',
        ],
      ),
      workflow(
        'listing-audit-scorecard',
        'Listing audit scorecard',
        'Score a listing draft before publish and fix blockers.',
        [
          'Full draft: title, tags, description, image count',
          'Keyword matrix',
          'Etsy policy notes for product type',
        ],
        `You are an Etsy listing QA lead for [YOUR BRAND]. Audit one listing draft.

Score 0–100 across: Title (20), Tags (20), Description (25), Keyword alignment (15), Policy/compliance (10), Conversion clarity (10). Show sub-scores and **total**.

List **blockers** (must fix before publish) and **nice-to-haves**. Provide rewritten snippets only for failing sections.

End with **Go / No-go** and top 3 actions for +10 points.`,
        [
          'Total score calculated',
          'Blockers are specific and actionable',
          'Policy section mentions digital/license if applicable',
        ],
      ),
      workflow(
        'alt-text-image-seo',
        'Image alt-text pack',
        'Write Etsy image alt text that aids discovery and accessibility.',
        [
          'Listing title + primary phrase',
          'Short description of each image (1–5)',
          'Product type',
        ],
        `You are an accessibility-minded Etsy SEO writer for [YOUR BRAND]. Write **alt text** for each listing image.

Rules: describe what is visible; include primary phrase in **image 1 only** naturally; max ~125 characters each; no keyword lists; no "image of" filler.

Deliver numbered alt lines matching image order + note which image should be lifestyle vs flat mockup for CTR.`,
        [
          'One alt per image provided',
          'Primary phrase only once',
          'Each alt describes visible content',
        ],
      ),
      workflow(
        'shop-section-organizer',
        'Shop section organizer',
        'Organize shop sections for browse SEO and buyer clarity.',
        [
          'List of active products (names + 1 line)',
          'Top 3 revenue SKUs',
          'Keyword themes',
        ],
        `You are an Etsy shop merchandiser for [YOUR BRAND]. Propose a **shop section** structure (8–12 sections max).

Group products logically for buyers, not only SEO. For each section: name (≤24 chars), which SKUs belong, primary keyword theme, cross-link sentence for shop announcement.

Include **migration steps** from current sections without breaking URLs unnecessarily.`,
        [
          'Every listed product assigned to one section',
          'Section names buyer-friendly',
          'Top SKUs featured in high-traffic sections',
        ],
      ),
    ],
  },
  {
    slug: "agent-02-social-content-machine-agent-kit",
    readme: "# Social Content Machine Agent Kit — [YOUR BRAND]\n\nPlan, write, and batch 30 days of social content for Etsy-adjacent brands.\n\n## What you get\n- 10 workflows: themes → scripts → captions → batch schedule\n- Playbook for weekly content batching\n- 15-minute setup guide\n\n## Best for\nSellers driving traffic from Instagram, Pinterest, TikTok, and Facebook to Etsy listings or lead magnets.\n\nReplace `[YOUR BRAND]` before running workflows.",
    profile: "# Agent profile — Social Content Machine\n\n**Role:** Social content strategist for [YOUR BRAND].\n\n**Outcome:** 30 days of platform-ready posts with hooks, captions, CTAs, and a realistic publishing calendar.\n\n**Inputs:** Offer, audience, brand voice, primary CTA (Etsy shop, email list, freebie), active platforms.\n\n**Outputs:** Content calendar, post drafts, hashtag sets, repurposing maps.\n\n**KPIs:** Saves/shares, link clicks, profile visits, attributed Etsy favorites (UTM where possible).",
    playbook: "# Social Content Machine — Playbook (SOP)\n\n## Purpose\nRun a predictable weekly batch so [YOUR BRAND] publishes daily without last-minute scrambling.\n\n## Weekly batch (2 hours)\n**Plan (20 min):** Pick weekly theme tied to one product or pain point. Run workflow 01.\n\n**Create (60 min):** Run workflows 02–05 for your top 2 platforms. Aim for 5–7 posts per platform.\n\n**Engage (20 min):** Queue workflow 06 questions and 08 hashtag sets. Schedule via Meta Business Suite, Later, or Pinterest native.\n\n**Repurpose (20 min):** Workflow 09 turns one long asset into micro-posts.\n\n## 30-day cycle\nWeek 1: education | Week 2: proof/stories | Week 3: product spotlight | Week 4: community/UGC.\n\n## Quality bar\nEvery post needs one clear CTA, one visual note, and brand voice check. No engagement bait that violates platform rules.\n\n## Metrics review (monthly)\nTop 5 posts by saves/clicks → clone format next month. Kill formats with <1% engagement twice in a row.\n\n## Roles\nOwner approves calendar; AI drafts; human films/photographs templates noted in each brief.",
    setup: "# 15-minute setup — Social Content Machine\n\n**0–4 min:** List platforms, posting frequency goal, and 3 content pillars for [YOUR BRAND].\n\n**4–8 min:** Paste AGENT-PROFILE.md into AI project instructions. Link Etsy shop URL and lead magnet URL.\n\n**8–12 min:** Run workflow `01-thirty-day-theme-map` with one hero product.\n\n**12–15 min:** Block recurring 2-hour \"content batch\" on calendar. Create folders: `/content/ideas`, `/content/drafts`, `/content/posted`.\n\nYou are ready to run workflow 02 for week 1 posts.",
    workflows: [
      workflow(
        "thirty-day-theme-map",
        "30-day theme map",
        "Map 30 days of content themes tied to offers and seasons.",
        [
          "Hero product or collection",
          "Audience pain points (3)",
          "Brand voice (3 adjectives)",
          "Key dates/seasons",
          "Primary CTA"
        ],
        "You are a social strategist for [YOUR BRAND]. Build a **30-day content theme map** (one row per day).\n\nColumns: Date | Platform priority | Pillar (educate/inspire/sell/community) | Theme sentence | Hook idea | CTA | Asset type (carousel/reel/static/story). Align weeks: W1 educate, W2 proof, W3 product, W4 community. Tie at least 8 days directly to Etsy listings or freebies.\n\nEnd with **batching notes**: which days to film together and which need Canva templates.",
        [
          "30 distinct days",
          "Each day has hook + CTA",
          "At least 8 sell-focused days",
          "Seasonal dates used when provided"
        ],
      ),
      workflow(
        "carousel-caption-writer",
        "Carousel caption writer",
        "Write slide-by-slide carousel copy with strong save-worthy value.",
        [
          "Topic from theme map",
          "Audience level (beginner/advanced)",
          "Number of slides (5–10)",
          "Brand voice",
          "CTA"
        ],
        "You are an Instagram carousel copywriter for [YOUR BRAND]. Write **slide-by-slide text** for a carousel.\n\nSlide 1: hook (≤12 words). Slides 2–n: one idea each, max 2 short sentences. Final slide: CTA with reason to click. Add **design notes** per slide (icon, screenshot, product mockup).\n\nAlso deliver: caption body (150–220 words), 3 hook variants for first slide, and 5 comment prompts.",
        [
          "Every slide has text",
          "CTA on final slide",
          "Caption matches carousel promise",
          "No false income claims"
        ],
      ),
      workflow(
        "reel-script-hooks",
        "Reel / short script",
        "Script 15–45s vertical video with pattern interrupt hook.",
        [
          "Topic",
          "Target length (seconds)",
          "Filming style (talking head/b-roll/screen)",
          "Offer mention rules",
          "Brand voice"
        ],
        "You are a short-form video scriptwriter for [YOUR BRAND]. Write a **shot-by-shot script** with timestamps.\n\nInclude: spoken lines, on-screen text, b-roll suggestions, pattern interrupt in first 2 seconds, soft product mention mid-video, CTA in last 3 seconds. Add **caption** and **cover text** options.\n\nMark `[B-ROLL]` and `[ON-SCREEN]` clearly. Keep sentences speakable.",
        [
          "Hook in first 2 seconds noted",
          "Total length matches target",
          "One clear CTA",
          "Filming notes present"
        ],
      ),
      workflow(
        "pinterest-pin-batch",
        "Pinterest pin batch",
        "Create 10 pin title/description pairs for Etsy traffic.",
        [
          "Listing or blog URL target",
          "Primary keywords (5)",
          "Pin templates available (vertical sizes)",
          "Season"
        ],
        "You are a Pinterest SEO specialist for [YOUR BRAND]. Create **10 pin concepts** for one destination URL.\n\nEach pin: Title (≤100 chars) | Description (2–3 sentences, keyword natural) | Text overlay suggestion | Board name | Alt text.\n\nMix educational and product pins. Note which pins are best for Idea Pins vs static.",
        [
          "10 unique pins",
          "Keywords distributed naturally",
          "Each pin has board suggestion",
          "Destination URL referenced"
        ],
      ),
      workflow(
        "tiktok-script-30s",
        "TikTok 30s script",
        "Write TikTok script optimized for Etsy/digital product niches.",
        [
          "Niche audience",
          "Problem/solution",
          "Product tease level (soft/hard)",
          "Trend audio optional",
          "Voice"
        ],
        "You are a TikTok scriptwriter for [YOUR BRAND]. Write a **30-second script** with scene beats every 3–5 seconds.\n\nUse conversational tone, one myth-bust or mini-tutorial, subtle Etsy link verbal CTA ('link in bio'). Suggest 2 trending sound categories (no copyrighted lyrics).\n\nInclude hashtag strategy: 3 niche + 2 broad.",
        [
          "≤30s when read aloud",
          "Problem stated by second 5",
          "CTA to bio/link",
          "Hashtags listed"
        ],
      ),
      workflow(
        "engagement-question-bank",
        "Engagement question bank",
        "Generate 20 questions/stories prompts to boost comments.",
        [
          "Audience",
          "Content pillars",
          "Platforms",
          "Topics to avoid"
        ],
        "You are a community manager for [YOUR BRAND]. Create **20 engagement prompts**: 10 questions, 5 'this or that', 5 fill-in-the-blank.\n\nTag each with best platform and pillar. No polarizing politics or medical claims. Include 3 prompts that naturally surface customer stories for future UGC.",
        [
          "20 prompts total",
          "Mix of formats",
          "Platform tags present",
          "Avoid list respected"
        ],
      ),
      workflow(
        "ugc-brief-for-creators",
        "UGC creator brief",
        "Brief micro-creators to film authentic UGC for ads or organic.",
        [
          "Product description",
          "Must-show shots",
          "Do/don't list",
          "Compensation type",
          "Deadline"
        ],
        "You are a UGC director for [YOUR BRAND]. Write a **creator brief** one-pager.\n\nSections: Goal | Audience | Script outline (optional) | Shot list (6 shots) | Lighting/audio | Captions required | Usage rights | Submission checklist.\n\nTone: friendly, specific, leave room for creator personality.",
        [
          "Shot list has 6 items",
          "Rights/deadline mentioned",
          "Do/don't included",
          "Submission checklist present"
        ],
      ),
      workflow(
        "hashtag-set-by-platform",
        "Hashtag sets by platform",
        "Build 3 hashtag stacks per platform for rotation.",
        [
          "Niche keywords",
          "Platforms in use",
          "Banned tags (if any)",
          "Location (optional)"
        ],
        "You are a social SEO assistant for [YOUR BRAND]. Create **3 hashtag sets per platform** (Instagram, TikTok, Pinterest as applicable).\n\nEach set: 8–15 tags, mix reach and niche, no banned tags, no irrelevant mega tags only. Explain when to rotate sets.\n\nDeliver as tables: Set A/B/C per platform.",
        [
          "3 sets per requested platform",
          "No duplicates within a set",
          "Rotation guidance included"
        ],
      ),
      workflow(
        "repurpose-blog-to-social",
        "Repurpose long-form to social",
        "Turn one blog/newsletter into 8 micro-posts.",
        [
          "Long-form text or outline",
          "Primary CTA",
          "Platforms to target",
          "Brand voice"
        ],
        "You are a content repurposing editor for [YOUR BRAND]. Split the long-form into **8 social posts**.\n\nMap: 2 carousels, 2 reels scripts, 2 single-image captions, 2 story sequences. Each cites which section of source it came from. Maintain factual accuracy—do not invent stats.\n\nAdd a **master schedule** suggesting publish order for maximum novelty.",
        [
          "8 posts created",
          "Formats mix matches spec",
          "Source sections referenced",
          "CTA on each post"
        ],
      ),
      workflow(
        "weekly-batch-schedule",
        "Weekly batch schedule",
        "Turn drafts into a realistic weekly posting schedule.",
        [
          "List of drafted posts (titles)",
          "Platforms",
          "Best posting times (if known)",
          "Team capacity hours"
        ],
        "You are a social operations planner for [YOUR BRAND]. Build a **7-day schedule** with time slots.\n\nAssign each draft to day/time/platform. Include prep blocks (design, film, approve). Flag overload days and suggest swaps.\n\nOutput: table + checklist for Sunday night prep.",
        [
          "All drafts scheduled",
          "Prep blocks included",
          "Overload flagged",
          "Checklist at end"
        ],
      ),
    ],
  },
  {
    slug: "agent-03-marketing-planner-agent-kit",
    readme: "# Marketing Planner Agent Kit — [YOUR BRAND]\n\nWeekly marketing command center: campaigns, channels, owners, and metrics in one rhythm.\n\n## What you get\n- 10 planning workflows (calendar, launches, retros)\n- **03-templates/Marketing-Command-Center.xlsx** — copy budget, campaigns, and KPI tabs each week (generated with `npm run products:build`)\n- Playbook + 15-minute setup\n\nUse `[YOUR BRAND]` placeholders throughout.",
    profile: "# Agent profile — Marketing Planner\n\n**Role:** Marketing operations lead for [YOUR BRAND].\n\n**Outcome:** A rolling 90-day plan, weekly priorities, and campaign briefs that connect to the Marketing Command Center tracker.\n\n**Inputs:** Offers, revenue goal, channels, team capacity, past campaign notes.\n\n**Outputs:** Weekly plan, campaign one-pagers, retro summaries, channel experiments.\n\n**KPIs:** Leads, sales, MER/ROAS, email growth, content shipped vs plan.",
    playbook: "# Marketing Planner — Playbook (SOP)\n\n## Purpose\nKeep [YOUR BRAND] marketing proactive: one source of truth in **Marketing-Command-Center.xlsx** plus AI-assisted briefs.\n\n## Weekly ops (Friday, 60 min)\n1. Export last week's numbers into the KPI tab.\n2. Run workflow **weekly-marketing-priorities** for next week.\n3. Update campaign tab statuses (idea/active/done).\n4. Assign owners and due dates.\n\n## Monthly (first Monday, 90 min)\nRun **ninety-day-campaign-roadmap** and **channel-experiment-queue**. Archive completed campaigns to `/marketing/archive`.\n\n## Launch protocol\nFor any launch: workflow **campaign-brief-one-pager** → creative tasks → workflow **launch-day-checklist** → workflow **post-launch-retro** within 7 days.\n\n## Meeting cadence\n15-min daily standup optional; 30-min weekly marketing sync mandatory when more than one person touches campaigns.\n\n## Quality bar\nEvery active campaign has: goal metric, owner, start/end, offer, primary channel, backup channel.\n\n## Tooling\nSpreadsheet is system of record; AI drafts narrative sections you paste into Notes columns.",
    setup: "# 15-minute setup — Marketing Planner\n\n**0–3 min:** Open `03-templates/Marketing-Command-Center.xlsx`. Save as `[YOUR BRAND]-marketing-command-center.xlsx`.\n\n**3–7 min:** Fill Business tab: offers, prices, URLs, monthly revenue target.\n\n**7–11 min:** Paste AGENT-PROFILE into AI instructions. Import last month's rough notes into Campaigns tab.\n\n**11–15 min:** Run workflow `weekly-marketing-priorities` for the coming week; paste results into Weekly Plan tab.\n\nNext: block Friday 60-minute marketing ops on calendar.",
    workflows: [
      workflow(
        "weekly-marketing-priorities",
        "Weekly marketing priorities",
        "Set top 5 priorities for the coming week with owners.",
        [
          "Revenue goal this month",
          "Active campaigns",
          "Last week KPI snapshot",
          "Team hours available",
          "Deadlines"
        ],
        "You are marketing ops lead for [YOUR BRAND]. Produce **next week's top 5 priorities** ranked by revenue impact.\n\nFormat: Priority | Outcome metric | Owner | Time estimate | Dependencies | Done definition.\n\nInclude one maintenance task (email hygiene, SEO, reporting) and one experiment. Tie each priority to a campaign name for the spreadsheet.",
        [
          "Exactly 5 priorities",
          "Each has owner and metric",
          "One experiment included",
          "Done definitions are testable"
        ],
      ),
      workflow(
        "ninety-day-campaign-roadmap",
        "90-day campaign roadmap",
        "Lay out campaigns for the next quarter.",
        [
          "Annual goal",
          "Product/offer list",
          "Seasonality",
          "Budget range",
          "Channels"
        ],
        "You are a campaign planner for [YOUR BRAND]. Build a **90-day roadmap** by week.\n\nColumns: Week | Campaign | Objective | Channel mix | Offer | Success metric | Risk.\n\nBalance launch weeks with nurture weeks. Note email, paid, organic, and partnerships. Flag resource conflicts.",
        [
          "Covers 13 weeks",
          "Each week has objective",
          "Channel mix present",
          "Risks noted"
        ],
      ),
      workflow(
        "campaign-brief-one-pager",
        "Campaign brief one-pager",
        "Single-page brief for one campaign.",
        [
          "Campaign name",
          "Goal metric",
          "Audience",
          "Offer",
          "Dates",
          "Budget"
        ],
        "You are a campaign strategist for [YOUR BRAND]. Write a **one-page campaign brief**.\n\nSections: Background | Objective & KPI | Audience insight | Big idea | Messaging pillars (3) | Channel plan | Creative needs | Timeline | Measurement | Risks.\n\nKeep it scannable with bullets. End with **approval checklist** for stakeholder sign-off.",
        [
          "All sections present",
          "KPI is numeric or binary",
          "Creative needs listed",
          "Timeline matches dates input"
        ],
      ),
      workflow(
        "channel-experiment-queue",
        "Channel experiment queue",
        "Prioritize 5 marketing experiments with hypotheses.",
        [
          "Channels in use",
          "Recent wins/failures",
          "Budget cap",
          "Skill constraints"
        ],
        "You are a growth experimenter for [YOUR BRAND]. Propose **5 experiments** for the next 30 days.\n\nEach: Hypothesis | Channel | Test design | Sample size/duration | Success metric | Kill criteria.\n\nPrioritize ICE score (Impact, Confidence, Ease) 1–10.",
        [
          "5 experiments",
          "Kill criteria on each",
          "ICE scores included",
          "Budget cap respected"
        ],
      ),
      workflow(
        "content-calendar-sync",
        "Content calendar sync",
        "Align content calendar with campaign roadmap.",
        [
          "90-day roadmap",
          "Current content calendar",
          "Blog/email/social capacity"
        ],
        "You are a content ops manager for [YOUR BRAND]. Reconcile campaign roadmap with content calendar.\n\nOutput: 4-week calendar table (date, asset, campaign tie-in, owner, status). Identify gaps where campaigns lack content and recommend 3 fill-in assets.",
        [
          "4 weeks mapped",
          "Campaign tie-ins explicit",
          "3 gap fillers",
          "Owners assigned"
        ],
      ),
      workflow(
        "launch-day-checklist",
        "Launch day checklist",
        "Hour-by-hour launch checklist for a campaign.",
        [
          "Launch datetime",
          "Channels launching",
          "Assets list",
          "Support contact",
          "Rollback plan"
        ],
        "You are a launch manager for [YOUR BRAND]. Create **launch day checklist** T-24h to T+24h.\n\nGroup by: email, social, paid, site, ops. Include verification steps (links, coupons, tracking UTMs). Add war-room roles and escalation if metrics dip.",
        [
          "T-24 to T+24 coverage",
          "UTM/check links included",
          "Roles assigned",
          "Rollback mentioned"
        ],
      ),
      workflow(
        "post-launch-retro",
        "Post-launch retrospective",
        "Structured retro 7 days after campaign end.",
        [
          "Campaign brief",
          "Results vs KPI",
          "Spend",
          "Team notes"
        ],
        "You are a marketing analyst for [YOUR BRAND]. Run a **post-launch retro**.\n\nSections: Results vs goal | What worked | What didn't | Data surprises | Process fixes | Keep/stop/start | Next test.\n\nBe blunt but constructive; cite numbers from inputs only.",
        [
          "Compares to KPI",
          "Keep/stop/start present",
          "One next test defined",
          "No invented metrics"
        ],
      ),
      workflow(
        "budget-reallocation-memo",
        "Budget reallocation memo",
        "Recommend budget shifts based on performance.",
        [
          "Channel spend MTD",
          "ROAS/MER by channel",
          "Upcoming campaigns",
          "Constraints"
        ],
        "You are a performance marketing lead for [YOUR BRAND]. Draft a **budget reallocation memo** for the owner.\n\nRecommend % shifts with rationale, risks, and what to pause. Include 7-day monitoring plan after changes.",
        [
          "Specific % shifts",
          "Rationale tied to ROAS/MER",
          "Pause list included",
          "Monitoring plan present"
        ],
      ),
      workflow(
        "partner-co-marketing-pitch",
        "Partner co-marketing pitch",
        "Outreach brief for a co-marketing partner.",
        [
          "Partner name/type",
          "Mutual audience",
          "Your offer",
          "Their offer",
          "Goal"
        ],
        "You are partnerships marketing for [YOUR BRAND]. Write a **co-marketing pitch email** and bullet proposal.\n\nProposal: joint webinar, bundle, newsletter swap, or giveaway. Include timeline, promo assets needed, and success metric.",
        [
          "Email ≤250 words",
          "Clear mutual benefit",
          "Timeline included",
          "Metric defined"
        ],
      ),
      workflow(
        "executive-marketing-summary",
        "Executive marketing summary",
        "One-page weekly summary for leadership.",
        [
          "KPI dashboard paste",
          "Campaign statuses",
          "Wins/losses",
          "Next week priorities"
        ],
        "You are CMO chief of staff for [YOUR BRAND]. Write a **weekly exec summary** (≤400 words).\n\nStructure: Headline | Numbers | Campaign pulse | Risks | Asks | Next week focus.\n\nWrite for a busy founder; no jargon without definition.",
        [
          "≤400 words",
          "Numbers from input only",
          "Risks and asks present",
          "Next week focus clear"
        ],
      ),
    ],
    templates: {
      "README-Marketing-Command-Center.txt": "Marketing Command Center spreadsheet\n\nThe full Excel workbook is copied to this folder when you run the product generator:\n  Marketing-Command-Center.xlsx\n\nTabs to use weekly:\n- Weekly Plan — top 5 priorities\n- Campaigns — status and owners\n- KPI — actuals vs goal\n\nCustomize tab names for [YOUR BRAND] but keep column headers intact for workflow compatibility.\n"
    },
  },
  {
    slug: "agent-05-email-sequence-agent-kit",
    readme: "# Email Sequence Agent Kit — [YOUR BRAND]\n\nWelcome, nurture, and launch sequences with copy-paste agent workflows.\n\n## Includes\n- 10 workflows covering welcome, nurture, launch, win-back, and QA\n- Playbook for list hygiene and send cadence\n- Setup in 15 minutes\n\nFor digital product and Etsy sellers building an owned audience. Use `[YOUR BRAND]` everywhere.",
    profile: "# Agent profile — Email Sequence\n\n**Role:** Email lifecycle copywriter for [YOUR BRAND].\n\n**Outcome:** Sequences that onboard subscribers, build trust, and convert without burning the list.\n\n**Inputs:** Offer, lead magnet, voice, ESP (Mailchimp, Klaviyo, etc.), compliance needs.\n\n**Outputs:** 3–7 email sequences with subjects, preview text, body, CTAs.\n\n**KPIs:** Open rate, click rate, unsub rate, revenue per recipient, reply rate (if applicable).",
    playbook: "# Email Sequence — Playbook (SOP)\n\n## Purpose\nShip and maintain lifecycle email for [YOUR BRAND] with consistent voice and measurable CTAs.\n\n## Core sequences\n1. **Welcome** (3–5 emails / 7 days) — deliver lead magnet, story, quick win, soft offer.\n2. **Nurture** (weekly) — value, proof, segmentation paths.\n3. **Launch** (5–7 emails / 10 days) — seed, teach, proof, offer, urgency, last call.\n\n## Weekly rhythm (45 min)\nReview metrics → pick one sequence to improve → run relevant workflow → paste into ESP → test links.\n\n## Compliance\nInclude physical address if required, unsubscribe link, honest subject lines. No fake \"Re:\" threads.\n\n## Segmentation\nTag buyers vs non-buyers after welcome. Suppress purchasers from cold launch unless upsell.\n\n## QA before send\nRun workflow **email-qa-compliance-pass** on every batch.\n\n## Sunsetting\nArchive sequences with <15% open after 3 tests; rewrite hooks using workflow **subject-line-battery**.",
    setup: "# 15-minute setup — Email Sequence\n\n**0–4 min:** Document lead magnet URL, core offer, ESP name, and from-name for [YOUR BRAND].\n\n**4–8 min:** Load AGENT-PROFILE into AI. Export current welcome sequence (if any) for reference.\n\n**8–12 min:** Run `welcome-sequence-5-email` with your lead magnet details.\n\n**12–15 min:** Create ESP folder structure: Welcome / Nurture / Launch / Win-back.\n\nPaste draft email 1; schedule smoke test to yourself.",
    workflows: [
      workflow(
        "welcome-sequence-5-email",
        "Welcome sequence (5 emails)",
        "Draft 5-email welcome over 7 days.",
        [
          "Lead magnet description",
          "Audience",
          "Core offer + price",
          "Brand story (3 bullets)",
          "Voice",
          "ESP merge tags"
        ],
        "You are an email copywriter for [YOUR BRAND]. Write a **5-email welcome sequence** over 7 days.\n\nFor each email: Send day | Subject (3 variants) | Preview text | Body (markdown) | CTA | Goal metric.\n\nArc: deliver asset → founder story → quick tutorial → social proof → soft pitch with guarantee framing. Use merge tags like {{first_name}} only if provided.",
        [
          "5 emails with send days",
          "3 subjects each",
          "Lead magnet delivered email 1",
          "Soft pitch only by email 5"
        ],
      ),
      workflow(
        "nurture-weekly-value-email",
        "Nurture weekly value email",
        "Single nurture email with teach + gentle CTA.",
        [
          "Topic",
          "Subscriber segment",
          "Past email topics to avoid repeating",
          "CTA destination",
          "Voice"
        ],
        "You are a nurture email writer for [YOUR BRAND]. Write **one weekly value email**.\n\nStructure: story hook → teach 3 bullets → example → CTA. 350–500 words. One P.S. line.\n\nAdd **segment variants** (buyer vs non-buyer) for CTA paragraph only.",
        [
          "350–500 words",
          "Single primary CTA",
          "Segment variants included",
          "No duplicate past topics"
        ],
      ),
      workflow(
        "launch-sequence-7-email",
        "Launch sequence (7 emails)",
        "Product launch sequence with ethical urgency.",
        [
          "Offer name/price/deadline",
          "Bonuses",
          "Proof (testimonials)",
          "Objections (3)",
          "Cart link"
        ],
        "You are a launch copywriter for [YOUR BRAND]. Write **7 emails** over 10 days for a product launch.\n\nMap: seed problem → methodology → case study → offer reveal → FAQ → urgency → last call.\n\nSubjects must not be deceptive. Include plain-text link line. Note which email to skip for existing customers.",
        [
          "7 emails mapped to days",
          "FAQ email addresses objections",
          "Deadline honest",
          "Customer skip note"
        ],
      ),
      workflow(
        "subject-line-battery",
        "Subject line battery",
        "Generate 30 subject lines for testing.",
        [
          "Email summary",
          "Audience awareness level",
          "Tone",
          "Words to avoid"
        ],
        "You are an email CTR specialist for [YOUR BRAND]. Write **30 subject lines** grouped by angle: curiosity, benefit, proof, direct, question.\n\nEach ≤50 characters where possible. Flag 5 as 'high risk spam' to avoid. Pair top 5 with preview text.",
        [
          "30 subjects",
          "5 categories",
          "Preview text for top 5",
          "Spam flags explained"
        ],
      ),
      workflow(
        "cart-abandon-3-email",
        "Cart abandon (3 emails)",
        "Recover checkout or Etsy favorite intent ethically.",
        [
          "Product",
          "Price",
          "Objections",
          "Support link",
          "Incentive policy (yes/no)"
        ],
        "You are a lifecycle marketer for [YOUR BRAND]. Write **3 cart/checkout reminder emails** at 1h, 24h, 72h.\n\nEmail 1: help + FAQ. Email 2: social proof. Email 3: last nudge (only if incentive allowed).\n\nNo fake 'your cart is expiring' unless true.",
        [
          "3 emails with timing",
          "Incentive policy respected",
          "Support link included",
          "No false urgency"
        ],
      ),
      workflow(
        "win-back-sequence",
        "Win-back sequence",
        "Re-engage cold subscribers in 3 emails.",
        [
          "Months inactive",
          "Last known interest",
          "New offer or content",
          "Sunset policy"
        ],
        "You are email retention lead for [YOUR BRAND]. Write **3 win-back emails**.\n\nEmail 1: 'still want this?' with one-click preference update. Email 2: best resource since they left. Email 3: goodbye/sunset with clear unsubscribe encouragement.\n\nTone respectful, not guilt-tripping.",
        [
          "3 emails",
          "Preference update CTA",
          "Sunset policy referenced",
          "Respectful tone"
        ],
      ),
      workflow(
        "segmentation-tag-map",
        "Segmentation tag map",
        "Define ESP tags and triggers from behavior.",
        [
          "Offers list",
          "Behaviors tracked (clicks, purchases)",
          "ESP name",
          "Goals"
        ],
        "You are CRM architect for [YOUR BRAND]. Design a **tag map**: Tag name | Trigger | Entry email sequence | Exit rules.\n\nInclude at least 6 tags covering buyer, engaged, cold, VIP, launch-interest, support-issue.\n\nAdd diagram in mermaid optional.",
        [
          "≥6 tags",
          "Triggers defined",
          "Sequences named",
          "Exit rules present"
        ],
      ),
      workflow(
        "email-qa-compliance-pass",
        "Email QA & compliance",
        "Final QA before sending any campaign.",
        [
          "Full email draft",
          "Jurisdiction (US/EU)",
          "Physical address available?",
          "Link list"
        ],
        "You are email compliance QA for [YOUR BRAND]. Audit the draft.\n\nChecklist: subject honesty, unsubscribe, address, link validity placeholders, spam words, accessibility (alt text note), mobile length.\n\nOutput Pass/Fail with fix list prioritized.",
        [
          "Pass/Fail stated",
          "Fix list prioritized",
          "Unsubscribe mentioned",
          "Links listed for verification"
        ],
      ),
      workflow(
        "newsletter-edition-template",
        "Newsletter edition template",
        "Recurring newsletter structure with sections.",
        [
          "Theme of week",
          "3 content links",
          "Product spotlight",
          "Personal note seed",
          "Voice"
        ],
        "You are newsletter editor for [YOUR BRAND]. Write one **newsletter edition** using fixed sections: Opener | Tip of the week | Link trio | Spotlight | CTA | Sign-off.\n\n600–900 words max. Skimmable subheads.",
        [
          "All sections present",
          "Word count in range",
          "One primary CTA",
          "Links from inputs used"
        ],
      ),
      workflow(
        "post-purchase-onboarding-email",
        "Post-purchase onboarding",
        "Onboard digital product buyers to reduce refunds.",
        [
          "Product delivered",
          "Download/portal instructions",
          "Common setup issues",
          "Upsell (optional)"
        ],
        "You are customer success copywriter for [YOUR BRAND]. Write **4 post-purchase emails** over 14 days.\n\nFocus: delivery confirmation → quick start → advanced tip → review request + optional upsell.\n\nReduce support tickets with proactive FAQs.",
        [
          "4 emails",
          "Download steps clear",
          "FAQ preempts issues",
          "Review ask on email 4"
        ],
      ),
    ],
  },
  {
    slug: "agent-06-ad-copy-agent-kit",
    readme: "# Ad Copy Agent Kit — [YOUR BRAND]\n\nMeta and Google ad copy workflows: angles, headlines, extensions, and testing matrices.\n\n10 workflows + playbook + 15-minute setup. Replace `[YOUR BRAND]` before use.",
    profile: "# Agent profile — Ad Copy\n\n**Role:** Performance copywriter for [YOUR BRAND] paid social and search.\n\n**Outcome:** Ad variants ready for Meta Ads Manager and Google Ads with clear testing plan.\n\n**Inputs:** Offer, audience, landing URL, proof points, constraints (claims, compliance).\n\n**Outputs:** Primary text, headlines, descriptions, RSA sets, creative briefs.\n\n**KPIs:** CTR, CPC, CPA/ROAS, quality score (search), hook rate (video).",
    playbook: "# Ad Copy — Playbook (SOP)\n\n## Purpose\nProduce disciplined ad creative tests for [YOUR BRAND] without random one-off copy.\n\n## Weekly cycle (90 min)\nMonday: pick single offer + angle hypothesis. Tuesday: run workflows 01–03. Wednesday: upload variants. Thursday: monitor 48h leading indicators. Friday: document winners in testing log.\n\n## Testing rules\nChange one variable at a time when possible (hook OR visual OR audience). Minimum spend threshold before killing (set in sheet).\n\n## Platform notes\n**Meta:** 3–5 primary texts × 3 headlines per ad set. **Google RSA:** 8–15 headlines, 4 descriptions, pin sparingly.\n\n## Compliance\nNo before/after body claims unless allowed. Disclose digital product nature. Follow ad platform policies for income claims.\n\n## Creative handoff\nPair every copy set with workflow **ad-creative-brief-for-designer** before design starts.\n\n## Archive\nStore winning ads in `/ads/winners` with date and metrics snapshot.",
    setup: "# 15-minute setup — Ad Copy\n\n**0–4 min:** List offers, landing URLs, pixel/GA4 status, monthly ad budget cap.\n\n**4–8 min:** Paste AGENT-PROFILE + past best ad screenshots into AI project.\n\n**8–12 min:** Run `meta-angle-matrix` for one offer.\n\n**12–15 min:** Create spreadsheet columns: Angle | Hook | Result | Notes.\n\nUpload top 3 variants to ads manager as drafts.",
    workflows: [
      workflow(
        "meta-angle-matrix",
        "Meta angle matrix",
        "Generate ad angles for Facebook/Instagram.",
        [
          "Offer + price",
          "Audience",
          "Pain/desire",
          "Proof",
          "Landing page summary",
          "Restrictions"
        ],
        "You are a Meta ads strategist for [YOUR BRAND]. Build an **angle matrix** with 6 angles: pain, aspiration, proof, demo, objection-killer, founder story.\n\nPer angle: 3 hooks (first line) | 2 primary texts (125/250 char versions) | Headline | CTA button recommendation | Creative idea.\n\nMark angles that need video vs static.",
        [
          "6 angles complete",
          "Hooks distinct",
          "Restrictions honored",
          "CTA recommendations given"
        ],
      ),
      workflow(
        "google-rsa-asset-pack",
        "Google RSA asset pack",
        "Responsive search ad headlines and descriptions.",
        [
          "Keyword theme",
          "Offer",
          "USPs (3)",
          "Landing page URL",
          "Competitor positioning"
        ],
        "You are a Google Ads copywriter for [YOUR BRAND]. Produce **15 headlines** (≤30 chars each) and **4 descriptions** (≤90 chars).\n\nGroup headlines by theme: keyword, benefit, proof, CTA. Suggest pinning strategy. Include display path ideas.",
        [
          "15 headlines ≤30 chars",
          "4 descriptions ≤90 chars",
          "Pin strategy noted",
          "Keywords used naturally"
        ],
      ),
      workflow(
        "ugc-ad-script-30s",
        "UGC ad script (30s)",
        "Direct-response UGC script for paid social.",
        [
          "Product",
          "Avatar",
          "Problem",
          "Proof point",
          "Offer terms"
        ],
        "You are a DR video ad copywriter for [YOUR BRAND]. Write a **30s UGC script** with hook, problem agitation, demo, proof, CTA.\n\nInclude b-roll notes and on-screen text. Two hook variants for first 3 seconds.",
        [
          "Two hook variants",
          "CTA with offer terms",
          "≤30s read time",
          "Proof included"
        ],
      ),
      workflow(
        "ad-creative-brief-for-designer",
        "Ad creative brief",
        "Brief designer or Canva creator for ad visuals.",
        [
          "Copy set chosen",
          "Brand colors/fonts",
          "Format sizes needed",
          "Must-include elements"
        ],
        "You are creative director for [YOUR BRAND]. Write a **design brief** for paid ad statics/carousels.\n\nDeliver: concept | layout wire description | text hierarchy | 3 size variants | do/don't | export specs.",
        [
          "Sizes listed",
          "Text hierarchy clear",
          "Do/don't present",
          "Matches chosen copy"
        ],
      ),
      workflow(
        "landing-page-message-match",
        "Landing page message match",
        "Align ad promise with landing page above-the-fold.",
        [
          "Ad copy paste",
          "Current landing headline",
          "Offer details"
        ],
        "You are a CRO copywriter for [YOUR BRAND]. Compare ad vs landing page.\n\nScore message match 0–10. Rewrite landing hero (headline, subhead, bullet proof) to align without changing offer facts.",
        [
          "Score given",
          "Hero rewrite provided",
          "Offer facts unchanged",
          "Bullets include proof"
        ],
      ),
      workflow(
        "retargeting-ad-variants",
        "Retargeting ad variants",
        "Warm audience ads for site visitors or engagers.",
        [
          "Audience temperature",
          "Days since visit",
          "Objections",
          "Incentive allowed"
        ],
        "You are retargeting specialist for [YOUR BRAND]. Write **4 ad variants** for warm audiences.\n\nAngles: reminder, testimonial, FAQ busting, incentive (if allowed). Short primary text suitable for retargeting windows.",
        [
          "4 variants",
          "Matches temperature",
          "Incentive policy respected",
          "Short copy"
        ],
      ),
      workflow(
        "search-negative-keyword-copy-audit",
        "Search query copy audit",
        "Suggest negatives and copy tweaks from search terms.",
        [
          "Search terms export (paste top 30)",
          "Target keywords",
          "Offer"
        ],
        "You are PPC analyst for [YOUR BRAND]. From search terms, propose **negative keywords** and **copy tweaks** to improve relevance.\n\nTable: Term | Intent match | Action (negate/ad group/copy) | Suggested ad line.",
        [
          "≥10 negatives or tweaks",
          "Intent explained",
          "No contradicting offer",
          "Table format"
        ],
      ),
      workflow(
        "ab-test-documentation",
        "A/B test documentation",
        "Document test structure before spend.",
        [
          "Hypothesis",
          "Variants description",
          "Budget",
          "Duration",
          "Success metric"
        ],
        "You are experimentation lead for [YOUR BRAND]. Write a **test plan** doc.\n\nSections: Hypothesis | Setup | Variables isolated | Sample size guidance (qualitative) | Monitoring | Decision rules | Learnings template.",
        [
          "Hypothesis clear",
          "One variable focus",
          "Decision rules stated",
          "Learnings template included"
        ],
      ),
      workflow(
        "seasonal-ad-refresh",
        "Seasonal ad refresh",
        "Refresh ad copy for seasonal promotion.",
        [
          "Season/event",
          "Promo terms",
          "Existing winning ads",
          "Deadline"
        ],
        "You are seasonal campaign copywriter for [YOUR BRAND]. Refresh **3 winning ads** with seasonal hooks.\n\nProvide before/after snippets and note what stays unchanged for learning continuity.",
        [
          "3 refreshes",
          "Promo terms accurate",
          "Before/after shown",
          "Deadline respected"
        ],
      ),
      workflow(
        "compliance-claims-scrub",
        "Compliance claims scrub",
        "Remove or soften risky claims in ad copy.",
        [
          "Draft ad copy",
          "Industry (digital products/Etsy/etc.)",
          "Platform policies focus"
        ],
        "You are ad compliance editor for [YOUR BRAND]. Scrub copy for income, health, and misleading urgency claims.\n\nOutput: redline list | safer replacements | final cleaned copy set.",
        [
          "Redlines listed",
          "Safer replacements given",
          "Final copy provided",
          "Urgency claims verified"
        ],
      ),
    ],
  },
  {
    slug: "agent-07-sales-proposal-discovery-agent-kit",
    readme: "# Sales Proposal & Discovery Agent Kit — [YOUR BRAND]\n\nB2B discovery, proposals, and follow-up workflows for service sellers and agencies.\n\n10 workflows + SOP + quick setup. Customize `[YOUR BRAND]` and pricing.",
    profile: "# Agent profile — Sales Proposal & Discovery\n\n**Role:** B2B sales enablement partner for [YOUR BRAND].\n\n**Outcome:** Cleaner discovery, sharper proposals, faster follow-ups, higher close rate.\n\n**Inputs:** Offer, ICP, pricing model, case studies, call notes.\n\n**Outputs:** Discovery guides, proposals, SOW sections, follow-up emails.\n\n**KPIs:** Qualified pipeline, proposal win rate, cycle length, next-step rate after calls.",
    playbook: "# Sales Proposal & Discovery — Playbook (SOP)\n\n## Purpose\nStandardize how [YOUR BRAND] moves from first call to signed agreement.\n\n## Stages\n1. **Qualify** — workflow 01 before booking deep discovery.\n2. **Discover** — workflow 02 live; workflow 03 summary within 2 hours.\n3. **Propose** — workflow 04–06 within 48 hours of discovery.\n4. **Follow up** — workflow 07–08 cadence days 1, 3, 7, 14.\n5. **Close or nurture** — workflow 09 for objections; 10 for lost deal learning.\n\n## CRM hygiene\nEvery call note becomes structured SPICED or similar via workflow 03. No proposal without documented pain metric.\n\n## Pricing\nUse ranges in discovery; firm numbers only in proposal after scope locked.\n\n## Legal\nWorkflow outputs are drafts—not legal advice. Owner reviews contract terms.\n\n## Weekly pipeline (30 min)\nReview open proposals, update stages, run follow-ups due today.",
    setup: "# 15-minute setup — Sales Proposal & Discovery\n\n**0–5 min:** Write one-page offer sheet: deliverables, starting price, timelines, disqualifiers.\n\n**5–9 min:** Load AGENT-PROFILE into AI; link CRM or tracker.\n\n**9–13 min:** Run `discovery-call-question-bank` for your top offer.\n\n**13–15 min:** Save proposal template doc from workflow `proposal-outline-generator` output.\n\nBook next pipeline review.",
    workflows: [
      workflow(
        "qualification-scorecard",
        "Qualification scorecard",
        "Score inbound lead fit before deep discovery.",
        [
          "Lead source",
          "Company/role",
          "Stated need",
          "Budget signal",
          "Timeline",
          "ICP criteria"
        ],
        "You are sales development for [YOUR BRAND]. Score lead **0–100** on fit, need, budget, timing, authority.\n\nOutput: Score | Go/No-go | 3 clarifying questions | Suggested next step (book/call/nurture/pass).\n\nBe willing to disqualify politely.",
        [
          "Numeric score",
          "Go/No-go clear",
          "3 questions",
          "Next step recommended"
        ],
      ),
      workflow(
        "discovery-call-question-bank",
        "Discovery question bank",
        "Custom discovery guide for one offer.",
        [
          "Offer description",
          "ICP",
          "Typical objections",
          "Call length (minutes)"
        ],
        "You are discovery coach for [YOUR BRAND]. Create a **question bank** grouped: context, pain, impact, decision process, success criteria, logistics.\n\nInclude follow-up probes and red flag answers. Time-box sections for allotted minutes.",
        [
          "All groups present",
          "Follow-up probes included",
          "Time-box noted",
          "Red flags listed"
        ],
      ),
      workflow(
        "call-notes-to-spiced-summary",
        "Call notes → SPICED summary",
        "Structure raw notes into SPICED summary.",
        [
          "Raw call notes",
          "Attendees",
          "Date",
          "Next step promised"
        ],
        "You are sales operations for [YOUR BRAND]. Convert notes into **SPICED** (Situation, Pain, Impact, Critical Event, Decision).\n\nAdd: quoted phrases, metrics mentioned, risks, mutual action plan with dates.",
        [
          "SPICED complete",
          "Quotes included",
          "MAP with dates",
          "Risks noted"
        ],
      ),
      workflow(
        "proposal-outline-generator",
        "Proposal outline",
        "Generate proposal skeleton before full write.",
        [
          "SPICED summary",
          "Offer components",
          "Pricing model",
          "Case study snippet"
        ],
        "You are proposal architect for [YOUR BRAND]. Create **proposal outline**: Executive summary bullets | Approach phases | Deliverables table | Timeline | Investment | Assumptions | Next steps.\n\nMark `[CUSTOMIZE]` fields for client name and dates.",
        [
          "Deliverables table",
          "Timeline included",
          "Investment section",
          "Assumptions listed"
        ],
      ),
      workflow(
        "scope-of-work-writer",
        "Scope of work writer",
        "Detailed SOW section from agreed scope.",
        [
          "Approved outline",
          "Inclusions/exclusions",
          "Revision rounds",
          "Support terms"
        ],
        "You are SOW writer for [YOUR BRAND]. Draft **Scope of Work** with numbered deliverables, acceptance criteria, out-of-scope list, client responsibilities, change order clause (plain language).\n\nAvoid vague verbs like 'optimize' without metrics.",
        [
          "Acceptance criteria per deliverable",
          "Out-of-scope list",
          "Client responsibilities",
          "Change order mentioned"
        ],
      ),
      workflow(
        "pricing-options-3-tier",
        "3-tier pricing options",
        "Good/better/best packaging for proposal.",
        [
          "Base deliverables",
          "Margins/goals",
          "Client budget hint",
          "Optional add-ons"
        ],
        "You are packaging strategist for [YOUR BRAND]. Design **3 tiers** with names, price anchors, feature bullets, recommended tier callout.\n\nExplain psychology of anchor. Note which tier protects margin.",
        [
          "3 tiers",
          "Recommended tier identified",
          "Add-ons listed",
          "Prices align to hint"
        ],
      ),
      workflow(
        "proposal-follow-up-email",
        "Proposal follow-up email",
        "Follow-up after proposal sent.",
        [
          "Client name",
          "Proposal sent date",
          "Key value point",
          "Open questions from call"
        ],
        "You are account executive for [YOUR BRAND]. Write **follow-up email** day 2 after proposal.\n\nShort, assume busy reader: recap value, address one likely objection, suggest 15-min review call with two time options. No guilt language.",
        [
          "≤200 words",
          "One objection addressed",
          "Two time options",
          "Clear CTA"
        ],
      ),
      workflow(
        "stakeholder-forwardable-blurb",
        "Stakeholder forwardable blurb",
        "Paragraph champion can forward internally.",
        [
          "Client champion role",
          "Problem statement",
          "Proposed outcome",
          "Investment range"
        ],
        "You are sales copywriter for [YOUR BRAND]. Write a **forwardable blurb** (≤150 words) champion can paste to boss.\n\nFocus business case, ROI logic, risk of inaction—not feature list.",
        [
          "≤150 words",
          "ROI logic",
          "Risk of inaction",
          "No jargon overload"
        ],
      ),
      workflow(
        "objection-handling-sheet",
        "Objection handling sheet",
        "Responses for top objections on this deal.",
        [
          "Objections heard (3–5)",
          "Offer",
          "Proof points",
          "Competitor context"
        ],
        "You are sales coach for [YOUR BRAND]. For each objection, give **acknowledge | reframe | proof | question back** script.\n\nInclude 'walk away gracefully' line if objection is fundamental.",
        [
          "Each objection has 4-part script",
          "Proof cited",
          "Walk-away line present",
          "Questions are open-ended"
        ],
      ),
      workflow(
        "loss-retrospective",
        "Loss retrospective",
        "Learn from lost deals systematically.",
        [
          "Deal summary",
          "Stage lost",
          "Stated reason",
          "Internal notes"
        ],
        "You are sales analyst for [YOUR BRAND]. Write **loss retro**: true reason hypothesis | process gaps | competitive intel | one playbook change | nurture re-entry date if appropriate.\n\nHonest tone; no blame on prospect.",
        [
          "Hypothesis labeled",
          "One playbook change",
          "Nurture date or 'do not nurture'",
          "No invented facts"
        ],
      ),
    ],
  },
  {
    slug: "agent-08-copy-swipe-agent-kit",
    readme: "# Copy Swipe Agent Kit — [YOUR BRAND]\n\nAdapt proven copy frameworks to your offers—emails, ads, landing pages, and product descriptions.\n\nIncludes **03-templates/QUICK-REFERENCE.md** and references the full **50-business-copywriting-templates.md** swipe library (bundled path in Business-in-a-Box products).\n\n10 workflows + playbook. Replace `[YOUR BRAND]`.",
    profile: "# Agent profile — Copy Swipe\n\n**Role:** Conversion copy adapter for [YOUR BRAND].\n\n**Outcome:** Fast drafts by combining swipe frameworks with your specifics—never generic filler.\n\n**Inputs:** Offer, audience, proof, tone, channel constraints.\n\n**Outputs:** Headlines, emails, ads, product blurbs, CTA blocks.\n\n**KPIs:** Time-to-first draft, approval rate, conversion on published pieces.",
    playbook: "# Copy Swipe — Playbook (SOP)\n\n## Purpose\nUse frameworks (AIDA, PAS, BAB, 4Ps) systematically for [YOUR BRAND] instead of blank-page writing.\n\n## How to use swipes\n1. Pick framework in QUICK-REFERENCE.md matching channel.\n2. Run matching workflow with your inputs.\n3. Human edit for truth and brand voice.\n\n## Weekly (30 min)\nMaintain a **swipe log**: framework used | asset | result metric | keep/kill.\n\n## Ethics\nSwipes are structure inspiration—do not plagiarize competitor copy verbatim.\n\n## Bundled library note\nFor the full 50-template markdown library, copy from your **business-in-a-box-starter** or **copywriting-templates** product into `03-templates/` if not already present.\n\n## QA\nEvery piece gets workflow **voice-consistency-pass** before publish.",
    setup: "# 15-minute setup — Copy Swipe\n\n**0–4 min:** Open `03-templates/QUICK-REFERENCE.md` and pin frameworks you use most.\n\n**4–8 min:** Paste brand voice rules (3 do / 3 don't) into AI project.\n\n**8–12 min:** Run `pas-offer-blurb` on hero product.\n\n**12–15 min:** Save output to `/copy/swipe-log.md` with date and framework tag.",
    workflows: [
      workflow(
        "pas-offer-blurb",
        "PAS offer blurb",
        "Problem-Agitate-Solve blurb for offer page or Etsy listing.",
        [
          "Audience",
          "Problem",
          "Offer",
          "Proof",
          "CTA"
        ],
        "You are conversion copywriter for [YOUR BRAND]. Write **PAS blurb** 150–250 words.\n\nStrong specific problem, agitate with consequences (no fearmongering), solve with offer + proof + CTA.\n\nEnd with 3 bullet benefits.",
        [
          "PAS structure visible",
          "150–250 words",
          "3 bullets",
          "CTA present"
        ],
      ),
      workflow(
        "aida-landing-hero",
        "AIDA landing hero",
        "Above-the-fold hero using AIDA.",
        [
          "Offer",
          "Audience",
          "Key proof",
          "Primary CTA text"
        ],
        "You are landing page copywriter for [YOUR BRAND]. Write **AIDA hero**: headline | subhead | 3 bullets | CTA button text | micro-trust line.\n\nHeadline ≤12 words where possible.",
        [
          "All AIDA elements",
          "Headline concise",
          "CTA matches input",
          "Trust line included"
        ],
      ),
      workflow(
        "bab-transformation-email",
        "BAB transformation email",
        "Before-After-Bridge story email.",
        [
          "Customer before state",
          "After state",
          "Bridge (your product)",
          "Story seed",
          "CTA"
        ],
        "You are email storyteller for [YOUR BRAND]. Write **BAB email** 400–550 words with subject lines (3).\n\nUse concrete sensory details in before/after. Bridge explains mechanism without hype.",
        [
          "BAB clear",
          "Word count range",
          "3 subjects",
          "Mechanism explained"
        ],
      ),
      workflow(
        "four-ps-sales-page-section",
        "4Ps sales section",
        "One sales page section using 4Ps.",
        [
          "Section topic",
          "Promise",
          "Proof assets",
          "Push (offer terms)"
        ],
        "You are long-form copywriter for [YOUR BRAND]. Write **4Ps section** with subheads for Promise, Picture, Proof, Push.\n\nInclude placeholder for testimonial pull quote.",
        [
          "4 subheads",
          "Proof specific",
          "Push has offer terms",
          "Testimonial placeholder"
        ],
      ),
      workflow(
        "fab-product-bullets",
        "FAB product bullets",
        "Feature-Advantage-Benefit bullet stack.",
        [
          "Feature list",
          "Audience job-to-be-done",
          "Competitor weakness (optional)"
        ],
        "You are product copywriter for [YOUR BRAND]. Convert features into **10 FAB bullets** (Feature → Advantage → Benefit in one line each).\n\nLead with outcomes; keep each bullet ≤2 lines.",
        [
          "10 bullets",
          "FAB logic each",
          "Outcome-led",
          "≤2 lines each"
        ],
      ),
      workflow(
        "headline-swipe-25",
        "25 headline swipes",
        "Generate headline options from swipe patterns.",
        [
          "Offer",
          "Audience",
          "Tone",
          "Channel"
        ],
        "You are headline specialist for [YOUR BRAND]. Write **25 headlines** using varied patterns: how-to, number, question, contrast, without [pain].\n\nTag pattern type per headline. Star top 5 for testing.",
        [
          "25 headlines",
          "Patterns tagged",
          "Top 5 marked",
          "Channel appropriate"
        ],
      ),
      workflow(
        "cta-block-variants",
        "CTA block variants",
        "10 CTA microcopy blocks for buttons and links.",
        [
          "Offer action",
          "Risk reversal",
          "Urgency (if true)",
          "Voice"
        ],
        "You are CTA copywriter for [YOUR BRAND]. Write **10 CTA blocks**: button text + supporting line under button.\n\nMix value, urgency (only if true), and risk reversal.",
        [
          "10 blocks",
          "Urgency honest",
          "Risk reversal in ≥3",
          "Voice consistent"
        ],
      ),
      workflow(
        "social-proof-paragraph",
        "Social proof paragraph",
        "Turn rough testimonial into polished proof paragraph.",
        [
          "Raw testimonial",
          "Customer role",
          "Metric if any",
          "Placement (email/page)"
        ],
        "You are editor for [YOUR BRAND]. Polish testimonial into **proof paragraph** + pull quote ≤20 words.\n\nDo not invent metrics; mark [VERIFY] if missing.",
        [
          "Pull quote ≤20 words",
          "No invented metrics",
          "Placement noted",
          "Reads naturally"
        ],
      ),
      workflow(
        "voice-consistency-pass",
        "Voice consistency pass",
        "Edit draft to match brand voice rules.",
        [
          "Draft copy",
          "Voice do/don't list",
          "Reading level target"
        ],
        "You are brand editor for [YOUR BRAND]. Edit draft for voice, clarity, reading level.\n\nOutput: revised copy | change log (bullet) | flagged clichés removed.",
        [
          "Change log included",
          "Do/don't applied",
          "Reading level noted",
          "Full revised copy"
        ],
      ),
      workflow(
        "channel-trim-rewrite",
        "Channel trim rewrite",
        "Resize one core message for 3 channels.",
        [
          "Core message paragraph",
          "Channels (e.g., email, IG, Etsy)",
          "Limits per channel"
        ],
        "You are channel editor for [YOUR BRAND]. Rewrite core message for **3 channels** respecting limits.\n\nPreserve meaning; adjust hook and CTA per channel.",
        [
          "3 versions",
          "Limits respected",
          "Meaning preserved",
          "CTAs per channel"
        ],
      ),
    ],
    templates: {
      "QUICK-REFERENCE.md": "# Copy frameworks — quick reference ([YOUR BRAND])\n\n| Framework | Best for | Structure |\n|-----------|----------|-----------|\n| AIDA | Ads, landing heroes | Attention → Interest → Desire → Action |\n| PAS | Email, pain-led offers | Problem → Agitate → Solve |\n| BAB | Transformation stories | Before → After → Bridge |\n| 4Ps | Bulky sales pages | Promise → Picture → Proof → Push |\n| FAB | Product bullets | Features → Advantages → Benefits |\n\n## Full library\nThe complete **50-business-copywriting-templates.md** ships in the Copywriting Templates / Business-in-a-Box bundle. Copy that file into this folder's `03-templates/` for offline use.\n\n## Usage\n1. Pick framework for channel\n2. Run matching workflow in `02-workflows/`\n3. Log result in swipe-log\n"
    },
  },
  {
    slug: "agent-09-pod-design-prompt-agent-kit",
    readme: "# POD Design Prompt Agent Kit — [YOUR BRAND]\n\nMidjourney/DALL·E/SD-style prompts for print-on-demand niches—t-shirts, mugs, wall art, and digital prints.\n\n10 workflows + playbook + setup. Use `[YOUR BRAND]` and verify IP/trademark rules before selling.",
    profile: "# Agent profile — POD Design Prompt\n\n**Role:** POD art director and prompt engineer for [YOUR BRAND].\n\n**Outcome:** Niche-specific, print-ready design briefs and AI image prompts with style consistency.\n\n**Inputs:** Niche, product blank, print provider specs, brand aesthetic, banned themes.\n\n**Outputs:** Prompt packs, style guides, collections, QC checklists.\n\n**KPIs:** Designs approved per batch, listing conversion, refund rate, repeat buyers.",
    playbook: "# POD Design Prompt — Playbook (SOP)\n\n## Purpose\nBatch-create original POD artwork prompts for [YOUR BRAND] without style drift or IP risk.\n\n## Weekly batch (2 hours)\nSelect niche → workflow 01 collection brief → workflows 02–04 generate prompts → human generates images → workflow 09 QC → upload to POD/Etsy.\n\n## Style bible\nMaintain `/pod/style-bible.md` updated via workflow 05 monthly.\n\n## IP hygiene\nNo celebrities, sports teams, trademarked characters, or stolen art styles presented as official. Workflow 10 mandatory before publish.\n\n## Print specs\nAlways attach DPI/size from provider (Printful, Printify, etc.) in workflow inputs.\n\n## Collections\nRelease in collections of 6–12 designs with shared palette per workflow 06.",
    setup: "# 15-minute setup — POD Design Prompt\n\n**0–4 min:** Choose primary niche and 2 product types (e.g., tee + mug).\n\n**4–8 min:** Record print area pixels/DPI and safe margins from provider.\n\n**8–12 min:** Run `niche-collection-brief` for first collection name.\n\n**12–15 min:** Save first 5 prompts to `/pod/batch-001.md`.\n\nGenerate images in your AI tool; run QC workflow before listing.",
    workflows: [
      workflow(
        "niche-collection-brief",
        "Niche collection brief",
        "Plan a 8-design POD collection for a niche.",
        [
          "Niche/audience",
          "Product types",
          "Season",
          "Competitor style notes",
          "Banned topics"
        ],
        "You are POD merchandiser for [YOUR BRAND]. Create **collection brief**: name | buyer persona | 8 design concepts (title + one-line concept) | shared palette (hex) | typography style | mockup notes.\n\nConcepts must be original, niche-specific, not generic 'live laugh love' unless ironic per brief.",
        [
          "8 concepts",
          "Palette hex codes",
          "Persona clear",
          "Banned topics avoided"
        ],
      ),
      workflow(
        "midjourney-prompt-pack",
        "Midjourney prompt pack",
        "10 Midjourney-style prompts with parameters.",
        [
          "Design concepts list",
          "Style references (legal)",
          "Aspect ratio",
          "Color palette"
        ],
        "You are MJ prompt engineer for [YOUR BRAND]. Write **10 prompts** with --ar, style tokens, lighting, composition, negative prompts.\n\nSeparate **print-safe** (no tiny text) versions. Note upscaling step.",
        [
          "10 prompts",
          "Aspect ratio set",
          "Negative prompts",
          "Print-safe note"
        ],
      ),
      workflow(
        "dalle-prompt-pack",
        "DALL·E prompt pack",
        "10 DALL·E-friendly descriptive prompts.",
        [
          "Concepts",
          "Style adjectives",
          "Background preference (transparent/solid)",
          "Product type"
        ],
        "You are DALL·E art director for [YOUR BRAND]. Write **10 detailed prompts** optimized for clean POD graphics.\n\nSpecify flat illustration vs photo vs watercolor. Request high contrast for apparel.",
        [
          "10 prompts",
          "Style consistent",
          "Contrast noted for apparel",
          "Background preference"
        ],
      ),
      workflow(
        "typography-shirt-text",
        "Typography shirt text",
        "Short witty text-only shirt ideas with layout notes.",
        [
          "Niche inside jokes",
          "Max words per design",
          "Tone",
          "Words to avoid"
        ],
        "You are apparel copy designer for [YOUR BRAND]. Create **12 text-only shirt lines** with font style, placement, and color on shirt mockup notes.\n\nAvoid offensive content; explain joke briefly for outsider readability.",
        [
          "12 lines",
          "Font/placement each",
          "Word limits",
          "Offense check"
        ],
      ),
      workflow(
        "style-bible-generator",
        "Style bible generator",
        "Document reusable visual style for collections.",
        [
          "Brand adjectives",
          "Sample prompts that worked",
          "Niches",
          "Color palette"
        ],
        "You are brand designer for [YOUR BRAND]. Write **style bible** section: palette | line weight | shading | composition rules | do/don't | example prompt snippets.\n\nKeep it usable by any future prompt workflow.",
        [
          "Palette documented",
          "Do/don't lists",
          "Example prompts",
          "Composition rules"
        ],
      ),
      workflow(
        "seasonal-pod-drop",
        "Seasonal POD drop",
        "Seasonal prompt set with timing.",
        [
          "Holiday/season",
          "Lead time weeks",
          "Top products",
          "Prior year winners"
        ],
        "You are seasonal POD planner for [YOUR BRAND]. Plan **drop**: launch date | 6 prompts | marketing hook | Etsy tag angles (text only).\n\nInclude retire date for designs.",
        [
          "6 prompts",
          "Launch/retire dates",
          "Marketing hook",
          "Tag angles"
        ],
      ),
      workflow(
        "pattern-repeat-wall-art",
        "Seamless pattern brief",
        "Wall art / fabric seamless pattern prompts.",
        [
          "Theme",
          "Repeat style",
          "Color count",
          "Room context photo?"
        ],
        "You are pattern designer for [YOUR BRAND]. Write **5 seamless pattern prompts** with tile notes, scale, and color limits.\n\nMention testing repeat in editor.",
        [
          "5 patterns",
          "Tile/scale notes",
          "Color count respected",
          "Test note"
        ],
      ),
      workflow(
        "mug-wrap-composition",
        "Mug wrap composition",
        "Prompts optimized for mug wrap safe area.",
        [
          "Mug template specs",
          "Theme",
          "Handle side rules"
        ],
        "You are product mockup specialist for [YOUR BRAND]. Create **6 mug designs** with composition notes keeping art away from handle glare zone.\n\nProvide prompt + placement diagram in words.",
        [
          "6 designs",
          "Handle zone respected",
          "Specs referenced",
          "Prompt each"
        ],
      ),
      workflow(
        "pod-qc-checklist",
        "POD design QC",
        "QC AI art before sending to print.",
        [
          "Image description or checklist paste",
          "Product type",
          "Print DPI/size",
          "Provider"
        ],
        "You are POD QC lead for [YOUR BRAND]. Run **QC checklist**: resolution | edges | contrast | text legibility | IP red flags | background removal needs.\n\nOutput Pass/Fail and fix instructions for designer.",
        [
          "Pass/Fail",
          "IP check included",
          "DPI mentioned",
          "Fix instructions"
        ],
      ),
      workflow(
        "ip-trademark-screen",
        "IP / trademark screen",
        "Screen design concepts for obvious IP issues.",
        [
          "Design titles/descriptions",
          "Niche",
          "Risk tolerance"
        ],
        "You are IP screening assistant for [YOUR BRAND] (not a lawyer). Flag **likely risky elements**: celebrities, brands, sports, movie quotes, trademarked phrases.\n\nSuggest safer alternates. Recommend human legal review when borderline.",
        [
          "Risks flagged",
          "Alternates suggested",
          "Legal disclaimer",
          "No 'approved' language"
        ],
      ),
    ],
  },
  {
    slug: "agent-10-listing-mockup-photo-brief-agent-kit",
    readme: "# Listing Mockup & Photo Brief Agent Kit — [YOUR BRAND]\n\nEtsy listing image strategy: shot lists, mockup briefs, thumbnail tests, and alt text for digital products.\n\n10 workflows + playbook + 15-minute setup. Customize `[YOUR BRAND]`.",
    profile: "# Agent profile — Listing Mockup & Photo Brief\n\n**Role:** Visual merchandising lead for [YOUR BRAND] Etsy listings.\n\n**Outcome:** Cohesive image sets that lift CTR and clarify digital/POD deliverables.\n\n**Inputs:** Product type, files included, brand aesthetic, competitor thumbnails.\n\n**Outputs:** Shot lists, mockup briefs, Canva/PS notes, thumbnail variants.\n\n**KPIs:** Listing CTR, favorites, conversion, lower 'what do I get?' messages.",
    playbook: "# Listing Mockup & Photo Brief — Playbook (SOP)\n\n## Purpose\nStandardize how [YOUR BRAND] produces 5–10 listing images that explain the product in 2 seconds.\n\n## Image set formula\n1 Hero mockup | 2 What's included | 3 How to use | 4 Size/format proof | 5 Lifestyle or context | 6 Trust (reviews/license) | 7 FAQ visual | 8 Optional video cover.\n\n## Per-SKU workflow (60 min)\nRun 01 strategy → 02 shot list → 03 mockup brief → produce in Canva/PS → 09 thumbnail test → publish.\n\n## Tools\nCanva, Photoshop, smart mockups, or AI backgrounds—briefs specify.\n\n## Consistency\nUse brand kit colors/fonts from workflow 05 each quarter.\n\n## Mobile first\nThumbnail readable at 300px width—workflow 09 required.",
    setup: "# 15-minute setup — Listing Mockup & Photo Brief\n\n**0–4 min:** Export brand colors, fonts, and logo PNG for [YOUR BRAND].\n\n**4–8 min:** Screenshot 3 competitor thumbnails in your niche.\n\n**8–12 min:** Run `listing-image-strategy` for one SKU.\n\n**12–15 min:** Create Canva folder `Etsy Listing Images/[SKU]`.\n\nExecute shot list starting with hero mockup.",
    workflows: [
      workflow(
        "listing-image-strategy",
        "Listing image strategy",
        "Plan full image stack for one listing.",
        [
          "Product summary",
          "Deliverable formats",
          "Buyer confusion risks",
          "Competitor notes"
        ],
        "You are Etsy visual strategist for [YOUR BRAND]. Plan **8 listing images** with purpose per slot, on-image text (≤5 words each), and primary color background choices.\n\nNote which images are optional for digital vs physical.",
        [
          "8 slots defined",
          "On-image text short",
          "Buyer risks addressed",
          "Digital/physical noted"
        ],
      ),
      workflow(
        "shot-list-generator",
        "Shot list generator",
        "Detailed shot list for photographer or DIY.",
        [
          "Image strategy",
          "Equipment available",
          "Location",
          "Props list budget"
        ],
        "You are photo producer for [YOUR BRAND]. Expand strategy into **shot list**: shot ID | angle | lighting | props | file name convention.\n\nInclude B-roll for video cover if applicable.",
        [
          "Shot IDs",
          "Lighting noted",
          "Naming convention",
          "Matches strategy"
        ],
      ),
      workflow(
        "digital-mockup-brief",
        "Digital product mockup brief",
        "Brief for screen/mockup showing digital files.",
        [
          "Screens to show (PDF, Canva, etc.)",
          "Device frame preference",
          "Brand kit"
        ],
        "You are mockup art director for [YOUR BRAND]. Write brief for **digital product hero**: layout grid, devices, callouts, export size 2000px+, font sizes for mobile.\n\nSpecify do not use misleading UI.",
        [
          "Export size",
          "Devices listed",
          "Callouts",
          "No misleading UI"
        ],
      ),
      workflow(
        "whats-included-grid",
        "What's included grid",
        "Visual grid brief listing every file/template.",
        [
          "File list with counts",
          "Icons style",
          "Dimensions if relevant"
        ],
        "You are infographic brief writer for [YOUR BRAND]. Design **'What's included'** grid: rows/columns, icon per item, short labels, emphasis on total count.\n\nOutput wireframe in words + copy for each cell.",
        [
          "Every file listed",
          "Grid layout",
          "Total count highlighted",
          "Labels short"
        ],
      ),
      workflow(
        "brand-kit-listing-refresh",
        "Brand kit listing refresh",
        "Quarterly refresh of listing visual standards.",
        [
          "Current brand kit",
          "Top 5 listings",
          "Underperforming CTR listings"
        ],
        "You are brand manager for [YOUR BRAND]. Propose **listing visual refresh**: thumbnail rules, typography scale, badge usage, before/after examples on underperformers.\n\nLimit to changes feasible in Canva in one day.",
        [
          "Thumbnail rules",
          "Typography scale",
          "Examples on 2 listings",
          "One-day feasible"
        ],
      ),
      workflow(
        "lifestyle-scene-prompt",
        "Lifestyle scene prompt",
        "AI or stock brief for lifestyle background.",
        [
          "Product use context",
          "Audience",
          "Mood",
          "Aspect ratio"
        ],
        "You are lifestyle art director for [YOUR BRAND]. Write **scene prompt** for AI/stock: setting, props, model hands if needed, depth of field, color grade.\n\nKeep product area clear for mockup composite.",
        [
          "Clear product zone",
          "Mood matches audience",
          "Aspect ratio",
          "Props listed"
        ],
      ),
      workflow(
        "video-cover-thumbnail",
        "Video cover thumbnail",
        "Brief for Etsy video thumbnail/cover frame.",
        [
          "Video script summary",
          "Key frame moment",
          "Text overlay rules"
        ],
        "You are video thumbnail designer for [YOUR BRAND]. Specify **cover frame**: composition, text ≤3 words, contrast, freeze moment timestamp.\n\nMust read at small size.",
        [
          "≤3 words on cover",
          "Timestamp noted",
          "Small-size readable",
          "Contrast high"
        ],
      ),
      workflow(
        "size-chart-visual",
        "Size chart visual",
        "Size/dimension chart for apparel or prints.",
        [
          "Sizes/dimensions",
          "Units",
          "Product type",
          "Fit notes"
        ],
        "You are technical apparel designer for [YOUR BRAND]. Brief **size chart image**: table layout, measurement diagrams, disclaimer text.\n\nAccessible fonts; inches and cm if needed.",
        [
          "All sizes",
          "Diagram described",
          "Disclaimer",
          "Units correct"
        ],
      ),
      workflow(
        "thumbnail-ab-variants",
        "Thumbnail A/B variants",
        "3 thumbnail concepts for CTR testing.",
        [
          "Current thumbnail description",
          "Primary keyword/benefit",
          "Brand constraints"
        ],
        "You are CTR optimizer for [YOUR BRAND]. Propose **3 thumbnail variants** with different hooks: outcome, curiosity, social proof.\n\nDescribe visuals precisely for designer; note Etsy 1:1 crop.",
        [
          "3 distinct hooks",
          "1:1 crop noted",
          "Designer-ready detail",
          "Brand constraints"
        ],
      ),
      workflow(
        "listing-image-qc",
        "Listing image QC",
        "Final QC before upload to Etsy.",
        [
          "List of image filenames + descriptions",
          "Listing title",
          "Policy notes"
        ],
        "You are Etsy listing QC for [YOUR BRAND]. Score image set 0–100 on clarity, consistency, mobile readability, policy (no misleading).\n\nList fixes per image; go/no-go.",
        [
          "Score 0–100",
          "Per-image fixes",
          "Policy check",
          "Go/no-go"
        ],
      ),
    ],
  },
];
