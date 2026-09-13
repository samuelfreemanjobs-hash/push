# Appendix: The Implementation Toolkit

Production-ready artifacts referenced throughout *Building AI Agents with Claude*. Copy, adapt, deploy.

---

## Artifact A: The Agent Blueprint Canvas

**Purpose:** Translate any human SOP into a machine-executable agent specification before writing a single line of code.

### Section 1: Operational Context

| Field | Your Input |
|-------|-----------|
| **Agent Name** | e.g., `operations-reconciliation-coordinator` |
| **Business Function** | e.g., Vendor invoice & packing slip reconciliation |
| **Trigger Event** | e.g., PDF uploaded to `inbox@vendor-docs.company.com` |
| **Trigger Frequency** | e.g., 40–80 documents/week |
| **Current Owner (Human)** | e.g., Operations Coordinator |
| **Current Cycle Time** | e.g., 22 min/document average |
| **Current Error Rate** | e.g., 8% require rework |
| **Fully-Burdened Hourly Cost** | e.g., $62/hr |

### Section 2: TCAV Decomposition

| Phase | Definition | Your Specification |
|-------|-----------|-------------------|
| **TRIGGER** | What event wakes the agent? | Webhook on email ingest / file upload / CRM status change |
| **CONTEXT** | What data must the agent load? | ERP open POs, vendor master, tolerance policy JSON, last 30 days exceptions |
| **ACTION** | What steps execute in sequence? | 1) Extract line items 2) Match PO 3) Flag variances 4) Stage ledger entry 5) Draft exception report |
| **VERIFICATION** | What gates block bad output? | JSON schema validation, confidence ≥ 0.85, dollar variance < $500 auto-approve threshold |

### Section 3: Tool Inventory

| Tool Name | Type | Read/Write | System |
|-----------|------|-----------|--------|
| `query_open_pos` | MCP/SQL | Read | ERP |
| `get_vendor_master` | MCP/API | Read | Vendor DB |
| `stage_ledger_entry` | MCP/API | Write | Accounting |
| `send_exception_report` | MCP/Email | Write | SMTP |

### Section 4: HITL Thresholds

| Action | Autonomous | Human Required |
|--------|-----------|----------------|
| Extract & match | ✅ | |
| Variance < $500 | ✅ Auto-approve | |
| Variance $500–$5,000 | | ✅ Queue for review |
| Variance > $5,000 | | ✅ Block + alert CFO |
| Public vendor email | | ✅ Always |

### Section 5: Success Metrics (30-Day Targets)

| Metric | Baseline | Target |
|--------|----------|--------|
| Cycle time | 22 min | < 3 min |
| Error rate | 8% | < 2% |
| Human touches per doc | 1.0 | < 0.15 |
| Token cost per doc | — | < $0.25 |

---

## Artifact B: Enterprise XML Prompt Scaffolds for Claude

### B1: Zero-Drift System Prompt Template

```xml
<system_directive>
<agent_identity>
  <name>operations-reconciliation-coordinator</name>
  <role>Autonomous document reconciliation agent for vendor packing slips and invoices</role>
  <operational_mode>Deterministic TCAV execution — no conversational filler</operational_mode>
</agent_identity>

<instructions>
1. On trigger receipt, load context from authorized tools only.
2. Execute the ACTION sequence in declared order. Do not skip verification gates.
3. If confidence on any extraction field falls below 0.85, halt and emit structured exception.
4. Never send external communications without passing VERIFICATION layer 3.
</instructions>

<constraints>
- PROHIBITED: Guessing SKU mappings. Inventing PO numbers. Auto-approving variances above threshold.
- REQUIRED: Structured JSON output matching response_schema. Cite source document page for every line item.
- If tool call fails twice, emit incident payload and halt. Do not retry infinitely.
</constraints>

<tolerance_policy>
{
  "quantity_variance_pct": 2.0,
  "price_variance_usd": 500.00,
  "auto_approve_max_usd": 500.00,
  "block_threshold_usd": 5000.00
}
</tolerance_policy>

<response_schema>
{
  "type": "object",
  "required": ["status", "document_id", "line_items", "variances", "recommended_action"],
  "properties": {
    "status": {"enum": ["matched", "exception", "blocked", "incident"]},
    "document_id": {"type": "string"},
    "confidence": {"type": "number", "minimum": 0, "maximum": 1},
    "line_items": {"type": "array"},
    "variances": {"type": "array"},
    "recommended_action": {"type": "string"},
    "hitl_required": {"type": "boolean"}
  }
}
</response_schema>
</system_directive>
```

### B2: Evaluator Agent Prompt (Evaluator-Optimizer Loop)

```xml
<evaluator_directive>
<role>Quality gate agent. You do NOT execute tools. You audit worker output.</role>
<scoring_criteria>
  - Schema compliance (0-25): Valid JSON, all required fields present
  - Evidence anchoring (0-25): Every claim cites source document location
  - Policy compliance (0-25): Tolerance thresholds correctly applied
  - Action correctness (0-25): recommended_action matches variance severity
</scoring_criteria>
<pass_threshold>85</pass_threshold>
<on_fail>Return specific rewrite instructions. Do not approve.</on_fail>
</evaluator_directive>
```

