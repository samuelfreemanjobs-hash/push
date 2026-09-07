# Chapter 5: Multi-Agent Choreography — Designing Workflows, Routers, and Worker Swarms

## The 47-Step Monolith That Collapsed at Step 31

A logistics startup tried to automate their entire vendor onboarding pipeline with a single Claude agent. One system prompt. Forty-seven sequential steps: receive vendor application, validate tax ID format, cross-reference SAM.gov exclusion list, pull insurance certificate from email attachment, extract coverage limits, compare against policy requirements, draft approval memo, route to legal, update ERP vendor master, send welcome packet.

It worked in sandbox. In production, it failed at step 31—insurance certificate parsing—roughly 40% of the time. When it failed, the entire 30-step chain restarted from scratch. Context windows filled with retry logs. Token costs spiked. Human operators spent more time debugging the agent than doing the work manually.

The fix was not a better prompt. It was a better architecture.

They decomposed the monolith into six specialized agents orchestrated by a routing layer. Insurance parsing became an isolated worker with its own error-recovery loop. Legal routing became a handoff to a separate agent with a clean state payload. Failures localized. Retries scoped. Throughput recovered.

This chapter teaches you to build that architecture—using Anthropic's five core workflow patterns as your design vocabulary.

---

## Anthropic's Five Workflow Patterns: Your Design Vocabulary

Before engineering multi-agent systems, internalize the pattern catalog Anthropic defines for production LLM workflows. These are not abstract theory. They are the architectural primitives you combine to build any operational agent system.

### Pattern 1: Prompt Chaining

Decompose a task into sequential steps where each LLM call processes the output of the previous one. No agent autonomy—just a pipeline.

**Use when:** Steps are strictly sequential, each step's output is the next step's input, and you do not need dynamic routing.

**Example:** Inbound email → extract sender intent → classify urgency → draft response template. Three chained prompts, three discrete outputs.

**Limitation:** No branching. If step 2 fails, the chain breaks. No parallel processing.

### Pattern 2: Routing

A classifier agent examines the input and routes it to specialized downstream handlers based on category.

**Use when:** Inputs vary significantly in type, and specialized prompts outperform generalist ones.

**Example:** Support inbox router → classifies tickets as billing, technical, or account management → dispatches to domain-specific response agents.

**Key metric:** Routing accuracy. A 5% misroute rate on 1,000 daily tickets means 50 tickets handled by the wrong specialist.

### Pattern 3: Parallelization

Multiple agents process the same input simultaneously, or different segments of a large input in parallel. Results merge downstream.

**Use when:** Subtasks are independent, latency matters, or you need multiple perspectives on the same data.

**Example:** RFP analysis → three agents simultaneously evaluate pricing feasibility, technical requirements, and competitive positioning → merger agent synthesizes findings.

**Tradeoff:** Higher token cost (multiple concurrent calls) vs. lower latency (wall-clock time drops to the slowest single agent).

### Pattern 4: Orchestrator-Workers

A lead agent breaks a complex task into subtasks, delegates to worker agents, and synthesizes their outputs into a final result.

**Use when:** The task is too complex for a single prompt, subtasks are dynamic (not known in advance), and you need adaptive decomposition.

**Example:** Competitive analysis request → orchestrator identifies five competitors → spawns five research workers → each worker returns structured findings → orchestrator writes executive summary.

**This is the backbone pattern for most operational agent systems.**

### Pattern 5: Evaluator-Optimizer

One agent generates output. A second agent evaluates it against criteria and provides feedback. The generator revises. Loop until quality threshold met or max iterations reached.

**Use when:** Output quality is paramount, subjective judgment is required, and you can define evaluation criteria explicitly.

**Example:** Client proposal draft → evaluator checks tone, accuracy, pricing consistency, compliance language → generator revises → evaluator re-scores. Max three iterations.

### Pattern Selection Matrix

