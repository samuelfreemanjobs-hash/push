# Chapter 3: The Digital Worker Architecture — Standard Operating Procedures as Executable Code

## Deconstructing Human Tasks: The Trigger-Context-Action-Verification (TCAV) Matrix

Your business already has agent specifications. They live in Google Docs titled "Receiving SOP v4," "Quote Request Handling," and "Vendor Exception Process—READ THIS FIRST."

The problem is not missing procedures. It is that procedures written for human interpretation are **ambiguous by design**. Humans fill gaps with institutional knowledge. LLMs fill gaps with invention.

The TCAV Protocol converts any SOP into a machine-executable matrix with zero interpretive latitude.

### Trigger

What event initiates the workflow?

Triggers must be **observable, unambiguous, and automatable**:

| Valid Trigger | Invalid Trigger |
|---|---|
| Email arrives at receiving@ with PDF attachment | "When someone notices a discrepancy" |
| File uploaded to /intake/vendor-docs/ S3 bucket | "End of day if backlog exists" |
| Webhook from ERP: PO status → PARTIALLY_RECEIVED | "Manager assigns task" |
| Scheduled cron: daily 6:00 AM batch | "When it feels urgent" |

For packing slip reconciliation, the production trigger is:

```
TRIGGER: Inbound email to receiving@company.com
CONDITION: At least one attachment with MIME type application/pdf OR image/*
EXCLUDE: Emails with subject containing "RE:" and no new attachment (thread replies)
```

Invalid triggers produce agents that never wake up—or wake up constantly on noise.

### Context

What state must the worker know before acting?

Context splits into three categories:

**Session context:** Identifiers and status for this execution—workflow ID, current step, timestamps, retry count.

**Domain context:** Business rules that apply—vendor UOM conversions, approval thresholds ($500 variance limit), matching tolerances (±0.1% on line totals).

**Historical context:** Prior interactions that inform current action—"Vendor Acme Corp had 3 exceptions in Q2, all UOM-related" or "Customer Henderson ordered M8-SS-40 × 500 in March PO #2291."

Context must be **injected, not assumed**. Your SOP says "check the vendor history." TCAV specifies:

```xml
<context_requirements step="COMPARE">
  <required>vendor_profile</required>
  <required>po_record</required>
  <required>uom_conversion_table</required>
  <optional>vendor_exception_history_90d</optional>
</context_requirements>
```

If required context is unavailable, the agent escalates with `MISSING_CONTEXT: vendor_profile`—not proceeds with guesswork.

### Action

What system mutations occur?

Actions are **typed, bounded, and permissioned**:

| Action Type | Example | Permission Level |
|---|---|---|
| READ | query_erp_purchase_order | Autonomous |
| READ | parse_document_fields | Autonomous |
| WRITE | update_reconciliation_status | Autonomous (status only) |
| WRITE | create_exception_ticket | Autonomous |
| WRITE | modify_po_quantity | HITL required above threshold |
| WRITE | send_vendor_email | HITL always |

Every action in your SOP must map to a tool definition with explicit permission tier. "Notify the vendor" becomes `draft_vendor_email` (autonomous) + `send_vendor_email` (HITL)—never a single ambiguous "handle vendor communication" action.

For B2B quote assembly, the action sequence:

```
1. parse_rfq_document → structured fields
2. query_crm_customer → account_id, pricing_tier
3. query_product_catalog → SKU matches, unit pricing
4. calculate_quote_totals → line items, discounts, freight
5. create_crm_opportunity → draft status
6. generate_quote_pdf → attached to opportunity
7. [HITL GATE] submit_quote_for_approval
8. send_quote_email → only after approval flag
```

Each action has a tool, a schema, and a verification step before the next action executes.

### Verification

How do you know the work is correct?

Verification is the most under-specified section in human SOPs—and the most critical for agents. Every TCAV workflow defines verification at two levels:

**Step verification:** After each action, confirm the expected state change occurred.

```
ACTION: update_reconciliation_status(po=8842, status=MATCHED)
VERIFY: query_reconciliation_status(po=8842) → returns MATCHED
ASSERT: response_timestamp within 30 seconds of action_timestamp
```

