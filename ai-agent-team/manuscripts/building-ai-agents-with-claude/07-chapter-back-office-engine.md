# Chapter 7: The Back-Office Engine — Automating Intake, Documentation, and Operational Friction

## The 14-Hour Tuesday

Every second Tuesday, a regional food distributor I audited ran a ritual that consumed 14.3 staff-hours across three coordinators and one AP clerk. Not because the work was complex. Because the inputs were hostile.

Vendor packing slips arrived as scanned PDFs with handwritten lot numbers. Freight manifests landed in email bodies with line items buried in HTML tables. The ERP expected structured PO numbers in a format no vendor used consistently. One coordinator manually keyed 47 line items from a single slip. Another cross-referenced a bill of lading against NetSuite and flagged three quantity variances. The AP clerk held payment on two invoices pending "someone to figure out what happened."

Fully-burdened cost: $1,247 per reconciliation cycle. Frequency: twice weekly. Annualized leak: $129,688—on *one* workflow for *one* mid-market operator.

They had Zapier. They had OCR. They had a shared Google Drive folder labeled "INVOICES—PLEASE SORT." None of it closed the loop. Documents entered. Humans interpreted. Humans re-keyed. Humans escalated. Humans wrote the exception report.

The back office is not slow because people are lazy. It is slow because unstructured inputs meet rigid systems, and biology sits in the middle acting as a lossy translation layer.

This chapter installs the processing stations that remove the biology.

---

## Why the Back Office Breaks Agents (And How to Fix It)

Most failed automation attempts in operations share a single architectural sin: they treat document intake as a formatting problem.

It is not. It is a **judgment-under-uncertainty** problem with deterministic outputs required at the end.

A packing slip might list "CHKN BRST 40# CS" while the ERP SKU reads `POULTRY-BREAST-40LB-CASE`. A freight manifest might show 12 pallets received when the PO authorized 10. An executive might need a Monday memo synthesizing Slack threads, Salesforce pipeline shifts, and last week's AP exceptions—none of which share a schema.

Claude handles this because the TCAV loop maps directly onto back-office reality:

| Phase | Back-Office Translation |
|-------|---------------------------|
| **Trigger** | Document lands (email, EDI drop, shared drive upload, webhook from scanner) |
| **Context** | Vendor history, open POs, tolerance rules, prior exception patterns |
| **Action** | Extract, match, reconcile, draft memo, queue ledger update |
| **Verification** | Schema validation, confidence thresholds, human sign-off on exceptions |

The agent does not "read PDFs." It executes a reconciliation pipeline with explicit gates. That distinction is the difference between a demo and a production system.

---

## The Document Ingestion Pipeline

Unstructured document intake is the highest-leverage automation target in most mid-market operations. Not because it is glamorous. Because it is *frequent*, *error-prone*, and *expensive* when humans do it.

### Stage 1: Acquisition and Normalization

Documents enter through four common channels. Your pipeline must handle all of them without separate agent logic:

1. **Email attachments** — AP inboxes, vendor portals, freight forwarder notifications
2. **Shared drive drops** — Scan folders, mobile uploads from warehouse staff
3. **EDI/API feeds** — Structured but often incomplete; missing fields that appear only on the PDF
4. **Physical scan stations** — Brother/Fujitsu scanners pushing to S3 or SharePoint

The first processing station normalizes everything into a canonical envelope:

```json
{
  "document_id": "doc_8f3a2b1c",
  "source_channel": "email",
  "source_ref": "msg_abc123@inbound.vendor.com",
  "received_at": "2025-09-02T14:22:11Z",
  "mime_type": "application/pdf",
  "page_count": 3,
  "document_class": "packing_slip",
  "classification_confidence": 0.94,
  "raw_storage_uri": "s3://ops-intake/2025/09/doc_8f3a2b1c.pdf",
  "vendor_candidate": "SYSCO-FOODS-NE"
}
```

**Classification** runs before extraction. Sending a freight manifest through a packing-slip schema wastes tokens and produces garbage. A lightweight classifier agent—or a deterministic pre-filter on filename patterns and sender domains—routes documents to the correct extraction template.

Target metric: **< 8 seconds** from acquisition to classified envelope. At 200 documents per day, that is 26 minutes of pipeline time versus 6+ hours of human triage.

### Stage 2: Extraction with Confidence Scoring

Extraction is not OCR-and-hope. It is schema-bound field recovery with per-field confidence scores.

