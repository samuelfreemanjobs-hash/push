# PM multi-agent runtime SOP

Execution-ready pipeline (5 agents). Run sequentially unless `research` and `planner` outputs are frozen.

## Agents

| Stage | Role | Input | Output |
|-------|------|-------|--------|
| 1 Planner | Mission orchestrator | User brief | `mission`, epics, success criteria |
| 2 Research | Market & user intel | Mission | `artifacts.research[]` with sources |
| 3 Requirements | Spec writer | Research + mission | `artifacts.requirements[]` |
| 4 Prioritization | RICE / MoSCoW | Requirements | `artifacts.backlog[]` scored |
| 5 Output/QA | PRD + gate | All prior | `documents.prd`, QA pass/fail |

## Gates

- **Confidence < 0.70** → rerun Research; do not prioritize.
- **Any P0 without acceptance criteria** → halt; return to Requirements.
- **External writes** (Slack post, Linear issue, Etsy publish) → human confirms.

## Triggers

| Trigger | Action |
|---------|--------|
| New GitHub issue on `push` | Add row to `pm/backlog.yaml`, post summary to `#pm-product-matrix` |
| `npm run pm:sync` | Refresh backlog from `gh issue list` |
| Cloud Agent "PM autonomy" | Pick highest P0 with `autonomous_next` items |

## Cursor command

`/pm build-plan [product]` — load `skills/pm-agent/SKILL.md`, emit build-ready plan under `docs/plans/`.
