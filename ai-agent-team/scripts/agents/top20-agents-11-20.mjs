/**
 * Top 20 AI Agent Kit definitions (kits 11–20).
 * Consumed by scripts/generate-top20-agents.mjs (or merged catalog builders).
 */

function workflow(slug, title, job, inputs, brief, qc) {
  return { slug, title, job, inputs, brief, qc };
}

export const AGENT_KITS_11_20 = [
  {
    slug: 'agent-11-customer-support-reply-agent-kit',
    readme: `# Customer Support Reply Agent Kit — [YOUR BRAND]

Turn messy inbound tickets into fast, on-brand replies that de-escalate and close loops.

## What you get
- **AGENT-PROFILE.md** — role, inputs, outputs, KPIs
- **01-playbook/** — daily support operating rhythm
- **02-workflows/** — 10 copy-paste agent briefs (triage → resolution → QA)
- **05-implementation/** — 15-minute setup

## Who this is for
Etsy sellers, course creators, SaaS founders, and service businesses who answer the same questions daily and need consistent tone without sounding robotic.

## Quick start
1. Open \`05-implementation/setup-guide.md\`
2. Paste your policies into the support context doc
3. Run workflow **01-ticket-triage-classifier** on your inbox backlog
4. Draft replies with workflows 02–06; finish with **10-reply-qa-tone-check**

Replace every \`[YOUR BRAND]\` placeholder before you run workflows.
`,
    profile: `# Agent profile — Customer Support Reply

**Role:** Frontline support copywriter and triage partner for [YOUR BRAND].

**Outcome:** Every customer gets a clear, empathetic reply within your SLA with correct policy and next steps.

**Inputs:** Ticket text, order/subscription ID, policy snippets, tone rules, past thread.

**Outputs:** Triage labels, draft replies, internal notes, escalation summaries, macro suggestions.

**KPIs:** First-response time, resolution time, CSAT/review sentiment, reopen rate, refunds saved (where policy allows).

**Guardrails:** No promises outside policy, no medical/legal advice, no sharing other customers' data, escalate threats and chargebacks immediately.
`,
    playbook: `# Customer Support Reply — Playbook (SOP)

## Purpose
Run a repeatable daily loop so [YOUR BRAND] support stays human, fast, and policy-correct.

## Roles
- **Owner:** approves policy updates, handles escalations, refunds above threshold.
- **Support agent (AI):** triage + drafts via workflows in \`02-workflows/\`; human sends.

## Daily rhythm (≈60–90 minutes)
**Open (10 min):** Sort inbox by urgency. Run workflow 01 on anything older than SLA.

**Draft block (40 min):** Batch similar tickets. Use workflows 02–06 by type (how-to, angry, billing, shipping, technical).

**Escalation (10 min):** Workflow 07 for anything legal, safety, or repeat contact.

**Close loop (15 min):** Workflow 08 follow-ups on waiting customers. Log macros worth saving (workflow 09).

**QA (10 min):** Workflow 10 on every reply before send during first two weeks.

## SLA defaults (customize)
- Urgent (order blocked, payment failed): same business day
- Standard: within 24 hours
- Low priority (pre-sales): within 48 hours

## Macro hygiene
Weekly: add 1–2 approved replies to macro library via workflow 09. Retire macros with <80% reuse after 30 days.

## Quality bar
One primary ask per email. Acknowledge emotion before policy. Never argue in writing—offer path forward.

## Escalation triggers
Chargeback threat, harassment, GDPR/privacy request, bug causing data loss, influencer/press, repeat contact 3+ without resolution.
`,
    setup: `# 15-minute setup — Customer Support Reply Agent

## Minutes 0–3: Policy spine
Create **[YOUR BRAND] Support Context** with: refund window, shipping times, digital download help link, escalation email, words to avoid, signature block.

## Minutes 3–6: Tooling
Pin **AGENT-PROFILE.md** in your AI project. Connect helpdesk or label Gmail/Outlook folder "Support — draft".

## Minutes 6–10: Tone samples
Paste 3 best past replies (one warm, one firm, one technical) into the context doc as "gold examples".

## Minutes 10–13: First triage
Run \`02-workflows/01-ticket-triage-classifier.md\` on 5 real open tickets. Tag them in your tracker.

## Minutes 13–15: SLA + calendar
Block 60 minutes daily on calendar labeled "Support batch". Set phone notification for urgent tag only.

You are ready. Draft your highest-urgency ticket with workflow 02 or 03 next.
`,
    workflows: [
      workflow(
        'ticket-triage-classifier',
        'Ticket triage classifier',
        'Label and prioritize one or many support messages.',
        [
          'Paste ticket(s) with timestamps',
          'Product/order type',
          'SLA rules',
          'Tags you already use (optional)',
        ],
        `You are support operations lead for [YOUR BRAND]. Triage each message.

For each ticket output:
- **ID** (use subject line or number provided)
- **Category** (billing, shipping, how-to, bug, refund, pre-sales, abuse, other)
- **Urgency** (P0–P3) with reason
- **Sentiment** (calm/frustrated/angry)
- **Suggested workflow** (02–10 from this kit)
- **Needs human escalation?** (yes/no + why)
- **One-line summary**

If multiple tickets, sort P0 first. Flag missing order IDs. Do not draft full replies yet—classification only.`,
        [
          'Every ticket has category and urgency',
          'Escalation flag explicit',
          'Sorted by priority when batch',
          'No full reply drafts',
        ],
      ),
      workflow(
        'how-to-step-reply',
        'How-to step-by-step reply',
        'Answer product how-to questions for non-technical users.',
        [
          'Customer message',
          'Product/version',
          'Known setup steps or help doc URL',
          'Screenshots described (optional)',
        ],
        `You are patient support educator for [YOUR BRAND]. Write a **customer reply** that teaches.

Structure: warm greeting → restate question → numbered steps (max 7) → what success looks like → offer follow-up → sign-off.

Use plain language. Bold UI labels. Link to help doc once. If unsure, ask one clarifying question instead of guessing.

Also output **internal note** (2 bullets) for CRM.`,
        [
          'Numbered steps present',
          'No jargon without definition',
          'One help link max',
          'Internal note included',
        ],
      ),
      workflow(
        'angry-customer-de-escalation',
        'Angry customer de-escalation',
        'De-escalate frustrated messages without admitting liability.',
        [
          'Full thread',
          'What went wrong (facts)',
          'Policy limits',
          'Approved compensation options',
        ],
        `You are de-escalation specialist for [YOUR BRAND]. Draft reply to an upset customer.

Rules: acknowledge feelings, apologize for **experience** (not legal fault unless confirmed), state facts neutrally, offer **one** clear remedy path aligned with policy, set timeline, no debate, no sarcasm.

Include **do-not-say** list (3 phrases to avoid) and **if-no-response** follow-up line for workflow 08.`,
        [
          'Empathy before policy',
          'Single remedy path',
          'No liability admission beyond policy',
          'Timeline stated',
        ],
      ),
      workflow(
        'billing-payment-reply',
        'Billing & payment reply',
        'Handle invoices, failed payments, and receipt requests.',
        [
          'Customer question',
          'Payment platform (Etsy, Stripe, PayPal, etc.)',
          'Transaction ID if any',
          'Refund/charge policy',
        ],
        `You are billing support for [YOUR BRAND]. Draft accurate billing reply.

Cover: what they were charged, date, product name, how to access receipt/invoice, next steps if payment failed, how to update card (if applicable). Never share full card numbers.

If duplicate charge suspected, list verification steps before refund promise.`,
        [
          'Amount/date/product referenced',
          'Receipt steps clear',
          'No sensitive payment data exposed',
          'Verification before refund promise',
        ],
      ),
      workflow(
        'shipping-delivery-status-reply',
        'Shipping & delivery status',
        'Reply to where-is-my-order and tracking questions.',
        [
          'Order date and shipping method',
          'Tracking number/status (if any)',
          'Stated processing times',
          'International customs note (if relevant)',
        ],
        `You are logistics support for [YOUR BRAND]. Draft shipping status reply.

Include: current status, tracking link placeholder [TRACKING_URL], realistic delivery window, what to do if lost (policy), digital vs physical clarity if mixed order.

Tone reassuring, not defensive about carriers.`,
        [
          'Status and window clear',
          'Lost-package policy cited',
          'Digital vs physical clarified if needed',
          'Tracking placeholder if unknown',
        ],
      ),
      workflow(
        'technical-bug-report-reply',
        'Technical issue & bug report',
        'Collect repro info and set expectations on fixes.',
        [
          'Customer description',
          'Platform/browser/device',
          'Known issues list',
          'Workaround if any',
        ],
        `You are technical support for [YOUR BRAND]. Draft reply that **gathers repro** without overwhelming user.

Ask up to 4 targeted questions (device, steps, screenshot, error text). Provide workaround if known. Set expectation on fix timeline [CUSTOMIZE]. Thank them for report.

Add **engineering ticket title** one-liner for internal tracker.`,
        [
          '≤4 diagnostic questions',
          'Workaround if available',
          'Internal ticket title',
          'Timeline placeholder flagged',
        ],
      ),
      workflow(
        'escalation-internal-handoff',
        'Escalation internal handoff',
        'Summarize thread for owner or specialist.',
        [
          'Full thread paste',
          'Customer tier (standard/VIP)',
          'What customer wants',
          'Policy already offered',
        ],
        `You are support lead for [YOUR BRAND]. Write **internal escalation brief** (not customer-facing).

Sections: Customer snapshot | Timeline | Facts vs assumptions | Risk (legal/reputation/churn) | Recommended resolution options (A/B/C) | Draft customer holding reply (≤80 words).

Label urgency. Suggest owner decision needed by [DATE].`,
        [
          'Timeline included',
          'Three resolution options',
          'Holding reply ≤80 words',
          'Risk section present',
        ],
      ),
      workflow(
        'waiting-customer-follow-up',
        'Waiting customer follow-up',
        'Nudge when customer or team owes a response.',
        [
          'Last message date',
          'Who owes reply',
          'Open promise made',
          'New info since last contact',
        ],
        `You are support follow-up writer for [YOUR BRAND]. Draft short follow-up.

If we owe reply: apologize for delay + status + new ETA. If customer owes info: friendly bump with bullet list of what we need. Max 120 words.`,
        [
          '≤120 words',
          'ETA or specific ask',
          'References last promise',
          'Tone warm not guilt-trip',
        ],
      ),
      workflow(
        'macro-snippet-library-update',
        'Macro library update',
        'Turn a great reply into a reusable macro.',
        [
          'Final sent reply',
          'Category',
          'Merge fields available (name, order id, etc.)',
        ],
        `You are support knowledge manager for [YOUR BRAND]. Convert reply into **macro template**.

Output: Macro name | When to use | Template with {{placeholders}} | 2 variant openings | Tags.

Strip customer-specific facts. Keep tone. Note policy footers to attach.`,
        [
          'Placeholders labeled',
          'When-to-use clear',
          'No customer PII left in template',
          'Two opening variants',
        ],
      ),
      workflow(
        'reply-qa-tone-check',
        'Reply QA & tone check',
        'Final QA before sending any support reply.',
        [
          'Draft reply',
          "Brand voice do/don't list",
          'Applicable policy',
          'Customer language (if non-English, note)',
        ],
        `You are support QA editor for [YOUR BRAND]. Audit draft reply.

Score 0–100: Empathy (25), Clarity (25), Policy accuracy (25), Tone (15), Next step (10). List **must-fix** and **optional polish**. Provide revised reply if score <85.

Check: one CTA, no double apology spam, no forbidden phrases from voice rules.`,
        [
          'Score and sub-scores',
          'Revised reply if <85',
          'Must-fix list',
          'One primary CTA',
        ],
      ),
    ],
  },
  {
    slug: 'agent-12-etsy-buyer-conversation-agent-kit',
    readme: "# Etsy Buyer Conversation Agent Kit — [YOUR BRAND]\n\nEtsy Messages replies for orders, customization, digital downloads, and pre-sales—on-policy and conversion-aware.\n\n## What you get\n- 10 workflows for Etsy-specific buyer scenarios\n- Playbook for message batching and star-seller habits\n- 15-minute setup\n\n## Best for\nEtsy shop owners handling high message volume on digital, POD, and made-to-order products.\n\nReplace `[YOUR BRAND]` before running workflows.",
    profile: "# Agent profile — Etsy Buyer Conversation\n\n**Role:** Etsy shop messenger for [YOUR BRAND].\n\n**Outcome:** Fast, friendly Etsy Messages that protect reviews, clarify digital delivery, and close custom-order details.\n\n**Inputs:** Buyer message, order ID, listing type, shop policies, processing times.\n\n**Outputs:** Reply drafts, conversation summaries, custom order clarifiers, review-safe language.\n\n**KPIs:** Message response time (Etsy stats), review rate, conversion on custom requests, case rate.",
    playbook: "# Etsy Buyer Conversation — Playbook (SOP)\n\n## Purpose\nKeep [YOUR BRAND] Etsy response rate and sentiment strong without living in the inbox.\n\n## Daily batch (30–45 min)\nRun workflow 01 on unread → draft with 02–07 by type → workflow 10 QA → send via Etsy (human click).\n\n## Rules\nNever discuss refunds outside Etsy case flow when possible. Digital: always restate download steps. Custom: confirm in writing before production.\n\n## Star Seller\nTarget <24h response. Use workflow 08 for proactive shipping/delay messages.\n\n## Review moments\nWorkflow 09 only after order delivered and issue resolved—never bribe for reviews.",
    setup: "# 15-minute setup — Etsy Buyer Conversation\n\n**0–4 min:** Copy Etsy shop policies (processing, refunds, digital) into one note.\n\n**4–8 min:** Paste AGENT-PROFILE into AI; add listing types you sell.\n\n**8–12 min:** Run `01-etsy-message-intent-router` on 3 recent threads.\n\n**12–15 min:** Pin Etsy Messages on phone; block daily 30-min batch.\n\nNext: draft oldest unread with workflow 02 or 04.",
    workflows: [
      workflow(
        'etsy-message-intent-router',
        'Etsy message intent router',
        'Classify Etsy buyer messages and pick the right workflow.',
        ['Buyer message paste', 'Order status if linked', 'Listing type', 'Shop policies summary'],
        "You are Etsy inbox triage for [YOUR BRAND]. Classify message intent: pre-sales, customization, digital help, shipping, damaged/wrong, review threat, spam, other.\n\nOutput: Intent | Urgency | Suggested workflow (02–09) | Info missing from buyer | Review risk (low/med/high).\n\nDo not draft full reply.",
        ['Intent labeled', 'Workflow suggested', 'Review risk noted', 'No full draft'],
      ),
      workflow(
        'pre-sales-etsy-reply',
        'Pre-sales Etsy reply',
        'Answer buyer questions before purchase.',
        ['Buyer question', 'Listing URL/title', 'Price/shipping', 'Differentiators'],
        "You are Etsy pre-sales specialist for [YOUR BRAND]. Draft Etsy Message reply ≤150 words.\n\nAnswer directly, mention 1 differentiator, clarify digital vs physical, invite purchase with soft CTA. Etsy tone: warm, human, no external links unless Etsy allows for your shop type [CUSTOMIZE].",
        ['≤150 words', 'Digital/physical clear', 'Direct answer first', 'Soft CTA'],
      ),
      workflow(
        'custom-order-clarifier',
        'Custom order clarifier',
        'Lock customization details before making.',
        ['Buyer request', 'Listing customization rules', 'Lead time', 'Extra fees policy'],
        "You are custom order coordinator for [YOUR BRAND]. Draft message that **confirms specs** in bullet list: size, color, text, deadline, price delta.\n\nAsk buyer to reply 'approved' before production. Note revision round policy.",
        ['Bulleted specs', 'Approval request', 'Fee/lead time mentioned', 'Revision policy'],
      ),
      workflow(
        'digital-download-help-reply',
        'Digital download help',
        'Fix I cant download / where is my file issues.',
        ['Buyer issue', 'Delivery method (Etsy digital, email, etc.)', 'File types', 'FAQ steps'],
        "You are digital delivery support for [YOUR BRAND] on Etsy. Step-by-step download instructions for non-technical buyers.\n\nNumber steps. Mention mobile vs desktop. Offer one alternative delivery if policy allows. No blame.",
        ['Numbered steps', 'Mobile note', 'Alternative if allowed', 'Patient tone'],
      ),
      workflow(
        'order-delay-proactive-update',
        'Proactive delay update',
        'Message buyer before they ask about late order.',
        ['Original ship date', 'New ETA', 'Reason (brief)', 'Compensation policy'],
        "You are proactive Etsy seller for [YOUR BRAND]. Draft **unsolicited update** message.\n\nApologize briefly, new date, what you're doing, optional goodwill per policy. Keep <120 words.",
        ['New ETA', '<120 words', 'Goodwill only if policy allows', 'No over-promising'],
      ),
      workflow(
        'damaged-wrong-item-resolution',
        'Damaged or wrong item',
        'Resolve physical order problems on Etsy.',
        ['Issue description', 'Photo offered? (yes/no)', 'Reship/refund policy', 'Order value'],
        "You are Etsy resolution specialist for [YOUR BRAND]. Draft reply: empathize → ask for photo if needed → offer resolution path (reship/refund/partial) per policy → timeline.\n\nStay Etsy-case-friendly; avoid asking to cancel off-platform.",
        ['Resolution path clear', 'Photo request if needed', 'Timeline', 'On-platform tone'],
      ),
      workflow(
        'etsy-case-chargeback-prep',
        'Case or chargeback prep',
        'Draft factual reply when buyer threatens case.',
        ['Thread history', 'Tracking/proof of delivery', 'Policy', 'What we can offer'],
        "You are Etsy seller advocate for [YOUR BRAND]. Draft calm, factual reply. Summarize what was delivered, cite policy kindly, offer one fair remedy.\n\nAlso output **internal case notes** bullet list with evidence to upload.",
        ['Customer reply + internal notes', 'Facts only', 'One remedy', 'Evidence list'],
      ),
      workflow(
        'review-request-after-resolution',
        'Post-resolution review ask',
        'Ask for review only after positive resolution.',
        ['Issue resolved summary', 'Buyer sentiment', 'Shop review policy'],
        "You are Etsy reputation manager for [YOUR BRAND]. Draft gentle review request **only** appropriate after good outcome.\n\nThank them, mention how reviews help small shop, no incentives, easy opt-out. If issue unresolved, say **do not send**.",
        ['No incentive language', 'Skip flag if unresolved', 'Gratitude genuine', '≤100 words'],
      ),
      workflow(
        'conversation-close-summary',
        'Conversation close summary',
        'Close thread with clear next steps.',
        ['Thread paste', 'Agreed outcome', 'Follow-up date if any'],
        "You are Etsy inbox closer for [YOUR BRAND]. Draft closing message confirming agreement, what happens next, how to reach you, friendly sign-off.\n\nInternal: 1-line CRM note.",
        ['Agreement restated', 'Next steps', 'CRM note', 'Friendly close'],
      ),
      workflow(
        'etsy-message-qa-pass',
        'Etsy message QA',
        'QA Etsy replies for policy and tone.',
        ['Draft message', 'Etsy policies', 'Listing facts'],
        "You are Etsy message QA for [YOUR BRAND]. Check: length, no prohibited claims, digital clarity, no review manipulation, star-seller tone.\n\nScore 0–100; rewrite if needed.",
        ['Score given', 'Rewrite if <85', 'Policy checks', 'No review bribery'],
      ),
    ],
  },
  {
    slug: 'agent-13-review-reputation-response-agent-kit',
    readme: "# Review & Reputation Response Agent Kit — [YOUR BRAND]\n\nRespond to public reviews, harvest testimonials, and handle reputation crises with calm, on-brand copy.\n\n10 workflows + playbook + 15-minute setup. Use `[YOUR BRAND]` placeholders.",
    profile: "# Agent profile — Review & Reputation Response\n\n**Role:** Reputation copywriter for [YOUR BRAND].\n\n**Outcome:** Timely review responses, testimonial-ready quotes, and crisis statements that protect trust.\n\n**Inputs:** Review text, platform (Google, Etsy, G2, etc.), facts, brand voice.\n\n**Outputs:** Public replies, private follow-ups, testimonial edits, FAQ updates from patterns.\n\n**KPIs:** Response rate, rating trend, testimonial conversion, negative review resolution rate.",
    playbook: "# Review & Reputation — Playbook (SOP)\n\n## Purpose\nSystematize how [YOUR BRAND] shows up publicly when customers praise or criticize.\n\n## Weekly (45 min)\nScan platforms → workflow 01 prioritize → draft 02–05 → workflow 10 QA → publish.\n\n## Negative reviews\nRespond within 48h. Take detailed conversation offline when possible. Never argue publicly.\n\n## Testimonials\nWorkflow 06 monthly from 4–5 star private feedback.\n\n## Crisis\nWorkflow 08 for spikes in 1-star; owner approves before post.",
    setup: "# 15-minute setup — Review & Reputation\n\n**0–4 min:** List all review platforms and login links.\n\n**4–8 min:** Paste 2 positive + 1 negative response you admire into context.\n\n**8–12 min:** Run `01-review-priority-queue`.\n\n**12–15 min:** Calendar weekly reputation block.\n\nStart with highest-impact negative review via workflow 03.",
    workflows: [
      workflow('review-priority-queue', 'Review priority queue', 'Rank unreplied reviews by impact.', ['List of reviews with stars and dates', 'Platforms', 'Business priorities'], "You are reputation ops for [YOUR BRAND]. Rank reviews needing response.\n\nTable: Platform | Stars | Age | Priority (1-5) | Suggested workflow | Risk.\n\nPrioritize recent 1-3 star and high-visibility platforms.", ['All reviews ranked', 'Workflow mapped', 'Risk noted', 'Table format']),
      workflow('five-star-thank-you-reply', '5-star thank-you reply', 'Public thank-you for positive reviews.', ['Review text', 'Product/service mentioned', 'Voice'], "You are [YOUR BRAND] community voice. Write short public thank-you (≤80 words). Personalize one detail from review. No incentive for more reviews.", ['≤80 words', 'Personal detail', 'No bribery', 'Warm tone']),
      workflow('three-star-balanced-reply', '3-star balanced reply', 'Respond to mixed reviews constructively.', ['Review text', 'Valid points', 'Improvements made'], "Draft public reply acknowledging positives and addressing concerns with **specific fix or invite to DM/email**. Professional, not defensive.", ['Acknowledges positives', 'Specific next step', 'Invite offline', 'Professional']),
      workflow('one-star-recovery-reply', '1-star recovery reply', 'De-escalate public 1-star reviews.', ['Review text', 'Facts verified', 'Remediation offered', 'Contact channel'], "Draft public reply: apologize for experience, state commitment, invite to private channel, **no arguing with accusations**. Internal: investigation checklist.", ['Offline invite', 'No public argument', 'Internal checklist', 'Apology for experience']),
      workflow('fake-spam-review-flag', 'Fake or spam review', 'Draft response or platform report strategy.', ['Review text', 'Evidence of fake/order mismatch', 'Platform rules'], "Assess if review violates platform policy. Draft brief public response if required + **reporting steps** for owner. Do not accuse customer of crime in public text.", ['Report steps', 'Neutral public text', 'Evidence list', 'No legal accusations']),
      workflow('testimonial-polish-from-review', 'Testimonial polish', 'Turn review into website testimonial.', ['Original review', 'Customer name/role permission', 'Placement (sales page, Etsy)'], "Polish into testimonial ≤40 words pull quote + longer 2-sentence version. Mark [PERMISSION NEEDED] if name unclear.", ['Pull quote ≤40 words', 'Permission flag', 'Two lengths', 'Facts unchanged']),
      workflow('review-insight-themes-report', 'Review themes report', 'Monthly themes from reviews for product/ops.', ['Paste 20+ reviews or summaries', 'Date range'], "You are insights analyst for [YOUR BRAND]. Cluster themes: praise, complaints, feature requests. Top 5 each. Recommend 3 product/ops changes.", ['5 themes each bucket', '3 recommendations', 'Quotes anonymized', 'Date range noted']),
      workflow('reputation-crisis-statement', 'Reputation crisis statement', 'Draft holding statement for review spike or incident.', ['What happened (facts)', 'Audience', 'Channels', 'Legal constraints'], "Draft **holding statement** for website/social: acknowledge, what we know, what we're doing, when next update. ≤200 words. Owner must approve.", ['≤200 words', 'No speculation', 'Next update time', 'Approval note']),
      workflow('private-follow-up-after-public', 'Private follow-up', 'DM/email after public review reply.', ['Public reply sent', 'Customer contact', 'Offer'], "Write private message deepening resolution offer. Personal, concise, one clear CTA.", ['References public reply', 'One CTA', 'Concise', 'Policy-safe offer']),
      workflow('review-response-qa', 'Review response QA', 'Final QA on public review replies.', ['Draft reply', 'Platform rules', 'Facts'], "Score 0-100 on tone, accuracy, policy, brevity. Rewrite if <90. Public replies must be shorter than support emails.", ['Score + rewrite', 'Brevity check', 'Fact check', 'Platform rules']),
    ],
  },
  {
    slug: 'agent-14-refund-dispute-resolution-agent-kit',
    readme: "# Refund & Dispute Resolution Agent Kit — [YOUR BRAND]\n\nPolicy-clear refund offers, dispute documentation, and save attempts that protect margin and relationships.\n\n10 workflows + SOP + setup. Customize `[YOUR BRAND]` policies before use.",
    profile: "# Agent profile — Refund & Dispute Resolution\n\n**Role:** Resolution specialist for [YOUR BRAND].\n\n**Outcome:** Fair, documented outcomes: refund, partial credit, replacement, or firm decline with empathy.\n\n**Inputs:** Order facts, policy, customer ask, payment processor rules.\n\n**Outputs:** Customer letters, internal decision logs, evidence packets, save offers.\n\n**KPIs:** Dispute win rate, refund rate, save rate, time to resolve, repeat purchase after refund.",
    playbook: "# Refund & Dispute — Playbook (SOP)\n\n## Purpose\nHandle [YOUR BRAND] refund requests consistently—no ad-hoc exceptions that train bad behavior.\n\n## Flow\n01 qualify → 02 save attempt if eligible → 03–05 offer type → 06 document → 07 customer message → 10 QA.\n\n## Thresholds\nOwner approves refunds above $[AMOUNT] or outside policy.\n\n## Chargebacks\nWorkflow 08 immediately; pause other offers.\n\n## Logging\nEvery decision in spreadsheet: date, SKU, reason code, outcome, margin impact.",
    setup: "# 15-minute setup — Refund & Dispute\n\n**0–5 min:** Write refund policy one-pager with windows and exceptions.\n\n**5–9 min:** Add processor dispute links (Stripe, PayPal, Etsy).\n\n**9–13 min:** Run `01-refund-request-qualifier` on last 3 requests.\n\n**13–15 min:** Create log sheet columns: ID, reason, outcome, $.\n\nUse workflow 02 before issuing automatic refunds.",
    workflows: [
      workflow('refund-request-qualifier', 'Refund request qualifier', 'Decide if request is in policy.', ['Customer ask', 'Order date', 'Product type', 'Policy text'], "You are refund ops for [YOUR BRAND]. Output: Eligible? (yes/no/partial) | Reason code | Policy citation | Escalate? | Suggested workflow.", ['Policy citation', 'Reason code', 'Escalation flag', 'No customer draft yet']),
      workflow('save-offer-before-refund', 'Save offer before refund', 'Offer alternative to full refund.', ['Issue', 'Customer value tier', 'Allowed saves (credit, call, fix)'], "Draft save offer message: acknowledge, propose one alternative (store credit, replacement, extended access), deadline to accept. If decline, full refund per policy.", ['One save offer', 'Deadline', 'Fallback refund noted', 'Tier-aware']),
      workflow('full-refund-approval-letter', 'Full refund letter', 'Customer message approving full refund.', ['Order details', 'Refund amount', 'Processing time', 'Access revocation if digital'], "Draft approval message with amount, timeline, what happens to access/files, apology for poor experience if appropriate.", ['Amount and timeline', 'Digital access note', 'Professional tone', 'No extra promises']),
      workflow('partial-refund-offer', 'Partial refund offer', 'Negotiate partial refund fairly.', ['Original price', 'Issue severity', 'Max partial % allowed'], "Draft offer explaining partial amount rationale, math shown, accept/decline instructions, remain professional if they decline.", ['Math shown', 'Accept/decline path', 'Within max %', 'Respectful']),
      workflow('non-refundable-decline-letter', 'Decline with empathy', 'Decline refund outside policy.', ['Policy', 'Facts', 'Alternative (support, not $)'], "Draft firm but kind decline citing policy, offer non-monetary help, escalation path to owner if repeat customer.", ['Policy cited', 'Non-monetary help', 'No shaming', 'Escalation path']),
      workflow('internal-decision-log-entry', 'Internal decision log', 'Document decision for finance.', ['Case summary', 'Outcome', 'SKU', 'Margin notes'], "Write log row narrative: Customer | Order | Issue | Decision | $ impact | Who approved | Follow-up.", ['All fields', 'Approver placeholder', '$ impact', 'Concise']),
      workflow('payment-dispute-evidence-pack', 'Dispute evidence pack', 'Compile chargeback evidence outline.', ['Transaction', 'Delivery proof', 'Policy', 'Comms summary'], "Outline evidence packet sections for processor: timeline, TOS acceptance, delivery, prior comms, suggested exhibit list. Not legal advice.", ['Timeline', 'Exhibit list', 'Delivery proof', 'Disclaimer']),
      workflow('chargeback-urgent-response', 'Chargeback urgent response', 'Immediate plan when chargeback filed.', ['Dispute reason code', 'Deadline', 'Known facts'], "Output hour-by-hour plan T+0 to deadline: gather docs (workflow 07), draft customer optional message, owner tasks. Flag do-not refund duplicate.", ['Deadline noted', 'Task list', 'No duplicate refund', 'Reason code']),
      workflow('subscription-cancel-retention', 'Subscription cancel save', 'Handle cancel requests for subscriptions.', ['Plan', 'Usage data', 'Retention offers allowed'], "Draft response: acknowledge cancel, offer pause/downgrade if allowed, confirm effective date, win-back optional. Respect click-to-cancel laws [CUSTOMIZE].", ['Effective date', 'Pause option if allowed', 'Compliant', 'Clear confirm']),
      workflow('refund-comms-qa', 'Refund communication QA', 'QA refund/dispute messages.', ['Draft', 'Policy', 'Amounts'], "Verify amounts match policy, tone calm, no admitting legal liability beyond policy, clear next steps. Score and rewrite.", ['Amounts verified', 'Score', 'Rewrite if needed', 'Next steps']),
    ],
  },
  {
    slug: 'agent-15-faq-help-center-builder-agent-kit',
    readme: "# FAQ & Help Center Builder Agent Kit — [YOUR BRAND]\n\nTurn support tickets into searchable FAQs, help articles, and macros that deflect repeat questions.\n\n10 workflows + playbook + setup. Replace `[YOUR BRAND]`.",
    profile: "# Agent profile — FAQ & Help Center Builder\n\n**Role:** Knowledge base architect for [YOUR BRAND].\n\n**Outcome:** Structured help center content that reduces ticket volume and speeds self-serve.\n\n**Inputs:** Tickets, product docs, policies, voice guide.\n\n**Outputs:** FAQ sets, article drafts, category map, search keywords, update changelog.\n\n**KPIs:** Ticket deflection, article views, search success, time on page, repeat question rate.",
    playbook: "# FAQ & Help Center — Playbook (SOP)\n\n## Purpose\nConvert [YOUR BRAND] support pain into durable help assets.\n\n## Biweekly (90 min)\nExport top tickets → workflow 01 cluster → 02–04 draft articles → 05 publish checklist → link in macros.\n\n## Structure\nCategories align to buyer journey: Getting started, Billing, Shipping, Troubleshooting, Account.\n\n## Freshness\nWorkflow 09 quarterly audit; workflow 10 when product ships.\n\n## SEO\nHelp articles double as SEO for SaaS/course sites—workflow 06.",
    setup: "# 15-minute setup — FAQ & Help Center\n\n**0–4 min:** Pick help tool (Notion, Intercom, GitBook, Etsy FAQ section).\n\n**4–8 min:** Export 30 recent support tickets or top questions.\n\n**8–12 min:** Run `01-ticket-to-faq-cluster`.\n\n**12–15 min:** Create category folders matching playbook.\n\nDraft first article with workflow 03.",
    workflows: [
      workflow('ticket-to-faq-cluster', 'Ticket to FAQ clusters', 'Cluster tickets into FAQ candidates.', ['30 ticket subjects/bodies', 'Product areas'], "Cluster into 8-12 themes. Each: theme name | ticket count | proposed FAQ title | priority | related workflow.", ['8-12 themes', 'Counts', 'Priority', 'FAQ titles']),
      workflow('faq-10-pack-generator', 'FAQ 10-pack', 'Write 10 concise FAQs for one theme.', ['Theme', 'Policy facts', 'Voice'], "Write 10 Q&As: question as buyer would ask, answer ≤120 words, link placeholders [ARTICLE]. Mark internal-only if sensitive.", ['10 Q&As', '≤120 words each', 'Buyer phrasing', 'Link placeholders']),
      workflow('help-article-longform', 'Help article longform', 'Full help article with steps and screenshots notes.', ['Topic', 'Audience skill', 'Product version'], "Article: title, intro, prerequisites, numbered steps, troubleshooting, related links. Note [SCREENSHOT: ...] per step.", ['Numbered steps', 'Screenshot notes', 'Troubleshooting', 'Related links']),
      workflow('getting-started-guide', 'Getting started guide', 'Onboarding guide for new customers.', ['Product type', 'First success milestone', 'Common blockers'], "Write Getting Started guide: 5-minute quick start, checklist, links to deeper articles, when to contact support.", ['Quick start', 'Checklist', 'Support boundary', 'Milestone defined']),
      workflow('publish-checklist-help-article', 'Publish checklist', 'QA before publishing help content.', ['Draft article', 'Links', 'Product version'], "Checklist: accuracy, version, links, tone, accessibility (headings), search title, meta description 155 chars.", ['All checklist items', 'Meta description', 'Version tag', 'Pass/fail']),
      workflow('help-center-seo-metadata', 'Help SEO metadata', 'SEO titles and descriptions for help pages.', ['Article draft', 'Primary keyword', 'Site URL pattern'], "Provide SEO title ≤60 chars, meta description ≤155, URL slug, 5 related internal links.", ['Char limits', 'Slug', '5 internal links', 'Keyword natural']),
      workflow('category-navigation-map', 'Category navigation map', 'Organize help center IA.', ['List of articles', 'User journeys'], "Propose category tree max 2 levels, ordering logic, featured articles for onboarding.", ['2 levels max', 'Every article placed', 'Featured list', 'Journey logic']),
      workflow('macro-to-article-promotion', 'Macro to article', 'Expand best macro into full article.', ['Macro text', 'Ticket volume'], "Expand macro into article; keep macro as short version linking to article.", ['Article + short macro', 'Link between', 'Volume cited', 'No duplication waste']),
      workflow('help-center-audit-stale', 'Stale content audit', 'Find outdated help articles.', ['Article list with dates', 'Changelog since'], "Flag stale articles, suggested updates, merge candidates, archive list.", ['Stale flags', 'Update suggestions', 'Merge candidates', 'Archive list']),
      workflow('release-notes-to-help-update', 'Release notes to help', 'Update docs after product release.', ['Release notes', 'Affected articles'], "Map release to article updates: which to edit, new FAQs, deprecation notices, customer email blurb.", ['Article map', 'New FAQs', 'Deprecation', 'Email blurb']),
    ],
  },
  {
    slug: 'agent-16-churn-retention-save-agent-kit',
    readme: "# Churn & Retention Save Agent Kit — [YOUR BRAND]\n\nSave cancelling customers, win back churned accounts, and run retention campaigns with ethical offers.\n\n10 workflows + playbook + 15-minute setup. Use `[YOUR BRAND]` throughout.",
    profile: "# Agent profile — Churn & Retention Save\n\n**Role:** Retention marketer for [YOUR BRAND].\n\n**Outcome:** Structured save paths before cancel, respectful exits, and win-back sequences that recover LTV.\n\n**Inputs:** Cancel reason, customer tenure, plan, usage, offer guardrails.\n\n**Outputs:** Save emails, call scripts, offer terms, exit surveys, win-back campaigns.\n\n**KPIs:** Save rate, churn rate, win-back revenue, NPS of cancelled users, offer margin impact.",
    playbook: "# Churn & Retention — Playbook (SOP)\n\n## Purpose\nReduce preventable churn for [YOUR BRAND] without training customers to threaten cancel for discounts.\n\n## Cancel flow\nIntercept → workflow 01 reason → 02 save offer if fit → 03 confirm cancel → 04 exit survey.\n\n## Win-back\nWorkflow 07 at 30/60/90 days post-churn; cap discounts.\n\n## Ethics\nHonest offers only; easy cancel always available.\n\n## Review monthly\nWorkflow 09 on reason trends; feed product roadmap.",
    setup: "# 15-minute setup — Churn & Retention\n\n**0–4 min:** Document allowed save offers (pause, % off, downgrade).\n\n**4–8 min:** Export last 20 cancel reasons from billing tool.\n\n**8–12 min:** Run `01-cancel-reason-analyzer`.\n\n**12–15 min:** Add save templates to ESP/billing cancel flow.\n\nTest workflow 02 on a sample reason.",
    workflows: [
      workflow('cancel-reason-analyzer', 'Cancel reason analyzer', 'Structure cancel feedback into actions.', ['Cancel reason text', 'Tenure', 'Plan', 'Usage'], "Categorize reason: price, fit, support, missing feature, temporary, competitor. Recommend save playbook (02-05) or honor cancel. Product insight bullet.", ['Category', 'Playbook', 'Product insight', 'Honor cancel flag']),
      workflow('retention-save-email-offer', 'Retention save email', 'Personalized save offer email.', ['Reason category', 'Allowed offers', 'Customer name'], "Write save email: acknowledge reason, one tailored offer, deadline, easy cancel link anyway. No guilt trip.", ['One offer', 'Cancel link', 'Deadline', 'No guilt']),
      workflow('pause-instead-of-cancel', 'Pause subscription offer', 'Offer pause for temporary cancels.', ['Pause months allowed', 'Policy'], "Draft pause offer explaining billing hold, what access remains, restart date picker instructions.", ['Pause terms clear', 'Access rules', 'Restart steps', 'Policy accurate']),
      workflow('downgrade-path-message', 'Downgrade path', 'Offer lower tier instead of cancel.', ['Current tier', 'Lower tier features', 'Price delta'], "Compare tiers honestly; recommend downgrade if fit issue; CTA to switch plan.", ['Honest comparison', 'Price delta', 'CTA', 'No fear tactics']),
      workflow('founder-save-call-script', 'Founder save call script', 'Short call script for high-value saves.', ['Account value', 'Reason', 'Max concession'], "5-minute call outline: open, listen, recap, offer, next step. Bullet talk track + what not to promise.", ['5-min outline', 'Listen phase', 'Max concession', 'Talk track']),
      workflow('graceful-exit-email', 'Graceful exit email', 'Confirm cancel respectfully.', ['End date', 'Data export', 'Door open'], "Confirm cancellation, thank them, export/data steps, optional feedback link, welcome back anytime.", ['End date', 'Export steps', 'Feedback link', 'Respectful']),
      workflow('win-back-30-60-90', 'Win-back sequence', '3-email win-back after churn.', ['Product updates since leave', 'Offer policy', 'Segment'], "3 emails days 30/60/90: what's new, case study, final offer if allowed. Subjects x3 each.", ['3 emails', 'Days noted', 'Honest updates', 'Offer policy']),
      workflow('at-risk-usage-alert', 'At-risk usage alert', 'Message when usage drops before cancel.', ['Usage trend', 'Segment', 'Help resources'], "Proactive email offering help before cancel intent. Tutorial links, offer call if high-touch.", ['Usage referenced', 'Help links', 'Proactive tone', 'No spam']),
      workflow('churn-reason-dashboard-narrative', 'Churn dashboard narrative', 'Monthly churn narrative for leadership.', ['Reason counts', 'Save rate', 'Revenue churn'], "≤400 word exec summary: top reasons, saves, recommendations for product/pricing/support.", ['≤400 words', 'Top 3 reasons', 'Recommendations', 'Numbers from input']),
      workflow('retention-offer-qa', 'Retention offer QA', 'QA save/win-back for margin and ethics.', ['Draft', 'Offer limits', 'Legal notes'], "Check offer within limits, no dark patterns, cancel still easy, margin note estimate.", ['Within limits', 'Ethics check', 'Margin note', 'Rewrite if fail']),
    ],
  },
  {
    slug: 'agent-17-operations-sop-writer-agent-kit',
    readme: "# Operations SOP Writer Agent Kit — [YOUR BRAND]\n\nDocument repeatable processes, handoffs, and checklists so [YOUR BRAND] scales without tribal knowledge.\n\n10 workflows + SOP playbook + quick setup.",
    profile: "# Agent profile — Operations SOP Writer\n\n**Role:** Operations documentation lead for [YOUR BRAND].\n\n**Outcome:** Clear SOPs with owners, triggers, steps, and quality checks.\n\n**Inputs:** Process description, tools, roles, failure modes.\n\n**Outputs:** SOP docs, checklists, RACI snippets, training outlines.\n\n**KPIs:** Time to train new hire, error rate on process, SOP usage, audit pass rate.",
    playbook: "# Operations SOP — Playbook (SOP)\n\n## Purpose\nCapture how [YOUR BRAND] actually works—not ideal fantasy docs.\n\n## When to write\nNew recurring task 3+ times → workflow 01 charter → 02 draft → 03 checklist → 04 train.\n\n## Maintenance\nQuarterly workflow 09 review. Version every change.\n\n## Storage\n`/ops/sops/` with naming `SOP-###-slug-v1.md`.\n\n## Quality\nOne owner per SOP; steps testable by new team member.",
    setup: "# 15-minute setup — Operations SOP Writer\n\n**0–4 min:** Create `/ops/sops/` folder and index spreadsheet.\n\n**4–8 min:** List top 5 processes only you know how to do.\n\n**8–12 min:** Run `01-process-charter` on #1.\n\n**12–15 min:** Schedule monthly SOP review 30 min.\n\nComplete workflow 02 for first process.",
    workflows: [
      workflow('process-charter', 'Process charter', 'Define scope and owner before writing SOP.', ['Process name', 'Goal', 'Pain today', 'Frequency'], "Charter: purpose, scope in/out, owner, trigger, frequency, tools, success metric, risks if not documented.", ['Owner named', 'Scope boundaries', 'Metric', 'Trigger']),
      workflow('sop-step-by-step-draft', 'SOP step-by-step', 'Full SOP with numbered steps.', ['Charter', 'Interview notes', 'Tools'], "Write SOP sections: Purpose | Roles | Prerequisites | Steps 1-n | Exceptions | QC | Revision log table starter.", ['Numbered steps', 'Exceptions', 'QC section', 'Roles']),
      workflow('one-page-checklist-extract', 'One-page checklist', 'Extract field checklist from SOP.', ['Full SOP'], "Single-page checklist with checkboxes, owner initials, date. No prose paragraphs.", ['Checkbox format', 'Date/initials', 'One page', 'Matches SOP']),
      workflow('raci-for-handoff', 'RACI handoff matrix', 'Clarify roles on cross-team process.', ['Teams involved', 'Steps'], "RACI table per step or deliverable. Flag gaps where A missing.", ['RACI table', 'Gaps flagged', 'Deliverables clear', 'Teams listed']),
      workflow('new-hire-training-outline', 'New hire training', 'Training plan from SOP set.', ['Role', 'SOPs list', 'Week 1 goals'], "Day-by-day week 1 training: read which SOP, shadow task, solo with QC, quiz questions 5.", ['Week 1 days', 'SOP links', 'Quiz 5', 'Shadow noted']),
      workflow('exception-escalation-tree', 'Exception escalation tree', 'Decision tree for when things go wrong.', ['Process', 'Known failures'], "Mermaid or indented tree: if X then Y else escalate to Z. Contact list placeholders.", ['Tree present', 'Escalation contacts', 'Common failures', 'Readable']),
      workflow('tool-stack-step-links', 'Tool stack links', 'Add deep links and screenshots list to SOP.', ['Tools per step', 'SOP draft'], "Annotate SOP with [TOOL: name] links and screenshot inventory list for designer.", ['Tool tags', 'Screenshot list', 'Per step', 'No broken logic']),
      workflow('sop-to-automation-brief', 'SOP to automation brief', 'Brief for Zapier/n8n automation.', ['SOP', 'Volume', 'Error cost'], "Automation brief: trigger, systems, fields, happy path, error handling, human approval step.", ['Trigger', 'Error handling', 'Approval step', 'Systems listed']),
      workflow('sop-audit-freshness', 'SOP audit', 'Audit SOP library for stale docs.', ['SOP index with dates', 'Org changes'], "Rate each SOP red/yellow/green; update priority queue; merge duplicates.", ['R/Y/G', 'Priority queue', 'Duplicates', 'Dates']),
      workflow('meeting-to-sop-update', 'Meeting to SOP update', 'Turn meeting decisions into SOP patches.', ['Meeting notes', 'Affected SOP'], "List precise SOP edits: section, old → new, version bump, comms to team.", ['Section-level edits', 'Version bump', 'Team comms', 'No vague notes']),
    ],
  },
  {
    slug: 'agent-18-va-delegation-brief-agent-kit',
    readme: "# VA Delegation Brief Agent Kit — [YOUR BRAND]\n\nWrite crystal-clear tasks for VAs, contractors, and freelancers—context, standards, and acceptance criteria included.\n\n10 workflows + playbook + 15-minute setup. Replace `[YOUR BRAND]`.",
    profile: "# Agent profile — VA Delegation Brief\n\n**Role:** Delegation and task design lead for [YOUR BRAND].\n\n**Outcome:** Tasks completed right the first time with minimal back-and-forth.\n\n**Inputs:** Goal, deadline, tools access, examples, constraints.\n\n**Outputs:** Task briefs, Loom scripts, QA rubrics, batch schedules.\n\n**KPIs:** Rework rate, time to complete, on-time %, VA satisfaction, cost per outcome.",
    playbook: "# VA Delegation — Playbook (SOP)\n\n## Purpose\nMake delegation repeatable for [YOUR BRAND]—not one-off Slack dumps.\n\n## Brief anatomy\nOutcome | Context | Steps | Examples | Do-not | Deadline | QA rubric.\n\n## Weekly\nMonday workflow 08 batch plan; Friday workflow 09 retro with VA.\n\n## Tools\nUse Notion/Asana/ClickUp; paste briefs from workflows.\n\n## Security\nWorkflow 10 for access-sensitive tasks.",
    setup: "# 15-minute setup — VA Delegation\n\n**0–4 min:** List VA tools and access they have.\n\n**4–8 min:** Collect 2 examples of great and bad past tasks.\n\n**8–12 min:** Run `01-task-outcome-framer` on next delegation.\n\n**12–15 min:** Create task template in PM tool.\n\nSend first brief using workflow 02.",
    workflows: [
      workflow('task-outcome-framer', 'Task outcome framer', 'Turn vague idea into measurable outcome.', ['Vague request', 'Why now', 'Deadline'], "Rewrite as outcome statement: verb + deliverable + format + done definition + deadline. Flag missing info questions (max 3).", ['Measurable outcome', 'Done definition', '≤3 questions', 'Deadline']),
      workflow('va-task-brief-full', 'Full VA task brief', 'Complete brief ready to assign.', ['Outcome framer output', 'Context links', 'Brand rules'], "Full brief sections: Title | Outcome | Background | Steps | Examples | Assets | Do-not | QA | Deadline | Time estimate.", ['All sections', 'Do-not list', 'QA rubric', 'Time estimate']),
      workflow('loom-training-script', 'Loom training script', 'Script for 3–5 min Loom walkthrough.', ['Task brief', 'Screens to show'], "Shot-by-shot Loom script with click path, what to say, common mistakes to call out.", ['Shot list', 'Mistakes noted', '3-5 min', 'Click path']),
      workflow('recurring-task-sop-lite', 'Recurring task SOP lite', 'Light SOP for weekly VA task.', ['Task name', 'Frequency', 'Checklist'], "SOP lite ≤1 page: trigger day, checklist, where to save output, escalation.", ['≤1 page', 'Checklist', 'Save location', 'Frequency']),
      workflow('batch-similar-tasks', 'Batch similar tasks', 'Group tasks for efficient VA batching.', ['Task list', 'VA hours'], "Group into batches by tool/context; ordered schedule; handoff notes between batches.", ['Batches logical', 'Schedule', 'Handoff notes', 'Hours fit']),
      workflow('qa-rubric-for-deliverable', 'QA rubric', 'Scoring rubric for VA deliverable.', ['Deliverable type', 'Standards'], "Rubric 5 criteria weighted, 1-5 scale, examples of 3 vs 5, pass threshold.", ['5 criteria', 'Weights', 'Pass threshold', 'Examples']),
      workflow('revision-request-message', 'Revision request', 'Kind, specific revision request to VA.', ['Deliverable issues', 'Brief'], "Bullet revisions only, reference brief sections, deadline for v2, appreciate effort.", ['Bullets specific', 'Brief refs', 'v2 deadline', 'Respectful']),
      workflow('weekly-va-priority-stack', 'Weekly VA priority stack', 'Rank week tasks for VA.', ['Backlog', 'Business priorities', 'Hours'], "Ranked stack top 10 with P1-P3, hours each, dependencies, drop list if over capacity.", ['Top 10 ranked', 'Hours', 'Dependencies', 'Drop list']),
      workflow('va-weekly-retro-questions', 'VA weekly retro', 'Retro questions and summary template.', ['Week tasks', 'Issues'], "5 retro questions for async VA retro + owner action items template.", ['5 questions', 'Action template', 'Async friendly', 'Issues addressed']),
      workflow('sensitive-data-task-guard', 'Sensitive data guard', 'Brief addendum for PII/finance tasks.', ['Task', 'Data types', 'Compliance'], "Security addendum: data allowed, redaction rules, delete after, no local storage, breach escalation. Not legal advice.", ['Data rules', 'Delete after', 'Escalation', 'Disclaimer']),
    ],
  },
  {
    slug: 'agent-19-group-program-community-agent-kit',
    readme: "# Group Program & Community Agent Kit — [YOUR BRAND]\n\nRun cohort prompts, community posts, office hours agendas, and member engagement for courses and group coaching.\n\n10 workflows + playbook + setup. Customize `[YOUR BRAND]`.",
    profile: "# Agent profile — Group Program & Community\n\n**Role:** Community and cohort facilitator for [YOUR BRAND].\n\n**Outcome:** Engaged members, clear weekly rhythm, and facilitator-ready session plans.\n\n**Inputs:** Program curriculum, member personas, platform (Circle, Slack, Skool), norms.\n\n**Outputs:** Discussion prompts, live session agendas, recap posts, engagement nudges, moderation replies.\n\n**KPIs:** Active members %, session attendance, completion rate, NPS, support tickets from confusion.",
    playbook: "# Group Program & Community — Playbook (SOP)\n\n## Purpose\nKeep [YOUR BRAND] cohort energy high without burning out the facilitator.\n\n## Weekly cadence\nMon workflow 01 theme → Tue member prompt → Wed resource → Thu accountability → Fri wins thread → live session workflow 04.\n\n## Moderation\nWorkflow 08 for conflict; workflow 09 boundaries.\n\n## Onboarding\nWorkflow 02 day 0-7 sequence for new members.\n\n## Offboarding\nWorkflow 10 alumni transition.",
    setup: "# 15-minute setup — Group Program & Community\n\n**0–4 min:** Paste community rules and program promise.\n\n**4–8 min:** Map module/week schedule.\n\n**8–12 min:** Run `01-weekly-community-theme`.\n\n**12–15 min:** Schedule posts in community tool.\n\nPrep next live with workflow 04.",
    workflows: [
      workflow('weekly-community-theme', 'Weekly community theme', 'Plan one week of community engagement.', ['Module/week topic', 'Member struggles', 'Live session date'], "7-day plan: daily post type, prompt, CTA, facilitator time estimate. Tie to curriculum outcomes.", ['7 days', 'Prompts each', 'Time estimate', 'Outcome tie']),
      workflow('new-member-onboarding-dms', 'New member onboarding', 'Day 0-7 welcome message sequence.', ['Program name', 'First action', 'Links'], "7 short messages or posts: welcome, how to introduce, where to ask questions, first win, live session prep, buddy optional, week 1 checkpoint.", ['7 touchpoints', 'First action clear', 'Links', 'Welcoming']),
      workflow('discussion-prompt-generator', 'Discussion prompts', '10 discussion prompts for one module.', ['Lesson summary', 'Audience', 'Avoid topics'], "10 prompts: reflection, application, hot take, show-your-work. Tag difficulty. No homework overload.", ['10 prompts', 'Variety', 'Avoid list', 'Application focus']),
      workflow('live-session-agenda-facilitator', 'Live session agenda', '60-minute session run-of-show.', ['Learning objective', 'Content outline', 'Q&A time'], "Agenda minute-by-minute, slides/topics, breakout optional, homework, follow-up email bullet.", ['Minute-by-minute', 'Objective', 'Q&A block', 'Follow-up']),
      workflow('session-recap-post', 'Session recap post', 'Recap for members who missed live.', ['Notes/recording link', 'Key takeaways'], "Recap post: 3 takeaways, action item, recording link placeholder, next session date, comment prompt.", ['3 takeaways', 'Action item', 'Recording placeholder', 'Comment prompt']),
      workflow('accountability-thread-prompt', 'Accountability thread', 'Weekly accountability post template.', ['Week goal theme', 'Member level'], "Thread starter: share goal, obstacle, commitment for week. Facilitator example answer. Pin instructions.", ['Thread starter', 'Example answer', 'Pin note', 'Theme tied']),
      workflow('member-win-celebration-post', 'Member win celebration', 'Celebrate wins without cringe.', ['Win description', 'Permission'], "Celebration post template tagging member [NAME], specific win, lesson for group, invite others to share. Opt-out note.", ['Specific win', 'Permission note', 'Invite others', 'Not cringe']),
      workflow('community-conflict-moderation', 'Conflict moderation', 'Moderate heated thread.', ['Thread paste', 'Community rules'], "Public moderation comment + optional DM to parties. De-escalate, cite rule, next step (cool-off, office hours).", ['Public + DM', 'Rule cited', 'De-escalate', 'Next step']),
      workflow('boundary-off-topic-redirect', 'Off-topic redirect', 'Redirect without shaming.', ['Off-topic post', 'Relevant channel'], "Friendly redirect message with link to right space, reaffirm welcome.", ['Friendly', 'Link to space', 'No shame', 'Brief']),
      workflow('alumni-transition-plan', 'Alumni transition', 'Move graduates to alumni community.', ['Program end', 'Offers'], "Alumni email + post: what's next, alumni perks, upsell if appropriate, feedback survey link, stay connected CTA.", ['Next steps', 'Survey', 'Perks', 'Upsell honest']),
    ],
  },
  {
    slug: 'agent-20-coach-consultant-professional-agent-kit',
    readme: `# Coach & Consultant Professional Agent Kit — [YOUR BRAND]

Professional coaching and consulting workflows: discovery, sessions, recaps, proposals, and thought leadership—without generic life-coach fluff.

## What you get
- **AGENT-PROFILE.md** — role, inputs, outputs, KPIs
- **01-playbook/** — weekly client delivery rhythm
- **02-workflows/** — 10 agent briefs (discovery → delivery → renewal)
- **05-implementation/** — 15-minute setup

## Who this is for
Business coaches, consultants, strategists, and fractional executives productizing expertise.

## Quick start
1. Open \`05-implementation/setup-guide.md\`
2. Run **01-icp-offer-clarity-pass** on your niche
3. Prep next client call with **04-session-prep-agenda**
4. Send **05-session-recap-actions** within 24h of every session

Replace \`[YOUR BRAND]\` and never present AI output as licensed therapy or legal/financial advice.
`,
    profile: `# Agent profile — Coach & Consultant Professional

**Role:** Executive coach and consulting partner for [YOUR BRAND].

**Outcome:** Clients get clarity, committed actions, and measurable progress between sessions—with professional boundaries.

**Inputs:** Client context, session notes, offer scope, frameworks you use, ethics rules.

**Outputs:** Session prep, agendas, recaps, homework, insight summaries, renewal proposals, content from expertise.

**KPIs:** Client retention, goal completion %, referrals, revenue per client, session NPS, days to recap sent.

**Guardrails:** No medical/mental health treatment claims; refer out for crisis; confidentiality; scope boundaries vs therapy/legal/financial advice.
`,
    playbook: `# Coach & Consultant Professional — Playbook (SOP)

## Purpose
Run a premium delivery loop for [YOUR BRAND] coaching/consulting clients with consistent session quality and business outcomes.

## Roles
- **Practitioner:** leads sessions, approves all client-facing AI drafts.
- **AI partner:** prep, recaps, frameworks via \`02-workflows/\`.

## Weekly per active client (≈45 min non-session)
**Prep (15 min):** Workflow 04 with last recap + client updates.

**Session:** Use agenda; capture raw notes.

**Recap (20 min):** Workflow 05 within 24 hours.

**Content (10 min optional):** Workflow 09 mine anonymized insight.

## Sales loop
Discovery workflow 02 → proposal workflow 07 → contract reminder in onboarding.

## Renewal
Workflow 08 at 30 days before package end.

## Ethics
Crisis protocol: if client expresses self-harm, violence, or abuse—do not coach; provide crisis resources [CUSTOMIZE] and pause engagement.

## Quality bar
Recaps must have ≤5 actions with owners and dates. No vague "think about" homework.
`,
    setup: `# 15-minute setup — Coach & Consultant Professional

## Minutes 0–3: Offer clarity
Document **[YOUR BRAND] offer**: who, transformation, duration, price, what's out of scope.

## Minutes 3–6: Frameworks
List 3 frameworks you actually use (e.g., OKRs, Eisenhower, value equation).

## Minutes 6–10: AI context
Paste AGENT-PROFILE + ethics + ICP into AI project instructions.

## Minutes 10–13: ICP pass
Run \`02-workflows/01-icp-offer-clarity-pass.md\`.

## Minutes 13–15: Next session
Schedule workflow 04 for next client on calendar with 15-min prep block.

You are ready. After next session, run workflow 05 before end of day.
`,
    workflows: [
      workflow(
        'icp-offer-clarity-pass',
        'ICP & offer clarity pass',
        'Sharpen ideal client and offer positioning.',
        [
          'Current offer description',
          'Best client examples (3)',
          'Nightmare clients to avoid',
          'Price and format',
        ],
        `You are positioning strategist for [YOUR BRAND] coaching/consulting practice. Refine **ICP** and **offer**.

Deliver: ICP paragraph | Anti-ICP red flags | Offer one-liner | 3 outcomes (measurable) | 3 objections + responses | Messaging do/don't.

Be specific to B2B/professional services unless inputs say otherwise. No income guarantees.`,
        [
          'ICP and anti-ICP',
          'Measurable outcomes',
          '3 objections handled',
          'No income guarantees',
        ],
      ),
      workflow(
        'discovery-call-guide',
        'Discovery call guide',
        'Custom discovery script for coaching/consulting sale.',
        [
          'Offer summary',
          'Call length',
          'Qualification criteria',
          'Price range',
        ],
        `You are sales discovery coach for [YOUR BRAND]. Write **discovery guide**: opening, permission, situation questions, pain/impact, fit check, next step close.

Include red flag answers that mean disqualify. Time-box sections for call length.`,
        [
          'Time-boxed sections',
          'Red flags listed',
          'Next step close',
          'Fit check questions',
        ],
      ),
      workflow(
        'coaching-intake-questionnaire',
        'Coaching intake questionnaire',
        'Pre-session intake for new clients.',
        [
          'Program length',
          'Goals client stated',
          'Tools they use',
          'Confidentiality note',
        ],
        `You are client onboarding designer for [YOUR BRAND]. Create **intake questionnaire** (12-15 questions): context, goals, metrics, stakeholders, constraints, communication prefs, success definition.

Add facilitator note on how to use answers in session 1.`,
        [
          '12-15 questions',
          'Success definition',
          'Facilitator note',
          'Confidentiality mention',
        ],
      ),
      workflow(
        'session-prep-agenda',
        'Session prep & agenda',
        'Prep doc and agenda for upcoming client session.',
        [
          'Last recap actions status',
          'Client updates since last time',
          'Session length',
          'Primary objective',
        ],
        `You are executive coach for [YOUR BRAND]. Build **prep brief** for practitioner eyes only: context, hypothesis, agenda timed to session length, powerful questions (5), watch-outs, desired outcome.

End with **opening script** 30 seconds.`,
        [
          'Timed agenda',
          '5 powerful questions',
          'Opening script',
          'Practitioner-only tone',
        ],
      ),
      workflow(
        'session-recap-actions',
        'Session recap & actions',
        'Client-facing recap with committed actions.',
        [
          'Raw session notes',
          'Client name',
          'Date',
          'Tone (direct/warm)',
        ],
        `You are coaching chief of staff for [YOUR BRAND]. Write **client recap email**: themes discussed, insights (no therapy speak), **≤5 actions** with owner and due date, resources, next session confirm.

Do not invent commitments not in notes—mark [CONFIRM] if unclear.`,
        [
          '≤5 actions with dates',
          'No invented commitments',
          'Client-facing tone',
          'Next session noted',
        ],
      ),
      workflow(
        'between-session-accountability-nudge',
        'Between-session nudge',
        'Short accountability check-in between sessions.',
        [
          'Actions from recap',
          'Days since session',
          'Client preference (email/Slack)',
        ],
        `Draft brief accountability message referencing specific actions, ask for status, offer office hours slot if blocked. ≤100 words.`,
        [
          'References specific actions',
          '≤100 words',
          'Blocker offer',
          'Respectful tone',
        ],
      ),
      workflow(
        'consulting-proposal-from-discovery',
        'Consulting proposal',
        'Proposal after discovery for consulting engagement.',
        [
          'Discovery summary',
          'Scope',
          'Timeline',
          'Investment',
          'Case proof',
        ],
        `You are proposal writer for [YOUR BRAND]. Write consulting proposal: executive summary, situation, approach phases, deliverables table, timeline, investment, assumptions, next steps.

Mark [CUSTOMIZE] for legal terms. Not legal advice.`,
        [
          'Deliverables table',
          'Phases',
          'Investment section',
          'Assumptions',
        ],
      ),
      workflow(
        'package-renewal-conversation',
        'Package renewal conversation',
        'Renewal email and talk track before package ends.',
        [
          'Results achieved',
          'Remaining goals',
          'Renewal options',
          'End date',
        ],
        `Draft renewal email celebrating specific wins from inputs, gap to future goals, 2 renewal options, call CTA. Talk track bullets for live renewal conversation.`,
        [
          'Specific wins cited',
          '2 renewal options',
          'Email + talk track',
          'CTA to call',
        ],
      ),
      workflow(
        'thought-leadership-from-session-themes',
        'Thought leadership from themes',
        'Turn anonymized session themes into content.',
        [
          'Themes from recent sessions (no names)',
          'Platform',
          'Voice',
        ],
        `Create LinkedIn-style post or newsletter outline teaching one lesson from anonymized patterns. No client identification. One CTA to [YOUR BRAND] offer soft.`,
        [
          'Anonymized',
          'One clear lesson',
          'Soft CTA',
          'Platform appropriate',
        ],
      ),
      workflow(
        'professional-ethics-boundary-check',
        'Ethics & boundary check',
        'Review client-facing draft for scope and ethics.',
        [
          'Draft text',
          'Scope of practice',
          'Crisis indicators (yes/no)',
        ],
        `You are ethics reviewer for [YOUR BRAND] (not a lawyer). Flag therapy/medical/legal/financial overreach, confidentiality risks, outcome guarantees, crisis mishandling.

If crisis=yes, output crisis referral template [CUSTOMIZE] and **do not send** coaching content. Otherwise provide cleaned draft.`,
        [
          'Overreach flagged',
          'Crisis protocol if needed',
          'No guarantees',
          'Cleaned draft or stop',
        ],
      ),
    ],
  },
];