| Operational Scenario | Primary Pattern | Secondary Pattern |
|---------------------|-----------------|-------------------|
| Lead triage + enrichment | Routing | Prompt Chaining |
| Document reconciliation | Orchestrator-Workers | Evaluator-Optimizer |
| Executive briefing from 6 data sources | Parallelization | Prompt Chaining |
| Client-facing email draft | Evaluator-Optimizer | — |
| Vendor onboarding (multi-step) | Orchestrator-Workers | Routing |
| Support ticket resolution | Routing | Evaluator-Optimizer |

Most production systems combine three or more patterns. The art is knowing which pattern owns which segment of the workflow.

---

## The Orchestrator-Worker Dynamic: Strategy Separated from Execution

The Orchestrator-Worker pattern is the foundational architecture for operational agent swarms. One agent plans. Multiple agents execute. The orchestrator never touches tools directly—it delegates.

### Role Definitions

**Orchestrator Agent**
- Receives the high-level task (trigger event, human request, scheduled job).
- Decomposes into subtasks with explicit success criteria.
- Selects which worker agents to invoke.
- Passes clean state payloads to each worker.
- Synthesizes worker outputs into the final deliverable.
- Handles failures: retry, reroute, escalate.

**Worker Agents**
- Single-purpose. One domain, one toolset, one output schema.
- Receive a scoped subtask with all context needed—no access to the full workflow history.
- Execute their TCAV loop independently.
- Return structured results to the orchestrator.
- No awareness of other workers or the broader workflow.

### Architecture Diagram

```text
                    ┌─────────────────────────┐
                    │     TRIGGER EVENT       │
                    │  (webhook, cron, human) │
                    └───────────┬─────────────┘
                                │
                                ▼
                    ┌─────────────────────────┐
                    │    ORCHESTRATOR AGENT   │
                    │                         │
                    │  • Decompose task       │
                    │  • Assign subtasks      │
                    │  • Synthesize results   │
                    │  • Handle failures      │
                    └───────────┬─────────────┘
                                │
              ┌─────────────────┼─────────────────┐
              │                 │                 │
              ▼                 ▼                 ▼
    ┌─────────────────┐ ┌─────────────┐ ┌─────────────────┐
    │  WORKER: Enrich  │ │ WORKER:     │ │ WORKER: Draft   │
    │  CRM data from   │ │ Score lead  │ │ personalized    │
    │  external APIs   │ │ against ICP │ │ outreach email  │
    └────────┬────────┘ └──────┬──────┘ └────────┬────────┘
             │                 │                 │
             └─────────────────┼─────────────────┘
                               │
                               ▼
                    ┌─────────────────────────┐
                    │   ORCHESTRATOR SYNTHESIS │
                    │  • Merge worker outputs  │
                    │  • Update CRM record     │
                    │  • Queue human review    │
                    └─────────────────────────┘
```

### Orchestrator System Prompt Structure

The orchestrator prompt is fundamentally different from worker prompts. It contains no tool definitions for CRM or email—it contains worker definitions:

```xml
<instructions>
You are the Lead Triage Orchestrator. You receive inbound lead events and coordinate three worker agents to produce a complete triage package within 60 seconds.
</instructions>

<workers>
  <worker id="enrichment" description="Enriches contact with company data, tech stack, and funding info from external APIs. Input: email, company name. Output: enrichment_payload JSON."/>
  <worker id="scoring" description="Scores lead against ICP criteria. Input: contact record + enrichment_payload. Output: icp_score (0-100), tier (A/B/C/D), scoring_rationale."/>
  <worker id="outreach" description="Drafts personalized first-touch email. Input: contact record, icp_score, tier. Output: email_draft with subject line and body."/>
</workers>

<workflow>
  1. Invoke enrichment worker with lead email and company.
  2. Pass enrichment output to scoring worker.
  3. If tier A or B: invoke outreach worker.
  4. If tier C: create nurture task only.
  5. If tier D: log and archive. No outreach.
  6. Synthesize all outputs into triage_summary JSON.
</workflow>

<constraints>
  Maximum 3 worker retries per subtask. Escalate to human after 3 failures.
  Total workflow timeout: 90 seconds.
</constraints>
```

### Worker Isolation Principle

Each worker operates in a clean context window. The enrichment worker does not see the outreach draft. The scoring worker does not see the raw webhook payload. The orchestrator passes exactly the fields each worker needs—nothing more.

