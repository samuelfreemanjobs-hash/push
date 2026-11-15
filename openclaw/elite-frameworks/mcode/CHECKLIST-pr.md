# MCode — pull request checklist

- [ ] Title + body explain **what** and **why** (complete sentences)
- [ ] Scope matches STACK-FROZEN / opt-in doc if touching agent frameworks
- [ ] Waza `run_all.sh` green for agent-touched paths
- [ ] Security: storefront changes → `vercel-security-review/checklist-storefront.md`
- [ ] Paid surface: Strix skill or `security-review` Task if user asked
- [ ] No secrets, `.env`, or customer PII in diff
- [ ] Human gate tokens respected (`HUMAN_GATE:*` in AGENTS.md)