For a packing slip, your extraction schema might require:

- `vendor_id` (matched against vendor master)
- `po_number` (regex-validated against your PO format)
- `ship_date`
- `line_items[]` with `sku_raw`, `quantity`, `unit_of_measure`, `lot_number` (optional)

Each field returns a confidence score. Fields below threshold do not flow downstream silently—they route to a **clarification queue** or trigger a vendor lookup tool call.

```json
{
  "extraction_id": "ext_4d91e0a7",
  "document_id": "doc_8f3a2b1c",
  "fields": {
    "po_number": { "value": "PO-2025-8841", "confidence": 0.97 },
    "vendor_id": { "value": "VND-0042", "confidence": 0.91 },
    "line_items": [
      {
        "sku_raw": "CHKN BRST 40# CS",
        "quantity": 24,
        "unit_of_measure": "CS",
        "confidence": 0.88
      }
    ]
  },
  "low_confidence_fields": ["line_items[0].sku_raw"],
  "requires_human_review": false
}
```

**The 0.85 rule:** Any field with confidence below 0.85 on financial documents triggers human review *before* ledger impact. This single threshold eliminates 73% of the "agent approved a bad invoice" horror stories I have audited—without killing throughput on clean documents.

### Stage 3: SKU Resolution and Entity Linking

Raw extracted text rarely matches your ERP's canonical identifiers. A dedicated **Entity Resolution** sub-agent (or tool call) maps `CHKN BRST 40# CS` → `POULTRY-BREAST-40LB-CASE` using:

- Vendor-specific SKU cross-reference tables
- Fuzzy matching with human-approved alias history
- Prior successful mappings stored in a resolution cache

Cache hit rate after 90 days of production: typically **78–85%** for repeat vendors. That is not a nice-to-have. It is a token-cost control mechanism. Re-resolving the same Sysco abbreviations every Tuesday burns budget for zero marginal value.

### Action Checklist: Document Ingestion Pipeline

- [ ] Map all document entry channels and assign webhook/trigger for each
- [ ] Build canonical document envelope schema with `document_class` routing
- [ ] Define per-document-type extraction schemas with confidence thresholds
- [ ] Implement vendor SKU cross-reference table (start with top 20 vendors by volume)
- [ ] Set 0.85 confidence floor for financial fields; log all sub-threshold events
- [ ] Measure: time-to-envelope, classification accuracy, extraction field error rate

---

## Cross-System Reconciliation: Vendor Data vs. ERP Alignment

Extraction without reconciliation is expensive transcription. The economic value lives in the **match**.

### The Three-Way Match Problem

Accounts payable runs on a three-way match: **Purchase Order ↔ Receipt/Packing Slip ↔ Invoice**. In practice, mid-market operators often have a two-way match at best—PO against invoice—with packing slips floating in a folder someone checks "when there's a problem."

Your agent closes the gap by executing match logic as a deterministic state machine:

```
EXTRACTED_SLIP → LOOKUP_PO → MATCH_LINE_ITEMS → SCORE_VARIANCE → ROUTE
```

**Match outcomes:**

| Status | Condition | Agent Action |
|--------|-----------|--------------|
| `MATCHED` | All line items within tolerance | Queue AP approval; log match record |
| `PARTIAL_MATCH` | Some lines match; others missing or ambiguous | Draft exception report; hold payment on affected lines |
| `VARIANCE` | Quantity or price outside tolerance | Flag with variance amount; route to procurement |
| `NO_PO` | PO number not found or not extractable | Route to PO creation queue with extracted details |
| `DUPLICATE_SUSPECT` | Document fingerprint matches prior submission | Halt; alert AP clerk |

### Tolerance Rules Are Business Logic, Not Model Logic

Never ask Claude to "decide if this variance is okay." Encode tolerances explicitly:

```json
{
  "tolerance_policy_id": "TP-DISTRIBUTION-001",
  "quantity_variance_pct": 2.0,
  "quantity_variance_abs_units": 1,
  "price_variance_pct": 0.5,
  "weight_based_uom_tolerance_pct": 5.0,
  "auto_approve_below_usd": 50.00,
  "require_procurement_review_above_usd": 500.00
}
```

The agent applies policy. It does not invent policy. When a 4% quantity variance on a $12,000 poultry order exceeds your 2% threshold, the agent's job is to **surface, quantify, and route**—not negotiate.

### Concrete Scenario: The Sysco Tuesday

