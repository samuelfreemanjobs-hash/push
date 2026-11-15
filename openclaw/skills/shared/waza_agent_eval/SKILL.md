---
name: waza_agent_eval
description: Run repeatable artifact evals after agent coding sessions.
version: 1.0.0
user-invocable: true
---

# Waza agent eval

## When to use

- After Cloud Agent or OpenClaw dev pod merges
- Before marking CoworkOS milestone done
- CI optional hook on `openclaw/elite-frameworks/**` changes

## Run

```bash
bash openclaw/elite-frameworks/waza/evals/run_all.sh
```

Extend tests in `waza/evals/test_agent_artifacts.py` when adding new agent contracts (naming, headers, build scripts).