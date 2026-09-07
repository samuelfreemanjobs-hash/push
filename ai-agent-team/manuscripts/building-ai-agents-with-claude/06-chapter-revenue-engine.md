# Chapter 6: Deploying the Revenue Engine — Autonomous Lead Qualification and Pipeline Management

## The Lead That Died in Four Hours

At 2:14 PM on a Tuesday, Sarah Chen submitted an enterprise demo request on a B2B SaaS company's website. She was VP Operations at a 450-person manufacturer evaluating workflow automation tools. Budget approved. Timeline: Q4. She had also submitted the same form to two competitors that morning.

The company's SDR team saw the notification at 4:47 PM—buried under 23 other form submissions, 8 Slack messages, and a mandatory all-hands meeting. Sarah's record sat in HubSpot with three fields populated: name, email, company. No enrichment. No ICP score. No personalized outreach. No calendar link.

Their closest competitor responded at 2:18 PM. Personalized email referencing Acme Corp's recent Series C. Calendar link for Thursday. Enriched CRM record with tech stack, team size, and funding data.

Sarah booked the competitor's demo on Wednesday. The first company called her on Thursday. She was already in a sales cycle with someone else.

Speed-to-lead is not a sales tactic. It is a survival metric. Research across B2B SaaS consistently shows response within five minutes generates 4-8x higher conversion than response after one hour. After four hours, you are not competing for the deal—you are competing for leftover attention.

This chapter deploys the full agent architecture from Chapters 3-5 against your revenue engine: inbound triage in 60 seconds, autonomous calendar dispatch, cold outbound pipelines, and the complete SDR Agent field playbook with production prompt schemas.

---

## The 60-Second Inbound Lead Triage

Inbound lead triage is the highest-ROI agent deployment in most B2B companies. The workflow is well-defined, the data sources are API-accessible, the output is structured, and the cost of failure is measured in lost pipeline.

### The Triage Workflow

From webhook receipt to human-ready package in 60 seconds:

```text
  0s ─── Webhook received (HubSpot form submission)
  2s ─── Router classifies: inbound_lead, enterprise_demo, priority=high
  3s ─── Orchestrator spawns triage workflow
  5s ─── Enrichment worker: Clearbit + LinkedIn + CRM history lookup
 18s ─── Scoring worker: ICP match against criteria matrix
 25s ─── Outreach worker: personalized email draft
 35s ─── Evaluator: quality gate on email draft
 42s ─── CRM update: enriched fields, score, activity log
 45s ─── Calendar worker: availability check + booking link selection
 50s ─── Human review queue: triage package ready for SDR approval
 60s ─── SDR receives notification with one-click approve/send
```

The SDR's job shifts from *doing* the triage to *approving* the triage. One click to send the draft. Thirty seconds instead of thirty minutes.

### ICP Scoring Matrix

The scoring worker evaluates leads against explicit, weighted criteria—not vibes:

| Criterion | Weight | Score Logic |
|-----------|--------|-------------|
| Company size | 20% | 200-2000 employees = 1.0, 50-199 = 0.6, <50 or >2000 = 0.3 |
| Industry fit | 25% | Target verticals (manufacturing, logistics, distribution) = 1.0, adjacent = 0.5, other = 0.2 |
| Tech stack overlap | 15% | Uses Salesforce + ERP = 1.0, CRM only = 0.6, no detected stack = 0.3 |
| Funding/growth signals | 15% | Series B+ in last 18 months = 1.0, bootstrapped/profitable = 0.7, unknown = 0.4 |
| Engagement quality | 15% | Enterprise demo form = 1.0, pricing page visit + form = 0.8, newsletter = 0.2 |
| Existing relationship | 10% | Prior opportunity or customer = 1.0, prior activity = 0.5, net new = 0.3 |

**Tier assignment:**
- **Tier A** (score ≥ 80): Immediate outreach + calendar link + SDR notification
- **Tier B** (score 60-79): Outreach draft + SDR review within 4 hours
- **Tier C** (score 40-59): Nurture sequence enrollment, no direct outreach
- **Tier D** (score < 40): Archive with tag, quarterly re-evaluation

Encode this matrix in the scoring worker's system prompt as a deterministic rubric. The model applies judgment only where data is ambiguous—never where the rubric provides a clear answer.

### Enrichment Data Package

The enrichment worker assembles a standardized payload every downstream agent consumes:

```json
{
  "contact": {
    "name": "Sarah Chen",
    "email": "sarah.chen@acmecorp.com",
    "title": "VP Operations",
    "linkedin_url": "https://linkedin.com/in/sarahchen"
  },
  "company": {
    "name": "Acme Corp",
    "domain": "acmecorp.com",
    "employee_count": 450,
    "industry": "Manufacturing",
    "hq_location": "Chicago, IL",
    "funding_stage": "Series C",
    "funding_amount": "$45M",
    "funding_date": "2025-03-15",
    "tech_stack": ["Salesforce", "NetSuite", "Slack", "Microsoft 365"]
  },
  "crm_history": {
    "existing_contact": false,
    "existing_account": false,
    "prior_opportunities": 0,
    "prior_support_tickets": 0
  },
  "trigger_context": {
    "form_name": "Enterprise Demo Request",
    "utm_source": "linkedin",
    "utm_campaign": "manufacturing_q3",
    "submission_time": "2025-09-07T14:14:32Z"
  }
}
```

### Triage Output Package

What lands in the SDR's review queue:

```json
{
  "triage_id": "tri_20250907_001",
  "lead_tier": "A",
  "icp_score": 87,
  "scoring_rationale": "Strong fit: 450 employees in target vertical, Series C funding 6 months ago, uses Salesforce + NetSuite, enterprise demo form submission.",
  "recommended_action": "send_outreach_with_calendar",
  "email_draft": {
    "to": "sarah.chen@acmecorp.com",
    "subject": "Acme's ops automation timeline — quick intro",
    "body": "Sarah,\n\nSaw Acme closed Series C in March — congrats on the growth trajectory. When VP Ops teams at manufacturers your size start evaluating workflow automation, it usually maps to one of three triggers: scaling pain after funding, ERP integration bottlenecks, or manual processes that survived the growth phase but won't survive the next one.\n\nWe help ops teams at companies like [similar customer] cut manual processing time by 60%+ without ripping out their existing Salesforce/NetSuite stack.\n\nWorth 20 minutes this week? I have Thursday 2pm or Friday 10am open: [calendar_link]\n\n— [SDR name]"
  },
  "calendar_link": "https://calendly.com/sdr-team/enterprise-demo-20min",
  "crm_updates_applied": true,
  "evaluator_score": 0.86,
  "processing_time_seconds": 47
}
```

The SDR reviews, edits if needed, clicks send. Or clicks approve-and-send for zero-edit drafts that pass evaluator thresholds above 0.85.

### Metrics That Matter

Track these weekly. Ignore vanity metrics.

| Metric | Target | Why |
|--------|--------|-----|
| Time to triage package | < 60 seconds | Speed-to-lead survival |
| CRM completeness on inbound | > 90% fields populated | Data quality compounds |
| Tier A/B accuracy (human audit) | > 85% agreement | Scoring model calibration |
| SDR edit rate on approved drafts | < 30% | Generator + evaluator quality |
| Lead-to-meeting conversion (Tier A) | > 25% | Revenue impact |
| Cost per triaged lead | < $0.15 | Unit economics vs. $3+ manual |

---

## Autonomous Calendar Dispatch

Getting a personalized email in front of a lead in 60 seconds is half the battle. Getting them on a calendar is the other half. Calendar dispatch agents eliminate the back-and-forth scheduling tax that kills momentum between first touch and first meeting.

### Calendar Agent Architecture

The calendar worker operates as a specialized agent within the triage orchestrator—or as a standalone agent triggered after SDR approval:

**Tools required:**
- `calendar_list_availability` — query open slots for assigned SDR(s) within next 5 business days
- `calendar_create_hold` — place tentative hold pending lead confirmation
- `calendar_get_booking_link` — retrieve personalized Calendly/Cal.com link with pre-filled context
- `calendar_get_team_availability` — round-robin across SDR team for optimal slot distribution

### Availability Logic

Deterministic rules the calendar agent applies before selecting slots:

1. **Business hours only:** 9 AM - 5 PM in lead's detected timezone (from company HQ or email domain geolocation).
2. **Minimum lead time:** No slots within 4 hours (gives SDR prep time).
3. **Maximum lead time:** No slots beyond 10 business days (urgency decays).
4. **SDR load balancing:** Assign to SDR with fewest meetings booked this week among qualified team members.
5. **Tier-based duration:** Tier A leads get 30-minute enterprise demo slots. Tier B get 20-minute intro calls.

### Personalized Booking Links

Generic Calendly links signal generic outreach. The calendar agent generates links with pre-populated fields:

```
https://calendly.com/james-wilson/enterprise-demo-30
  ?name=Sarah+Chen
  &email=sarah.chen@acmecorp.com
  &a1=Acme+Corp
  &a2=VP+Operations
  &utm_source=agent_triage
  &utm_content=tri_20250907_001
```

When Sarah clicks, she sees a booking page with her name and company already filled. One click to select a time. Zero friction.

### Post-Booking Automation

When a lead books a meeting, a second webhook fires. Route to the meeting-prep orchestrator:

1. **CRM update:** Create meeting activity, advance lifecycle stage to "Meeting Scheduled."
2. **SDR briefing:** Generate pre-call brief with enrichment data, company news, tech stack, scoring rationale, and suggested discovery questions.
3. **Reminder sequence:** Schedule confirmation email (immediate), prep email to SDR (24 hours before), reminder to lead (1 hour before).
4. **No-show protocol:** If lead no-shows, trigger re-engagement sequence after 30 minutes.

### Calendar Dispatch Metrics

| Metric | Target |
|--------|--------|
| Email-to-booking conversion (Tier A) | > 15% |
| Average time from email to booked meeting | < 48 hours |
| SDR prep brief generation time | < 30 seconds |
| No-show re-engagement trigger time | < 5 minutes after missed meeting |

---

## The Cold Outbound Pipeline

Inbound triage captures demand. Cold outbound creates it. The agent architecture differs: higher volume, lower individual conversion, stricter compliance requirements, and a longer feedback loop between send and response.

### Outbound Workflow Architecture

```text
  ┌─────────────┐     ┌──────────────┐     ┌─────────────┐
  │  PROSPECT   │────▶│  RESEARCH    │────▶│  PERSONALIZE│
  │  LIST INGEST│     │  WORKER      │     │  WORKER     │
  └─────────────┘     └──────────────┘     └──────┬──────┘
                                                   │
                                                   ▼
  ┌─────────────┐     ┌──────────────┐     ┌─────────────┐
  │  CRM LOG +  │◀────│  HUMAN       │◀────│  EVALUATOR  │
  │  SEQUENCE   │     │  APPROVAL    │     │  GATE       │
  └─────────────┘     └──────────────┘     └─────────────┘
```

**Stage 1 — Prospect List Ingest:** Import target accounts from CSV, CRM segment, or intent data provider. Validate email deliverability. Deduplicate against existing CRM records and suppression lists.

**Stage 2 — Research Worker:** Per prospect, gather company news (last 90 days), LinkedIn activity, tech stack, hiring signals, and funding events. Output: `research_brief` JSON.

**Stage 3 — Personalize Worker:** Draft cold email using research brief. Reference one specific, verifiable signal. No fabricated connections. No "I noticed you went to Stanford" unless the data confirms it.

**Stage 4 — Evaluator Gate:** Score against compliance criteria (CAN-SPAM, no false claims, unsubscribe present) and quality criteria (personalization depth, tone, length < 150 words).

**Stage 5 — Human Approval:** Batch review queue. SDR approves/edits/rejects each draft. Approved drafts enter send queue with rate limiting.

**Stage 6 — CRM Log + Sequence:** Log activity in CRM. Enroll in follow-up sequence (day 3 bump, day 7 value-add, day 14 breakup).

### Volume and Rate Controls

Cold outbound agents need hard limits:

| Control | Setting | Rationale |
|---------|---------|-----------|
| Max drafts per hour | 50 | Prevents runaway generation |
| Max sends per SDR per day | 40 | Deliverability protection |
| Min time between sends to same domain | 24 hours | Avoid domain blacklisting |
| Max sequence length | 4 touches | Diminishing returns beyond 4 |
| Suppression list check | Every send | Legal compliance, brand protection |

Implement these as server-side constraints on the MCP email tools—not prompt instructions.

### Personalization Depth Tiers

Not every prospect gets the same research investment:

| Tier | Research Depth | Email Approach | Volume |
|------|---------------|----------------|--------|
| Strategic (top 50 accounts) | Full research brief, custom case study reference | Highly personalized, 120-150 words | 5-10/day |
| Target (ICP-fit accounts) | Company news + role-specific angle | Personalized template with 2-3 custom sentences | 20-30/day |
| Volume (broader list) | Industry-level personalization only | Segment template with company name + industry | 40-50/day |

The orchestrator assigns research depth based on account tier before spawning workers.

### Response Handling

When a cold outbound generates a reply, the router (Chapter 5) classifies it:

- **Positive interest** → Route to SDR with full thread context + calendar dispatch
- **Objection** → Route to objection-handling agent (addresses concern, offers alternative)
- **Not interested** → Update CRM, remove from sequence, add to suppression list
- **Out of office** → Pause sequence, reschedule follow-up for return date + 2 days
- **Unsubscribe** → Immediate suppression, CRM update, sequence termination

Response handling is where outbound ROI materializes. An agent that only sends but never processes replies is a broadcast machine, not a pipeline engine.

### Outbound Metrics

| Metric | Target |
|--------|--------|
| Research brief generation time | < 20 seconds per prospect |
| Draft approval rate (first pass) | > 60% |
| Open rate (Tier 1 personalized) | > 45% |
| Reply rate (Tier 1) | > 8% |
| Positive reply rate | > 3% |
| Meeting booked from outbound | > 1.5% of sends |
| Cost per meeting booked | < $15 (agent + data + SDR time) |

---

## Field Playbook: SDR Agent Prompt Schemas and Tool Definitions

This section ships production-ready schemas. Copy, adapt to your CRM and data sources, deploy.

### SDR Triage Orchestrator — System Prompt

```xml
<instructions>
You are the SDR Triage Orchestrator for [Company Name]. You coordinate inbound lead processing from webhook receipt to human-ready triage package within 60 seconds.
</instructions>

<context>
  <company>[Company Name] provides [product category] for [target vertical] companies with 200-2000 employees.</company>
  <icp_definition>
    Target: VP/Director level in Operations, Supply Chain, or IT.
    Company: 200-2000 employees, manufacturing/logistics/distribution verticals.
    Tech: Salesforce or similar CRM, ERP system (NetSuite, SAP, Dynamics).
    Signals: Recent funding, hiring in operations roles, manual process pain indicators.
  </icp_definition>
</context>

<workers>
  <worker id="enrichment" tools="clearbit_company_lookup, clearbit_person_lookup, crm_get_contact, crm_get_deal_history" timeout="15s"/>
  <worker id="scoring" tools="none" timeout="10s"/>
  <worker id="outreach" tools="none" timeout="15s"/>
  <worker id="calendar" tools="calendar_list_availability, calendar_get_booking_link" timeout="10s"/>
  <worker id="evaluator" tools="none" timeout="10s"/>
</workers>

<workflow>
  1. Invoke enrichment with lead email and company from trigger event.
  2. Invoke scoring with enrichment output and ICP scoring matrix.
  3. If tier A or B: invoke outreach with enrichment + scoring output.
  4. If tier A or B: invoke evaluator on outreach draft.
  5. If evaluator pass AND tier A: invoke calendar for booking link.
  6. Apply CRM updates: enrichment fields, icp_score, tier, activity log.
  7. Package triage_summary and route to human review queue.
  8. If tier C: enroll in nurture sequence, skip outreach.
  9. If tier D: tag and archive, skip outreach.
</workflow>

<constraints>
  Total workflow timeout: 90 seconds.
  Max retries per worker: 2.
  Never send email directly. Draft only.
  Never modify CRM fields: owner_id, deal_amount, contract_status.
  Escalate to human queue on any worker failure after retries.
</constraints>
```

### Scoring Worker — System Prompt

```xml
<instructions>
Score the provided lead against the ICP criteria matrix. Return structured JSON only. Apply rubric deterministically—use judgment only when data is genuinely ambiguous.
</instructions>

<scoring_matrix>
  <criterion name="company_size" weight="0.20">
    200-2000 employees: 1.0 | 50-199: 0.6 | less than 50 or greater than 2000: 0.3
  </criterion>
  <criterion name="industry_fit" weight="0.25">
    manufacturing, logistics, distribution: 1.0 | adjacent (retail, wholesale): 0.5 | other: 0.2
  </criterion>
  <criterion name="tech_stack" weight="0.15">
    CRM plus ERP detected: 1.0 | CRM only: 0.6 | none detected: 0.3
  </criterion>
  <criterion name="funding_growth" weight="0.15">
    Series B+ within 18 months: 1.0 | bootstrapped/profitable: 0.7 | unknown: 0.4
  </criterion>
  <criterion name="engagement_quality" weight="0.15">
    enterprise demo form: 1.0 | pricing page plus form: 0.8 | newsletter/content: 0.2
  </criterion>
  <criterion name="existing_relationship" weight="0.10">
    prior opportunity or customer: 1.0 | prior activity: 0.5 | net new: 0.3
  </criterion>
</scoring_matrix>

<tier_thresholds>
  A: score >= 80 | B: 60-79 | C: 40-59 | D: below 40
</tier_thresholds>

<output_schema>
{
  "icp_score": "integer 0-100",
  "tier": "A|B|C|D",
  "criterion_scores": { "company_size": 0.0, "industry_fit": 0.0, "tech_stack": 0.0, "funding_growth": 0.0, "engagement_quality": 0.0, "existing_relationship": 0.0 },
  "scoring_rationale": "2-3 sentences explaining tier assignment with specific data points"
}
</output_schema>
```