This isolation delivers three engineering benefits:

1. **Context efficiency.** Workers use 2,000-4,000 tokens, not 20,000 tokens of full workflow history.
2. **Failure containment.** Insurance parsing fails? Only that worker retries. Steps 1-30 do not restart.
3. **Independent optimization.** Tune the scoring worker's prompt without risking regression in enrichment or outreach.

### Delegation Payload Schema

Every orchestrator-to-worker handoff uses a standardized envelope:

```json
{
  "task_id": "subtask_enrich_8f3a",
  "parent_workflow_id": "wf_lead_triage_20250907_001",
  "worker_id": "enrichment",
  "input": {
    "email": "sarah.chen@acmecorp.com",
    "company": "Acme Corp"
  },
  "constraints": {
    "timeout_seconds": 15,
    "max_retries": 2
  },
  "expected_output_schema": "enrichment_payload_v1"
}
```

Workers return a matching envelope with `status`, `output`, and `error` fields. The orchestrator logs every handoff for debugging and performance measurement.

---

## The Triage Router: Dynamic Dispatch to Specialized Agents

Routing is the front door of multi-agent systems. Before any orchestrator decomposes a task, something must decide *which* orchestrator—or standalone agent—handles this particular input.

### Router Architecture

```text
  INBOUND EVENT
       │
       ▼
  ┌─────────────┐
  │   ROUTER    │──── Classify input type + attributes
  │   AGENT     │
  └──────┬──────┘
         │
    ┌────┼────────┬──────────┬──────────┐
    ▼    ▼        ▼          ▼          ▼
  Lead  Support  Vendor    Invoice   Unknown
  Triage Triage  Onboard   Reconcile  → Human
  Orch.  Agent    Orch.     Agent     Queue
```

The router is a lightweight classification agent—fast, cheap, high-accuracy. It does not execute workflows. It labels and dispatches.

### Router Prompt Design

Router prompts prioritize classification speed and accuracy over depth:

```xml
<instructions>
Classify the incoming event into exactly one category. Return JSON only.
</instructions>

<categories>
  <category id="inbound_lead" signals="form submission, demo request, pricing inquiry, new contact created"/>
  <category id="support_ticket" signals="help request, bug report, account issue, billing question"/>
  <category id="vendor_document" signals="invoice attachment, W-9, insurance cert, packing slip"/>
  <category id="outbound_reply" signals="response to SDR email, meeting acceptance, objection"/>
  <category id="unknown" signals="cannot classify with confidence above 0.7"/>
</categories>

<output_schema>
{
  "category": "string",
  "confidence": "float 0-1",
  "routing_attributes": {
    "priority": "high|medium|low",
    "source_system": "string",
    "entity_id": "string"
  }
}
</output_schema>
```

### Confidence Thresholds and Fallback Routing

Never route on low confidence. Define explicit thresholds:

| Confidence | Action |
|------------|--------|
| ≥ 0.90 | Auto-route to classified handler |
| 0.70 – 0.89 | Route with flag for post-hoc human review |
| < 0.70 | Route to `unknown` → human triage queue |

Track misroute rate weekly. If support tickets are landing in the lead triage orchestrator more than 2% of the time, retrain the router with misclassified examples added to the prompt's few-shot examples.

### Attribute-Based Sub-Routing

Classification is step one. Attributes refine routing within a category:

**Inbound lead sub-routing:**
- `form_id = enterprise_demo` → High-priority SDR orchestrator (60-second SLA)
- `form_id = newsletter` → Nurture sequence agent (batch, 4-hour SLA)
- `utm_source = partner_referral` → Partner channel orchestrator (includes partner context lookup)

**Support ticket sub-routing:**
- `priority = critical` AND `account_tier = enterprise` → Escalation orchestrator with on-call notification
- `category = billing` → Billing resolution agent with payment system access
- `category = how_to` → Self-service knowledge base agent (no human tools)

Implement sub-routing in deterministic code after the LLM classification—not inside the router prompt. The router classifies; your routing engine applies business rules. This separation keeps business logic version-controlled and testable without prompt rewrites.