---

## Artifact C: Core MCP Configuration Schemas & Webhook Templates

### C1: MCP Server Configuration (`.cursor/mcp.json` pattern)

```json
{
  "mcpServers": {
    "company-erp": {
      "command": "npx",
      "args": ["-y", "@company/mcp-erp-server"],
      "env": {
        "ERP_API_URL": "https://erp.internal.company.com/api",
        "ERP_API_KEY": "${ERP_API_KEY}",
        "READ_ONLY": "false",
        "ALLOWED_TABLES": "purchase_orders,vendors,ledger_staging"
      }
    },
    "company-storage": {
      "command": "npx",
      "args": ["-y", "@modelcontextprotocol/server-filesystem"],
      "env": {
        "ALLOWED_PATHS": "/data/vendor-inbox,/data/processed"
      }
    }
  }
}
```

### C2: Webhook Listener Template (Node.js / Express)

```javascript
import express from 'express';
import { createHash } from 'crypto';

const app = express();
app.use(express.json({ limit: '10mb' }));

const seen = new Set(); // Use Redis in production

app.post('/webhooks/agent-trigger', async (req, res) => {
  const idempotencyKey = req.headers['x-idempotency-key']
    || createHash('sha256').update(JSON.stringify(req.body)).digest('hex');

  if (seen.has(idempotencyKey)) {
    return res.status(200).json({ status: 'duplicate_ignored' });
  }
  seen.add(idempotencyKey);

  const { event_type, payload } = req.body;

  const routeMap = {
    'email.received': 'inbound-triage-router',
    'file.uploaded': 'document-ingestion-agent',
    'crm.lead.created': 'sdr-qualification-agent',
  };

  const agentId = routeMap[event_type];
  if (!agentId) {
    return res.status(422).json({ error: 'unroutable_event', event_type });
  }

  // Dispatch to agent orchestrator (queue recommended for production)
  await dispatchToAgent(agentId, {
    trigger: event_type,
    payload,
    received_at: new Date().toISOString(),
    idempotency_key: idempotencyKey,
  });

  res.status(202).json({ status: 'accepted', agent: agentId });
});

app.listen(3001);
```

### C3: Tool Definition Schema (Claude API format)

```json
{
  "name": "stage_ledger_entry",
  "description": "Stage a balanced ledger entry for human approval or auto-posting. NEVER posts directly to general ledger.",
  "input_schema": {
    "type": "object",
    "required": ["vendor_id", "po_number", "lines", "total_usd"],
    "properties": {
      "vendor_id": { "type": "string" },
      "po_number": { "type": "string" },
      "lines": {
        "type": "array",
        "items": {
          "type": "object",
          "required": ["sku", "quantity", "unit_cost"],
          "properties": {
            "sku": { "type": "string" },
            "quantity": { "type": "number" },
            "unit_cost": { "type": "number" }
          }
        }
      },
      "total_usd": { "type": "number" },
      "auto_approve_eligible": { "type": "boolean" }
    }
  }
}
```

---

## Artifact D: The 10-Minute Agent Health & Risk Audit Scorecard

Rate each dimension 1–5. **Score ≤ 3 on any dimension = do not promote to production.**

### Dimension 1: Trigger Reliability
- [ ] Trigger source is documented and monitored
- [ ] Idempotency keys prevent duplicate execution
- [ ] Failed triggers alert within 5 minutes

**Score (1-5):** ___

### Dimension 2: Context Integrity
- [ ] All required context sources listed in Blueprint Canvas
- [ ] Context load failures halt execution (no silent degradation)
- [ ] PII scope documented and minimized

**Score (1-5):** ___

### Dimension 3: Action Determinism
- [ ] TCAV action sequence is ordered and version-controlled
- [ ] Tool permissions follow least-privilege (read vs. write separated)
- [ ] No unbounded tool call loops (max retries defined)

**Score (1-5):** ___

### Dimension 4: Verification Gates
- [ ] JSON schema validation on all outputs
- [ ] Confidence thresholds enforced programmatically
- [ ] Dollar/impact thresholds route to HITL correctly

**Score (1-5):** ___

### Dimension 5: Cost Controls
- [ ] Per-execution token budget defined
- [ ] Circuit breaker configured (daily spend cap)
- [ ] Runaway loop detection active

**Score (1-5):** ___

### Dimension 6: Incident Response
- [ ] Kill switch documented and tested
- [ ] On-call contact assigned for agent failures
- [ ] Rollback procedure for prompt versions exists

**Score (1-5):** ___

### Dimension 7: Regression Coverage
- [ ] ≥ 10 golden test cases in regression suite
- [ ] Edge cases documented (illegible scans, missing fields, duplicate docs)
- [ ] CI runs regression on every prompt change

**Score (1-5):** ___

### Interpretation Matrix

| Total Score (max 35) | Status | Action |
|---------------------|--------|--------|
| 30–35 | Production-ready | Deploy with monitoring |
| 24–29 | Conditional | Fix lowest-scoring dimensions first |
| 17–23 | Sandbox only | Do not connect write tools to production |
| < 17 | Halt | Redesign agent architecture |

---

*End of Implementation Toolkit*