**Terminal verification:** Before marking the workflow complete, confirm all business invariants hold.

```
TERMINAL CHECKS:
- All line items classified (MATCH / VARIANCE / MISSING)
- Sum of matched line values within 0.1% of document total
- Exception tickets created for all VARIANCE items
- Audit log entry written with workflow_id
- No WRITE actions pending HITL approval
```

Verification transforms agents from "executed something" to "executed correctly"—the difference between automation and liability.

### TCAV Worked Example: BOL Reconciliation

```
TRIGGER: PDF attachment on receiving@ email

CONTEXT:
  - Parse vendor_id from document header or email domain mapping
  - Load vendor_profile (UOM rules, contact, historical exception rate)
  - Retrieve PO number from parsed doc or email subject regex

ACTIONS:
  1. parse_packing_slip → extracted_lines[]
  2. parse_bol → bol_lines[]
  3. query_erp_purchase_order(po_number) → erp_lines[]
  4. compare_three_way(extracted, bol, erp) → match_results[]
  5. FOR EACH variance: create_exception_ticket
  6. FOR EACH match: update_receipt_status
  7. generate_reconciliation_summary

VERIFICATION:
  - match_results covers 100% of erp_lines
  - exception_ticket count == variance count
  - ERP receipt status reflects matched quantities (re-query)
  - summary totals reconcile to source document totals
```

This matrix is your SOP translated to executable logic. No interpretive gaps.

---

## Translating SOPs to System Directives: Structuring Bulletproof System Prompts with Explicit XML Zoning

English prose prompts drift. Operators reword instructions. Model updates shift behavior. New engineers "improve" prompts without regression testing.

The **Zero-Drift Prompt Spec** eliminates interpretive variance through explicit XML zoning—structured sections the model parses as distinct instruction classes, not narrative it interprets loosely.

### XML Zone Architecture

```xml
<agent_identity>
  <!-- WHO: Role, scope boundaries, authority limits -->
</agent_identity>

<operational_protocol>
  <!-- HOW: TCAV sequence, step ordering, loop bounds -->
</operational_protocol>

<context_rules>
  <!-- WHAT STATE: Required context, injection triggers, missing context handling -->
</context_rules>

<tool_directives>
  <!-- ACTIONS: Tool usage rules, permission tiers, parameter constraints -->
</tool_directives>

<verification_gates>
  <!-- PROOF: Step verification, terminal checks, escalation conditions -->
</verification_gates>

<error_handling>
  <!-- FAILURE: Retry logic, escalation payloads, forbidden behaviors -->
</error_handling>

<output_contract>
  <!-- DELIVERABLE: Response schema, logging requirements, audit fields -->
</output_contract>
```

Each zone is **independently versionable**. Update verification gates without touching agent identity. Regression-test zones in isolation.

### Zero-Drift Prompt Spec: Reconciliation Agent Example

```xml
<agent_identity>
You are Receiving Reconciliation Agent RC-001.
Scope: Three-way matching of vendor packing slips, bills of lading, and ERP PO line items.
Authority: Autonomous READ and status WRITE. Exception ticket CREATE.
Prohibited: PO quantity modification, vendor email send, payment approval.
</agent_identity>

<operational_protocol>
Execute TCAV sequence strictly in order:
1. INGEST → validate attachment, parse documents
2. CONTEXT → load vendor profile, identify PO number
3. QUERY → retrieve ERP PO lines
4. COMPARE → three-way match with UOM normalization
5. ACT → update statuses, create exception tickets
6. VERIFY → terminal checks (see verification_gates)
Maximum workflow duration: 12 tool calls. Exceed → escalate WORKFLOW_TIMEOUT.
</operational_protocol>

<context_rules>
Required before COMPARE step:
- vendor_profile with uom_conversions
- erp_lines with received_qty
- parsed slip and BOL with line_items[]
If po_number not extractable with confidence > 0.8:
  escalate MISSING_PO with parsed fields attached.
Do not guess PO numbers.
</context_rules>

<tool_directives>
query_erp_purchase_order: Use only after po_number confirmed.
create_exception_ticket: Required for every VARIANCE classification.
  Include: line_number, variance_type, variance_amount, supporting_fields.
update_receipt_status: Only for MATCH classifications.
  Never update status for VARIANCE lines until exception resolved.
</tool_directives>

<verification_gates>
Step gate after COMPARE:
  Assert len(match_results) == len(erp_lines)
  Assert every erp_line has classification in [MATCH, VARIANCE, MISSING]
Terminal gate before COMPLETE:
  Assert exception_count == count(VARIANCE)
  Assert sum(matched_values) within 0.1% of document_total
  Assert audit_log written
If any gate fails: escalate VERIFICATION_FAILURE with gate name and diagnostic.
</verification_gates>

<error_handling>
Retry policy: Max 2 retries per step with injected failure context.
API timeout: Wait 5s, retry once, then escalate API_UNAVAILABLE.
Parse failure: Retry with alternate extraction, then escalate PARSE_FAILURE.
Never fabricate line items, quantities, or PO numbers.
Never skip verification gates.
</error_handling>

<output_contract>
Return JSON matching ReconciliationResult schema v1.2.
Required fields: workflow_id, status, match_summary, exception_ids[], audit_trail[].
Status values: COMPLETE | ESCALATED | FAILED.
</output_contract>
```