### Router Performance Benchmarks

A production router should hit these targets:

- **Latency:** < 2 seconds per classification (use a fast model tier for routing)
- **Accuracy:** > 95% on primary category classification
- **Cost:** < $0.005 per routed event
- **Throughput:** Handle 10x your peak event volume without queue backup

---

## State Machines and Handoffs: Clean State Between Disconnected Processes

Multi-agent workflows are state machines. Each agent transition is a state change. Each handoff passes a state payload. Engineering clean state is what separates reliable swarms from fragile prompt chains.

### The Workflow State Object

Every multi-agent workflow maintains a single state object, updated at each transition:

```json
{
  "workflow_id": "wf_lead_triage_20250907_001",
  "status": "in_progress",
  "current_stage": "scoring",
  "created_at": "2025-09-07T14:14:32Z",
  "trigger_event": {
    "event_id": "evt_8f3a2b1c",
    "event_type": "inbound_lead",
    "payload": { }
  },
  "stage_results": {
    "enrichment": {
      "status": "completed",
      "completed_at": "2025-09-07T14:14:38Z",
      "output": {
        "company_size": 450,
        "industry": "manufacturing",
        "tech_stack": ["Salesforce", "NetSuite"],
        "funding_stage": "Series C"
      }
    },
    "scoring": {
      "status": "in_progress",
      "started_at": "2025-09-07T14:14:39Z"
    }
  },
  "metadata": {
    "orchestrator_version": "v2.3",
    "total_tokens_consumed": 4200,
    "retry_count": 0
  }
}
```

Store this in a durable state store—Postgres, Redis, DynamoDB—not in the LLM's context window. Context is volatile. State must survive agent crashes, API timeouts, and deployment restarts.

### State Machine Stages

Define explicit stages with entry conditions, exit conditions, and allowed transitions:

```text
  RECEIVED ──▶ ENRICHING ──▶ SCORING ──▶ ROUTING ──▶ COMPLETED
                  │              │           │
                  ▼              ▼           ▼
               FAILED         FAILED      ESCALATED
                  │              │           │
                  └──────────────┴───────────▶ HUMAN_REVIEW
```

**Transition rules:**
- `RECEIVED → ENRICHING`: Automatic on workflow creation.
- `ENRICHING → SCORING`: Enrichment worker returns `status: completed`.
- `ENRICHING → FAILED`: Enrichment worker returns `status: failed` after max retries.
- `SCORING → ROUTING`: Score computed. Tier assigned.
- `ROUTING → COMPLETED`: Outreach drafted (tier A/B) or nurture task created (tier C) or archived (tier D).
- `Any FAILED → HUMAN_REVIEW`: Automatic after retry exhaustion.
- `HUMAN_REVIEW → COMPLETED`: Human operator resolves and marks complete.

### Handoff Protocol

When Agent A completes its stage and Agent B must take over:

1. **Agent A** writes its output to `stage_results.{stage_name}` in the state object.
2. **Agent A** sets `current_stage` to the next stage.
3. **Orchestrator** (or routing engine) reads the state object.
4. **Orchestrator** constructs Agent B's input from `stage_results`—not from Agent A's raw context.
5. **Agent B** receives a fresh context window with only the fields it needs.

Agent B never sees Agent A's reasoning chain, failed attempts, or intermediate drafts. It sees structured output. This is the handoff protocol that prevents context pollution across a swarm.

### Idempotent Stage Execution

Stages must be safely re-runnable. If the scoring agent crashes mid-execution:

1. State shows `scoring: in_progress`.
2. Timeout triggers retry.
3. Scoring agent re-executes with the same input from `stage_results.enrichment.output`.
4. No duplicate CRM writes, no duplicate emails.

Implement idempotency keys on every write action: `workflow_id + stage_name + action_type`. Before writing, check if that key exists. If yes, skip.

### Long-Running Workflow Checkpoints

Some workflows span hours or days—vendor onboarding, legal review, multi-party approval. Checkpoint the state object at every stage boundary:

- Persist to durable storage after each stage completion.
- Include `checkpoint_at` timestamp and `checkpoint_by` (agent ID or human operator).
- On system restart, scan for workflows in `in_progress` status and resume from last checkpoint.

A vendor onboarding workflow might checkpoint across five days: application received (day 1), documents validated (day 1), insurance verified (day 2), legal review queued (day 3), legal approved (day 5), ERP updated (day 5). Each checkpoint is independently recoverable.

---

## The Evaluator-Optimizer Loop: Quality Gates Before Delivery

Generation without evaluation is hope dressed as automation. The Evaluator-Optimizer pattern installs quality gates that catch errors before they reach customers, CRMs, or compliance systems.

### Loop Architecture

```text
  ┌──────────────┐
  │  GENERATOR   │──▶ Draft output
  │   AGENT      │
  └──────────────┘
         │
         ▼
  ┌──────────────┐     Pass (score ≥ threshold)
  │  EVALUATOR   │─────────────────────────▶ APPROVED OUTPUT
  │   AGENT      │
  └──────┬───────┘
         │ Fail (score < threshold)
         ▼
  ┌──────────────┐
  │  FEEDBACK    │──▶ Specific revision instructions
  │   INJECTION  │
  └──────┬───────┘
         │
         ▼
  ┌──────────────┐
  │  GENERATOR   │──▶ Revised draft (iteration 2)
  │  (retry)     │
  └──────────────┘
         │
         ▼
       (loop max 3 iterations, then escalate to human)
```

### Evaluator Design Principles

The evaluator is not a duplicate of the generator with a critical tone. It is a structured scoring agent with explicit criteria:

```xml
<instructions>
Evaluate the provided email draft against scoring criteria. Return JSON scores only. Do not rewrite the draft.
</instructions>

<criteria>
  <criterion id="personalization" weight="0.25">
    References specific company details, role, or recent activity. Generic templates score below 0.5.
  </criterion>
  <criterion id="accuracy" weight="0.30">
    All factual claims verifiable from provided context. No fabricated metrics or case studies.
  </criterion>
  <criterion id="tone" weight="0.20">
    Professional, direct, not salesy. Matches company voice guidelines in context.
  </criterion>
  <criterion id="compliance" weight="0.25">
    No unsubstantiated claims. No competitor disparagement. Includes required disclaimer if mentioning pricing.
  </criterion>
</criteria>

<output_schema>
{
  "overall_score": "float 0-1",
  "criterion_scores": { "personalization": 0.0, "accuracy": 0.0, "tone": 0.0, "compliance": 0.0 },
  "pass": "boolean (overall_score >= 0.80)",
  "revision_instructions": ["specific, actionable feedback if pass=false"]
}
</output_schema>
```

### Threshold Configuration

| Output Type | Pass Threshold | Max Iterations | Escalation |
|-------------|---------------|----------------|------------|
| Internal draft (CRM notes) | 0.70 | 2 | Auto-approve with flag |
| SDR outreach email | 0.80 | 3 | Human review queue |
| Client proposal | 0.85 | 3 | Human review queue |
| Compliance-sensitive document | 0.90 | 2 | Mandatory human approval |

### Feedback Injection

When the evaluator fails a draft, revision instructions must be specific and actionable—not "make it better":

**Bad feedback:** "Tone needs improvement."

**Good feedback:** "Opening line uses generic greeting ('I hope this finds you well'). Replace with reference to their Series C announcement from enrichment data. Paragraph 2 claims '30% cost reduction'—no supporting data in context. Remove or cite the case study from variables."

The generator's retry prompt includes the original draft, the evaluator's scores, and the revision instructions. It does not include the evaluator's full reasoning—just the actionable deltas.

### Cost Control on Evaluation Loops

Each iteration doubles token spend for that output. Monitor:

- **Average iterations to pass:** Target < 1.5 for mature prompts.
- **Escalation rate:** Target < 10% reaching max iterations.
- **Evaluator-generator token ratio:** Evaluator should consume 30-40% of generator tokens (lighter prompt, structured output only).