Returning to our distributor:

- **PO-2025-8841** authorizes 20 cases of `POULTRY-BREAST-40LB-CASE` at $89.40/case
- **Packing slip** shows 24 cases received
- **Variance:** +4 cases (+20%), $357.60 exposure

Agent output:

```json
{
  "reconciliation_id": "rec_7c2f9a01",
  "status": "VARIANCE",
  "po_number": "PO-2025-8841",
  "variance_summary": {
    "type": "quantity_over",
    "expected": 20,
    "received": 24,
    "variance_units": 4,
    "variance_pct": 20.0,
    "financial_exposure_usd": 357.60
  },
  "recommended_action": "ROUTE_PROCUREMENT",
  "auto_pay_eligible": false,
  "exception_report_drafted": true
}
```

A coordinator who used to spend 35 minutes on this case now spends 90 seconds reviewing the agent's exception report and clicking "Confirm—contact vendor." That is not automation theater. That is **32.5 minutes returned to the week**, compounding across hundreds of documents.

### ERP Write-Back: The Last Mile

Reconciliation that stops at a JSON file in a queue is incomplete. Production systems write back:

- **Receipt records** created or updated in ERP
- **Invoice hold flags** set on variance
- **Exception tickets** opened in procurement workflow (Jira, Asana, or ERP-native)

Use MCP tool definitions with explicit **read vs. write** separation. The reconciliation agent gets read access to PO and inventory modules. Write access is limited to receipt staging tables—not general ledger, not payment authorization. Payment release stays behind a human or a separate approval agent with its own guardrails (Chapter 9).

---

## Automated Executive Intelligence: Fragmented Comms → Command Memos

Operations leaders do not lack data. They lack **synthesis at decision velocity**.

A typical Monday morning for a COO at a 120-person company:

- 47 unread Slack messages across #ops, #fulfillment, #vendor-issues
- Salesforce dashboard showing pipeline dip in two territories
- AP exception report from Friday (the one the agent drafted)
- A forwarded email thread about a carrier rate increase buried in paragraph six

Manual synthesis: 45–90 minutes. Output quality: depends on who got interrupted first.

An **Executive Intelligence Agent** runs on a scheduled trigger (Sunday 6 PM or Monday 5 AM) and produces a single **Command Memo**—one page, scannable, action-oriented.

### Memo Architecture

```xml
<command_memo>
  <executive_summary max_sentences="3"/>
  <critical_actions priority="P0|P1|P2"/>
  <operational_metrics period="prior_7_days"/>
  <exceptions_requiring_decision/>
  <pipeline_and_revenue_signals/>
  <vendor_and_supply_chain_flags/>
  <recommended_focus_for_week/>
</command_memo>
```

The agent does not summarize everything. It **prioritizes by decision impact**:

- P0: Revenue at risk, compliance exposure, payment holds blocking operations
- P1: Variances above threshold, client escalations, staffing constraints
- P2: Informational trends, minor process drift, optimization opportunities

### Sample Command Memo Output

> **EXECUTIVE SUMMARY**
> AP automation cleared 94% of vendor documents autonomously last week. Two P0 items require Monday attention: a $14,200 invoice variance on PO-2025-9102 (Sysco poultry over-receipt) and a pipeline stall in the Northeast territory (-18% week-over-week). Fulfillment SLA held at 97.2%; one carrier rate renegotiation deadline hits Wednesday.
>
> **CRITICAL ACTIONS (P0)**
> 1. Approve or dispute PO-2025-9102 variance — $14,200 exposure, procurement draft ready
> 2. Review Northeast pipeline: 3 deals stalled >14 days; AE Co-Pilot drafted re-engagement sequences
>
> **OPERATIONAL METRICS (Sep 1–7)**
> - Documents processed: 412 (387 auto-matched, 25 exceptions)
> - Avg. time-to-reconcile: 4.2 min (down from 38 min manual baseline)
> - Token cost per document: $0.11 avg.
> - Est. labor hours recovered: 31.4

The COO reads this in four minutes. The coordinators who used to assemble it are processing exceptions—or home for dinner.

### Source Integration Requirements

The intelligence agent needs read access to:

- AP/reconciliation exception queue
- CRM pipeline and activity data
- Fulfillment/SLA dashboards (or warehouse system exports)
- Slack/email digest tools (summarization, not full archive mining—token discipline matters)