This spec is not a prompt. It is a **contract**—reviewable by operations, testable by engineering, versionable in Git.

### Why XML Zoning Prevents Drift

Narrative prompts conflate instructions. "Be careful with exceptions and also check the ERP and remember our vendor Acme uses cases not eaches" becomes ambiguous when the model prioritizes "be careful" over "check the ERP."

XML zones isolate instruction classes. The model receives explicit boundaries: identity constraints live in `<agent_identity>`, never overridden by task payload. Verification gates in `<verification_gates>` execute regardless of what the parsed document contains.

When behavior drifts after a model update, you identify which zone failed regression—not rewrite the entire prompt from memory.

---

## Role Specialization: Why Specialized Single-Purpose Micro-Agents Outperform Monolithic Generalist Agents

The instinct when deploying AI is consolidation: one agent to rule them all. Receiving, quotes, AP, customer support—surely a sufficiently smart model handles everything?

Production data says otherwise.

### The Generalist Failure Mode

A monolithic "Operations Agent" attempting packing slip reconciliation, B2B quote assembly, and vendor invoice matching in one system prompt exhibits predictable failures:

**Context pollution:** Quote pricing rules bleed into reconciliation logic. A customer discount tier incorrectly applied to vendor UOM conversion.

**Tool sprawl:** Twenty-seven tool definitions in one agent. Claude selects wrong tools with increasing frequency as the tool surface grows. Error rates compound.

**Verification complexity:** Terminal checks for reconciliation (quantity matching) conflict with terminal checks for quotes (margin thresholds). Combined verification gates become unenforceable.

**Debugging nightmare:** When output quality degrades, you cannot isolate whether reconciliation prompts, quote prompts, or shared identity instructions caused the regression.

**Token waste:** Every execution loads the full system prompt—reconciliation rules the quote workflow never needs, support escalation policies irrelevant to AP matching.

### The Micro-Agent Architecture

Deploy **single-purpose agents** with narrow scope, focused tool sets, and independent verification:

| Agent ID | Scope | Tools | Avg Tokens/Run |
|---|---|---|---|
| RC-001 | Packing slip ↔ BOL ↔ ERP reconciliation | 6 | 11,000 |
| QA-002 | B2B quote assembly from RFQ | 8 | 14,000 |
| AP-003 | Vendor invoice three-way match | 7 | 10,500 |
| EX-004 | Exception triage and routing | 4 | 3,200 |

Each agent loads only its Zero-Drift Prompt Spec. Each has 4–8 tools, not 27. Each has verification gates testable in isolation.

### Orchestration, Not Monolith