### Outreach Worker — System Prompt

```xml
<instructions>
Draft a personalized first-touch email for the provided lead. Reference one specific, verifiable detail from the enrichment data. Professional tone, under 150 words, no generic openers.
</instructions>

<constraints>
  Never fabricate facts, metrics, or case studies not provided in context.
  Never use: "I hope this finds you well", "I wanted to reach out", "touching base".
  Always include: specific company/role reference, one value proposition, calendar link if provided.
  Subject line: under 50 characters, company name included, no clickbait.
</constraints>

<variables>
  <sender_name>{{sdr_name}}</sender_name>
  <sender_title>{{sdr_title}}</sender_name>
  <company_name>{{company_name}}</company_name>
  <similar_customer>{{similar_customer_reference}}</similar_customer>
  <calendar_link>{{calendar_link_or_empty}}</calendar_link>
</variables>

<output_schema>
{
  "subject": "string, max 50 chars",
  "body": "string, max 150 words",
  "personalization_signals_used": ["list of specific data points referenced"],
  "word_count": "integer"
}
</output_schema>
```

### MCP Tool Definitions — SDR Toolkit

```json
[
  {
    "name": "crm_get_contact",
    "description": "Retrieve contact record by email. Returns name, title, company, lifecycle stage, owner, custom properties, and last activity date.",
    "input_schema": {
      "type": "object",
      "properties": { "email": { "type": "string" } },
      "required": ["email"]
    }
  },
  {
    "name": "crm_update_contact",
    "description": "Update allowed contact fields: lifecycle_stage, icp_score, icp_tier, enrichment_notes, tech_stack, employee_count, funding_stage, last_agent_touch.",
    "input_schema": {
      "type": "object",
      "properties": {
        "email": { "type": "string" },
        "fields": { "type": "object" }
      },
      "required": ["email", "fields"]
    }
  },
  {
    "name": "crm_log_activity",
    "description": "Log an activity on a contact record: type (email_draft, triage_complete, meeting_scheduled), notes, timestamp.",
    "input_schema": {
      "type": "object",
      "properties": {
        "email": { "type": "string" },
        "activity_type": { "type": "string" },
        "notes": { "type": "string" }
      },
      "required": ["email", "activity_type", "notes"]
    }
  },
  {
    "name": "clearbit_company_lookup",
    "description": "Enrich company by domain. Returns employee count, industry, tech stack, funding stage/amount/date, HQ location.",
    "input_schema": {
      "type": "object",
      "properties": { "domain": { "type": "string" } },
      "required": ["domain"]
    }
  },
  {
    "name": "clearbit_person_lookup",
    "description": "Enrich person by email. Returns name, title, LinkedIn URL, seniority level.",
    "input_schema": {
      "type": "object",
      "properties": { "email": { "type": "string" } },
      "required": ["email"]
    }
  },
  {
    "name": "calendar_list_availability",
    "description": "List available meeting slots for assigned SDR within date range. Respects business hours in specified timezone.",
    "input_schema": {
      "type": "object",
      "properties": {
        "sdr_id": { "type": "string" },
        "days_ahead": { "type": "integer", "maximum": 10 },
        "duration_minutes": { "type": "integer", "enum": [20, 30] },
        "timezone": { "type": "string" }
      },
      "required": ["sdr_id", "days_ahead", "duration_minutes"]
    }
  },
  {
    "name": "calendar_get_booking_link",
    "description": "Generate personalized booking link with pre-filled name, email, company fields.",
    "input_schema": {
      "type": "object",
      "properties": {
        "sdr_id": { "type": "string" },
        "prospect_name": { "type": "string" },
        "prospect_email": { "type": "string" },
        "prospect_company": { "type": "string" },
        "meeting_type": { "type": "string", "enum": ["intro_20min", "demo_30min"] }
      },
      "required": ["sdr_id", "prospect_email", "meeting_type"]
    }
  },
  {
    "name": "email_create_draft",
    "description": "Create email draft in SDR's mailbox. Does NOT send. Returns draft ID for human review.",
    "input_schema": {
      "type": "object",
      "properties": {
        "to": { "type": "string" },
        "subject": { "type": "string" },
        "body": { "type": "string" },
        "sdr_email": { "type": "string" }
      },
      "required": ["to", "subject", "body", "sdr_email"]
    }
  }
]
```