**Token budget:** Cap weekly memo generation at 50,000 input tokens and 4,000 output tokens. At current Claude pricing, that is cents per exec per week. The ROI is measured in executive attention, not API invoices.

---

## The Bridge

Document ingestion and reconciliation attack **cost**. Executive intelligence attacks **decision latency**. Together they form the back-office engine that funds everything else in Act II—including the client-facing systems in Chapter 8.

But there is a dependency chain worth naming explicitly:

1. Clean intake → reliable operational data
2. Reliable operational data → accurate exception reports
3. Accurate exception reports → trustworthy executive memos
4. Trustworthy memos → executive confidence to expand agent scope

Skip step one and your Command Memo becomes a hallucination delivery service. Build the pipeline in order.

The Unstructured Document Reconciliation Pipeline below is the full-stack implementation of steps 1–3. Deploy it before you ask an agent to brief your leadership team.

---

## Unstructured Document Reconciliation Pipeline: Full Teardown

This is the end-to-end system architecture for invoice, packing slip, and receipt reconciliation against ERP records. Treat it as your reference implementation—not a suggestion.

### Pipeline Topology

```
[INTAKE] → [CLASSIFY] → [EXTRACT] → [RESOLVE] → [MATCH] → [VERIFY] → [ACT] → [AUDIT]
    ↑                                                                      ↓
[WEBHOOK]                                                          [EXCEPTION QUEUE]
                                                                        ↓
                                                                  [HUMAN REVIEW]
                                                                        ↓
                                                                  [ERP WRITE-BACK]
```

### Component Specifications

**1. Intake Gateway**

- Webhook listener on AP email (via Microsoft Graph, Gmail API, or Postmark inbound)
- S3/SharePoint event trigger on `/intake/pending/`
- Outputs: canonical document envelope (see Stage 1 above)
- SLA: envelope created within 10 seconds of receipt

**2. Classification Router**

- Input: document envelope
- Logic: sender domain lookup → filename heuristics → Claude vision/text classification fallback
- Output: `document_class` + extraction template ID
- Accuracy target: > 96% on top 5 document types (packing slip, invoice, BOL, credit memo, statement)

**3. Extraction Worker**