Micro-agents do not operate in silos. An **Orchestrator Agent** (Chapter 5's focus) routes incoming triggers to specialized workers:

```
INBOUND: Email to ops@ with subject "Invoice discrepancy - PO 8842"
ORCHESTRATOR: Classify → RECONCILIATION_EXCEPTION
ROUTE → RC-001 (reconciliation context loaded)
RC-001: Execute TCAV → ESCALATED (variance > threshold)
ROUTE → EX-004 (exception triage)
EX-004: Assign priority, notify human, link artifacts
```

The orchestrator is lightweight—classification and routing, not domain execution. Domain expertise lives in specialized workers with deep, narrow prompt specs.

### Role Specialization in Practice

For the distributor running $847,000 in hidden motion costs, role specialization deploys as:

**Week 1:** RC-001 (reconciliation) only. Highest ROI, clearest TCAV.
**Week 2:** QA-002 (quote assembly) after RC-001 reaches 95% auto-completion.
**Week 3:** EX-004 (exception triage) to handle escalations from RC-001 and QA-002.
**Week 4:** Orchestrator routing between deployed workers.

Each agent ships with its own sandbox tests, token budget, and KPI dashboard. Failure in QA-002 does not degrade RC-001 production throughput.

Specialization is not overhead. It is **fault isolation and optimization headroom**—the same reason your warehouse has receiving clerks and shipping clerks, not one person doing everything.

---

## The Agent Blueprint Canvas: A Modular Layout for Architecting Any Back-Office Digital Worker

The Agent Blueprint Canvas is the one-page architecture document that translates any SOP into a deployable agent specification. Complete this canvas before writing code or prompts. Review it with operations stakeholders. Version-control it in Git alongside your prompt specs.

### Canvas Layout

```
┌─────────────────────────────────────────────────────────────────┐
│ AGENT BLUEPRINT CANVAS                                          │
├─────────────────────────────────────────────────────────────────┤
│ AGENT ID: RC-001                                                │
│ NAME: Receiving Reconciliation Agent                            │
│ VERSION: 1.2.0                                                  │
│ OWNER: Operations / Jane Chen                                   │
│ STATUS: Production                                              │
├─────────────────────────────────────────────────────────────────┤
│ TRIGGER SPECIFICATION                                           │
│ Event: Email → receiving@ + PDF/image attachment                │
│ Filter: Exclude thread replies without new attachments            │
│ Volume: ~200/week | Peak: 40/day (Mon-Tue)                       │
├─────────────────────────────────────────────────────────────────┤
│ TCAV MATRIX                                                     │
│ T: [trigger spec above]                                         │
│ C: vendor_profile, po_record, uom_table, [optional: history]    │
│ A: parse → query → compare → update/create → summarize          │
│ V: line coverage 100%, variance count match, total ±0.1%        │
├─────────────────────────────────────────────────────────────────┤
│ TOOL INVENTORY                                                  │
│ READ:  parse_document, query_erp_po, query_vendor_profile       │
│ WRITE: update_receipt_status, create_exception_ticket           │
│ HITL:  modify_po_quantity, send_vendor_communication            │
├─────────────────────────────────────────────────────────────────┤
│ CONTEXT BUDGET                                                  │
│ Zone 1 (System): 1,400 tokens                                   │
│ Zone 2 (State): 400 tokens                                      │
│ Zone 3 (Payload): 2,000–6,000 tokens                            │
│ Zone 4 (Log): 1,500 tokens (compressed)                         │
│ Target total: <12,000 tokens/execution                          │
├─────────────────────────────────────────────────────────────────┤
│ VERIFICATION GATES                                              │
│ Step gates: post-COMPARE (classification complete)              │
│ Terminal gates: totals, exception count, audit log              │
│ Escalation triggers: timeout, missing context, threshold breach   │
├─────────────────────────────────────────────────────────────────┤
│ ERROR HANDLING                                                  │
│ Retries: 2/step | Sandbox: 4-layer validation                   │
│ Escalation payload: EscalationSchema v1.1                       │
├─────────────────────────────────────────────────────────────────┤
│ KPI TARGETS                                                     │
│ Auto-completion rate: ≥95%                                      │
│ False exception rate: <2%                                       │
│ Avg execution time: <45 seconds                                 │
│ Token cost/execution: <$0.08                                      │
├─────────────────────────────────────────────────────────────────┤
│ DEPENDENCIES                                                    │
│ MCP: erp-server, ticketing-server, document-store               │
│ External: ERP API v3.2, Zendesk integration                     │
├─────────────────────────────────────────────────────────────────┤
│ REGRESSION SUITE                                                │
│ Golden path: 15 cases | Edge: 8 cases | Failure injection: 5    │
│ Last run: 2025-09-04 | Pass rate: 100%                          │
└─────────────────────────────────────────────────────────────────┘
```

### Canvas Completion Guide

**Section 1: Identity and Ownership**
Every agent has an ID (RC-001), a human owner accountable for performance, and a version number incremented on prompt or tool changes.

**Section 2: Trigger Specification**
Quantify volume and peak patterns. Agents sized for average load fail on Monday morning receiving spikes.

**Section 3: TCAV Matrix**
One-line summary per component. Detailed spec lives in the Zero-Drift Prompt Spec; canvas captures the contract.

**Section 4: Tool Inventory**
Explicit READ/WRITE/HITL classification. If a tool is not listed, the agent cannot call it—permission enforcement at architecture level.

**Section 5: Context Budget**
Token targets per zone. Exceeding budget triggers engineering review, not silent cost creep.

**Section 6: Verification Gates**
Minimum gates required for production deployment. Agents without terminal verification defined stay in sandbox.

**Section 7: Error Handling**
Cross-reference to sandbox configuration and escalation schema version.

**Section 8: KPI Targets**
Measurable success criteria agreed with operations before deployment—not aspirational, contractual.

**Section 9: Dependencies**
MCP servers, API versions, external integrations. When ERP upgrades to v3.3, canvas flags affected agents.

**Section 10: Regression Suite**
Last run date and pass rate. No agent deploys to production with failing regression.

### Field Playbook: Quote Assembly Agent Canvas Summary

For QA-002 (B2B Quote Assembly):

```
TRIGGER: Email to quotes@ OR CRM webhook (new_opportunity, source=RFQ)
CONTEXT: customer_profile, pricing_tier, product_catalog, discount_policy
ACTIONS: parse_rfq → match_skus → calculate → create_opportunity → generate_pdf
VERIFY: all RFQ line items mapped or flagged, margin ≥18%, totals match catalog
HITL: send_quote (always), discounts >12% (policy exception)
KPI: 90% auto-assembly, <5% SKU mismatch escalation, <$0.12/execution
```

Two canvases. Two agents. Two independent regression suites. One orchestrator routing inbound work.

The canvas is **Artifact A** in this book's Implementation Toolkit (Appendix). Copy it. Complete it for your first workflow. Review it with the operator who currently performs the task manually—they will catch ambiguities your engineering team misses.

---

## Chapter Summary

- The **TCAV Protocol** (Trigger-Context-Action-Verification) decomposes any SOP into machine-executable logic with zero interpretive gaps.
- **Zero-Drift Prompt Specs** with XML zoning (identity, protocol, context, tools, verification, error handling, output) create versionable, testable agent contracts that resist behavioral drift.
- **Role-specialized micro-agents** outperform monolithic generalists through fault isolation, focused tool sets, and independent optimization—coordinated by lightweight orchestrators.
- The **Agent Blueprint Canvas** is the one-page architecture document connecting SOPs to deployable agents, with KPI targets, regression requirements, and dependency tracking.

---

## Action Checklist

- [ ] Select your highest-priority workflow from the 48-Hour Feasibility Audit
- [ ] Complete a full TCAV matrix for that workflow with explicit verification gates
- [ ] Draft a Zero-Drift Prompt Spec with all seven XML zones
- [ ] Complete an Agent Blueprint Canvas (use the layout in this chapter)
- [ ] Review the canvas with the human operator who currently performs the task—document their corrections
- [ ] Identify whether this workflow is one agent or should split into micro-agents (check for tool sprawl and conflicting verification)
- [ ] Commit canvas and prompt spec to version control with agent ID and version number

---

## The Bridge

Act I is complete. You understand the economics of the digital shop floor, Claude's technical advantages for operational work, and the architecture that transforms SOPs into executable agent logic.

Act II builds the factory. Chapter 4 connects your agents to the real world through the Model Context Protocol—granting read/write access to CRMs, databases, file systems, and internal APIs. Your reconciliation agent stops parsing documents into the void and starts querying your actual ERP. Your quote agent writes to your actual CRM.

The blueprints are drawn. Now we wire the machines to your building.