If average iterations exceed 2.0, the problem is usually the generator prompt—not the evaluator. Fix generation quality before tightening evaluation thresholds.

---

## Composing Patterns: The Full Revenue Triage Stack

The five patterns combine into production systems. Here is how a complete inbound lead triage stack maps to each pattern:

| Workflow Segment | Pattern | Agent Role |
|-----------------|---------|------------|
| Webhook received → classify event | Routing | Router Agent |
| Classify → enrich → score → draft | Orchestrator-Workers | Triage Orchestrator + 3 Workers |
| Enrichment + scoring simultaneously | Parallelization | (Optional) Run enrichment API calls in parallel within worker |
| Email draft quality check | Evaluator-Optimizer | Outreach Worker + Quality Evaluator |
| Stage-to-stage data passing | Prompt Chaining | State machine handoffs between workers |

Total agents in this stack: 6 (1 router, 1 orchestrator, 3 workers, 1 evaluator). Total latency target: < 60 seconds from webhook to approved draft in human review queue.

---

## Field Scenario: Vendor Onboarding Decomposed

Returning to the logistics startup from the opening—their decomposed architecture:

**Router:** Classifies inbound documents as vendor application, invoice, insurance cert, or unknown.

**Orchestrator:** Manages the 6-stage onboarding workflow with checkpoint persistence.

**Workers:**
- `doc_parser` — Extracts structured data from PDFs and attachments
- `compliance_checker` — Validates tax ID, SAM.gov status, insurance limits
- `erp_writer` — Creates vendor master record in NetSuite
- `comms_drafter` — Generates welcome packet and notification emails

**Evaluator:** Reviews compliance_checker output against policy requirements before ERP write.

**Results:**
- End-to-end processing time: 4.2 hours (human) → 12 minutes (automated) for standard applications
- Failure recovery: Localized to failed stage, not full pipeline restart
- Human review rate: 18% (complex exceptions only, down from 100%)
- Token cost per onboarding: ~$0.45 average

---

## Action Checklist: Multi-Agent Architecture Deployment

**Pattern Selection**
- [ ] Map your workflow to Anthropic's five patterns—identify primary and secondary patterns per segment
- [ ] Identify decomposition boundaries (where does one agent's job end and another's begin?)
- [ ] Define which segments require Evaluator-Optimizer loops vs. single-pass execution

**Orchestrator-Worker Setup**
- [ ] Write orchestrator system prompt with worker definitions and workflow stages
- [ ] Build isolated worker prompts—each with scoped tools and output schemas
- [ ] Implement delegation payload envelope (task_id, input, constraints, expected_output_schema)
- [ ] Set per-worker timeout and retry limits

**Routing Layer**
- [ ] Define classification categories with signal descriptions
- [ ] Set confidence thresholds (auto-route ≥ 0.90, flagged 0.70-0.89, human < 0.70)
- [ ] Implement attribute-based sub-routing in deterministic code
- [ ] Establish weekly misroute rate monitoring

**State Management**
- [ ] Design workflow state object schema with stage_results tracking
- [ ] Implement durable state store (not LLM context)
- [ ] Define state machine stages, transitions, and failure paths
- [ ] Add idempotency keys on all write actions
- [ ] Configure checkpoint persistence for long-running workflows

**Evaluation Gates**
- [ ] Define scoring criteria with weights for each output type
- [ ] Set pass thresholds and max iteration counts per output type
- [ ] Build feedback injection format (specific, actionable revision instructions)
- [ ] Monitor average iterations-to-pass and escalation rates

---

## The Bridge

You now have the architectural vocabulary to decompose any operational workflow into a choreographed swarm: routers that classify, orchestrators that delegate, workers that execute, state machines that persist, and evaluators that gate quality.

Chapter 4 gave your agents hands. This chapter gave them colleagues.

Chapter 6 deploys the full stack against the highest-ROI operational target in most B2B companies: the revenue engine. Sixty-second inbound lead triage. Autonomous calendar dispatch. Cold outbound pipelines that run while your SDRs sleep. Complete field playbooks with prompt schemas and tool definitions you can deploy this week.

The architecture is built. Time to point it at revenue.