### Webhook Configuration — HubSpot Form Submission

```json
{
  "endpoint": "https://agents.yourcompany.com/webhooks/hubspot",
  "events": ["contact.creation", "form.submission"],
  "filters": {
    "form_ids": ["enterprise_demo_request", "pricing_inquiry", "contact_us"]
  },
  "normalization": {
    "event_type": "inbound_lead",
    "priority_mapping": {
      "enterprise_demo_request": "high",
      "pricing_inquiry": "high",
      "contact_us": "medium"
    }
  },
  "routing": {
    "workflow": "sdr_lead_triage",
    "orchestrator_version": "v2.3"
  }
}
```

---

## Deployment Sequence: Week One Revenue Engine

Do not deploy inbound triage, calendar dispatch, and cold outbound simultaneously. Sequence the rollout:

**Days 1-2: Enrichment + Scoring Only**
- Connect MCP tools: CRM read, Clearbit lookup.
- Deploy scoring worker in shadow mode—scores logged but no outreach generated.
- Human SDRs compare agent scores to their own judgment. Calibrate ICP matrix.

**Days 3-4: Add Outreach Drafts**
- Deploy outreach worker + evaluator in shadow mode.
- SDRs receive draft packages alongside their manual workflow.
- Measure edit rate and evaluator pass rate.

**Days 5-7: Go Live on Inbound**
- Enable CRM writes and draft creation.
- SDRs approve-and-send from agent queue.
- Monitor 60-second SLA and conversion metrics.

**Week 2: Calendar Dispatch**
- Add calendar tools and booking link generation.
- Connect Calendly/Cal.com webhook for post-booking automation.
- Deploy meeting prep brief generator.

**Week 3-4: Cold Outbound Pilot**
- Start with Strategic tier only (top 50 accounts).
- Human approval on every send.
- Expand to Target tier after reply rate validation.

---

## Action Checklist: Revenue Engine Deployment

**Infrastructure (from Chapters 4-5)**
- [ ] MCP servers configured: CRM, enrichment, calendar, email draft
- [ ] Webhook listener active for form submissions with idempotency
- [ ] Router classifying inbound events with > 95% accuracy
- [ ] State machine persisting workflow state to durable storage

**Inbound Triage**
- [ ] ICP scoring matrix defined with weights and tier thresholds
- [ ] Enrichment worker returning standardized payload schema
- [ ] Outreach worker drafting with personalization constraints
- [ ] Evaluator gating drafts at ≥ 0.80 threshold
- [ ] 60-second SLA monitored and alerting on breach

**Calendar Dispatch**
- [ ] Availability logic configured (business hours, lead time, load balancing)
- [ ] Personalized booking links generating with pre-filled fields
- [ ] Post-booking webhook triggering meeting prep workflow
- [ ] No-show re-engagement protocol active

**Cold Outbound**
- [ ] Prospect ingest with deduplication and suppression list checks
- [ ] Research + personalize + evaluate pipeline operational
- [ ] Rate limits enforced server-side (sends/day, domain spacing)
- [ ] Response routing classifying replies into action categories

**Measurement**
- [ ] Weekly dashboard: time-to-triage, CRM completeness, tier accuracy, edit rate, conversion
- [ ] Cost per lead and cost per meeting tracked against manual baseline
- [ ] SDR feedback loop: monthly calibration session on scoring accuracy

---

## The Bridge

Your revenue engine is running. Leads triaged in 60 seconds. Calendar links dispatched autonomously. Cold outbound generating pipeline while your team closes deals. SDRs shifted from data entry to deal-making.

But revenue is only half the operational load crushing your margin. The back office—the document ingestion pipelines, vendor reconciliation, cross-system data alignment, executive briefing generation—consumes the other half of administrative overhead.

Chapter 7 deploys the same architecture against back-office operations: PDF ingestion, ERP reconciliation, and the Operations Coordinator Agent that turns unstructured operational chaos into structured, verified, auditable workflows.

The revenue engine captures demand. The back-office engine eliminates waste. Both run on the same infrastructure you built in these six chapters.

Turn the page. Your operations team is next.
