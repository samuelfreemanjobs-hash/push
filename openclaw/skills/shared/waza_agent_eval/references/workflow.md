# waza_agent_eval workflow

## Triggers

- After Cloud Agent or OpenClaw dev pod changes land
- Before closing a CoworkOS milestone
- When editing `openclaw/elite-frameworks/**` agent contracts

## Steps

1. Run `bash openclaw/elite-frameworks/waza/evals/run_all.sh` from repo root.
2. Fix failures in naming, headers, or build scripts covered by `test_agent_artifacts.py`.
3. Add tests when introducing new agent artifact contracts.

## Output

Passing eval suite or a list of contract violations to fix before merge.