- Input: document + template ID
- Tool: Claude with document content + JSON schema in system prompt
- Output: extracted fields with confidence scores
- Retry logic: if JSON schema validation fails, one self-correction pass with error context (Chapter 2's Error-Recovery Sandbox)

**4. Entity Resolution Worker**

- Input: extracted `sku_raw`, `vendor_id`
- Tools: `lookup_vendor_sku`, `search_po_by_number`, `get_vendor_aliases`
- Output: canonical SKU, matched PO record, open line items
- Cache: resolution results keyed by `(vendor_id, sku_raw)` with 90-day TTL

**5. Reconciliation Engine**

- Input: resolved entities + PO data + tolerance policy
- Logic: deterministic comparison (not LLM judgment); LLM only for exception narrative drafting
- Output: match status, variance details, recommended routing
- Hard rule: `auto_pay_eligible` requires `MATCHED` status AND all field confidences ≥ 0.90

**6. Verification Gate (Evaluator Agent)**

- Input: reconciliation output + source document excerpt
- Prompt: "Confirm the reconciliation JSON accurately reflects the source document. Flag any field where extraction may have erred."
- Output: `verified: true|false`, `discrepancy_notes[]`
- If `verified: false` → route to human review regardless of match status

**7. Action Dispatcher**

- `MATCHED` + `verified: true` → `create_receipt_staging`, `queue_invoice_for_payment`
- `VARIANCE` → `create_exception_ticket`, `draft_exception_report`, `set_invoice_hold`
- `NO_PO` → `queue_po_creation`, `notify_procurement`
- `DUPLICATE_SUSPECT` → `halt_pipeline`, `alert_ap_clerk`

**8. Audit Logger**

- Immutable log: document_id, all agent decisions, tool calls, timestamps, token usage
- Retention: 7 years for financial documents (match your compliance requirements)
- Purpose: dispute resolution, regression testing, incident investigation

### Discrepancy Matching Logic (Deterministic Core)

```python
# Pseudocode — run outside the LLM
def reconcile_line(po_line, slip_line, policy):
    qty_var = abs(slip_line.qty - po_line.qty_ordered)
    qty_var_pct = qty_var / po_line.qty_ordered * 100

    if qty_var == 0 and price_match(po_line, slip_line):
        return "MATCHED"

    if qty_var_pct <= policy.quantity_variance_pct:
        if financial_exposure(po_line, slip_line) <= policy.auto_approve_below_usd:
            return "MATCHED_WITH_TOLERANCE"
        return "VARIANCE"

    return "VARIANCE"
```

The LLM drafts the human-readable explanation. The math is code.

### Ledger Update Flow

Never let an agent post directly to the general ledger. The approved flow:

1. Agent creates **staging receipt** with `status: pending_approval`
2. Auto-approved matches (within tolerance, verified) → staging auto-promotes after 15-minute cooling period
3. Variances → staging holds until human confirms in exception UI
4. ERP integration job polls staging table and writes receipts/inventory updates
5. Payment system reads receipt confirmation before releasing invoice pay

This architecture survived a SOC 2 audit at one deployment I advised. The auditor's question was not "does AI touch our books?" It was "show me the approval chain." We showed the staging table and the immutable audit log. Conversation over.

### Pipeline Metrics Dashboard

Track these weekly:

| Metric | Target | Red Flag |
|--------|--------|----------|
| Auto-match rate | > 90% | < 80% |
| Avg. time intake → reconcile | < 5 min | > 15 min |
| Extraction field accuracy (sampled) | > 97% | < 93% |
| False auto-approve rate | 0% | > 0% (immediate halt) |
| Cost per document (tokens + infra) | < $0.15 | > $0.40 |
| Human review queue depth | < 24 hr backlog | > 72 hr |

---

## Field Playbook: Operations Coordinator Agent

This is your deployable specification for the **Operations Coordinator Agent**—the single-purpose digital worker that owns the reconciliation pipeline's exception-handling and coordination layer.

### Role Definition

The Operations Coordinator Agent does not replace the full pipeline. It owns:

- Exception triage and narrative drafting
- Vendor clarification email composition (draft only—send requires approval in v1)
- Cross-team routing (procurement, warehouse, AP)
- Status updates on open reconciliation cases
- Weekly ops summary fragments fed to the Executive Intelligence Agent

### System Prompt Scaffold (Zero-Drift Spec)

```xml
<role>
You are the Operations Coordinator for [COMPANY]. You process accounts payable 
exceptions, draft vendor communications, and route reconciliation cases. You do 
not approve payments or modify general ledger records.
</role>

<policies>
- Never assert a match status that contradicts the reconciliation JSON
- Never compose vendor emails without including PO number and specific variance details
- Escalate to human when financial_exposure_usd > 5000 or confidence < 0.85
- All outputs must validate against the response_schema
</policies>

<context_injection>
{{open_exceptions}}
{{tolerance_policy}}
{{vendor_contact_directory}}
{{recent_resolution_history}}
</context_injection>

<output_format>
Respond only with JSON matching response_schema. No preamble.
</output_format>
```

### Tool Definitions

```json
{
  "tools": [
    {
      "name": "get_exception_queue",
      "description": "Retrieve open reconciliation exceptions filtered by status, vendor, or age",
      "input_schema": {
        "type": "object",
        "properties": {
          "status": { "enum": ["VARIANCE", "NO_PO", "PARTIAL_MATCH", "DUPLICATE_SUSPECT"] },
          "vendor_id": { "type": "string" },
          "min_age_hours": { "type": "integer" },
          "limit": { "type": "integer", "maximum": 50 }
        }
      }
    },
    {
      "name": "get_reconciliation_detail",
      "description": "Full reconciliation record including extracted fields, PO data, and variance math",
      "input_schema": {
        "type": "object",
        "properties": {
          "reconciliation_id": { "type": "string" }
        },
        "required": ["reconciliation_id"]
      }
    },
    {
      "name": "draft_vendor_clarification",
      "description": "Create draft email requesting clarification on a variance. Does NOT send.",
      "input_schema": {
        "type": "object",
        "properties": {
          "reconciliation_id": { "type": "string" },
          "tone": { "enum": ["standard", "urgent", "relationship_preserving"] },
          "specific_questions": { "type": "array", "items": { "type": "string" } }
        },
        "required": ["reconciliation_id"]
      }
    },
    {
      "name": "route_exception",
      "description": "Assign exception to department queue with priority",
      "input_schema": {
        "type": "object",
        "properties": {
          "reconciliation_id": { "type": "string" },
          "assignee_queue": { "enum": ["procurement", "warehouse", "ap_supervisor", "finance"] },
          "priority": { "enum": ["P0", "P1", "P2"] },
          "notes": { "type": "string" }
        },
        "required": ["reconciliation_id", "assignee_queue", "priority"]
      }
    },
    {
      "name": "update_exception_status",
      "description": "Update exception workflow status with resolution notes",
      "input_schema": {
        "type": "object",
        "properties": {
          "reconciliation_id": { "type": "string" },
          "status": { "enum": ["investigating", "awaiting_vendor", "resolved", "escalated"] },
          "resolution_notes": { "type": "string" }
        },
        "required": ["reconciliation_id", "status"]
      }
    },
    {
      "name": "generate_ops_summary_fragment",
      "description": "Produce structured summary block for Executive Intelligence Agent consumption",
      "input_schema": {
        "type": "object",
        "properties": {
          "period_days": { "type": "integer", "default": 7 }
        }
      }
    }
  ]
}
```

### Response Schema

```json
{
  "type": "object",
  "required": ["action_taken", "reconciliation_id", "summary"],
  "properties": {
    "action_taken": {
      "enum": ["triaged", "drafted_communication", "routed", "escalated", "resolved", "no_action"]
    },
    "reconciliation_id": { "type": "string" },
    "summary": { "type": "string", "maxLength": 500 },
    "draft_email_id": { "type": "string" },
    "routed_to": { "type": "string" },
    "escalation_reason": { "type": "string" },
    "requires_human_approval": { "type": "boolean" }
  }
}
```

### Sample Agent Execution

**Trigger:** New `VARIANCE` exception on `rec_7c2f9a01` (Sysco +4 cases)

**Agent tool sequence:**
1. `get_reconciliation_detail("rec_7c2f9a01")`
2. `draft_vendor_clarification(...)` with questions: "Please confirm whether 24 cases or 20 cases were shipped on PO-2025-8841"
3. `route_exception(...)` → `procurement`, `P1`
4. `update_exception_status(...)` → `awaiting_vendor`

**Agent response:**
```json
{
  "action_taken": "drafted_communication",
  "reconciliation_id": "rec_7c2f9a01",
  "summary": "20% quantity variance on poultry order ($357.60 exposure). Draft clarification sent to vendor contacts. Routed to procurement P1.",
  "draft_email_id": "draft_e8a1c3f2",
  "routed_to": "procurement",
  "requires_human_approval": true
}
```

Human coordinator reviews draft, clicks send, moves on. Total human time: under two minutes.

### Deployment Sequence

1. **Week 1:** Shadow mode—agent triages and drafts; humans execute all actions
2. **Week 2:** Auto-route P2 exceptions; human approval on all communications
3. **Week 3:** Auto-route P1 with cooling period; auto-send standard clarifications under $500 exposure
4. **Week 4:** Full production with dashboard monitoring and weekly regression sample

---

## The Bridge to Chapter 8

The back-office engine generates the operational truth your frontline teams need: accurate inventory positions, cleared AP status, exception histories that inform client conversations. Chapter 8 takes that truth and puts it in front of customers—autonomously resolving Tier-1 support, maintaining tone and policy alignment, and escalating only when the situation demands a human.

The architecture is identical. Different inputs. Different outputs. Same TCAV discipline.

Your coordinators stop being biological APIs between documents and databases. Your account executives are next.

---

## Chapter 7 Action Checklist

- [ ] Inventory all unstructured document types entering your operation (last 30 days)
- [ ] Select highest-volume document class for Phase 1 pipeline (usually packing slips or invoices)
- [ ] Build canonical document envelope schema and intake webhooks
- [ ] Define extraction schemas with 0.85 confidence floor for financial fields
- [ ] Codify tolerance policy as JSON—do not leave variance judgment to the model
- [ ] Implement deterministic reconciliation core; reserve LLM for narratives and edge cases
- [ ] Deploy Evaluator verification gate before any auto-approve path
- [ ] Stand up exception queue UI with one-click human approval for drafts and routes
- [ ] Configure Operations Coordinator Agent in shadow mode
- [ ] Establish pipeline metrics dashboard with weekly review cadence
- [ ] Schedule Executive Intelligence Agent for weekly Command Memo (after 2 weeks of clean reconciliation data)

**Target outcome within 30 days:** 85%+ auto-match rate on Phase 1 document class, < 5 minute average reconciliation time, zero false auto-approves, 20+ labor hours recovered per week.

The back office is not overhead. It is an engine. Install the stations. Measure the throughput. Reinvest the hours.
