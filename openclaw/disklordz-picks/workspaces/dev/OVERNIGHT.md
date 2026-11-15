# DEV pod — overnight branch + app/VST (OC pick)

## Allowed overnight

- Branch `overnight/YYYY-MM-DD-slug` only
- `npm run build` / `tsc` must pass before notify
- `gh pr create --draft` when `gh` available
- Targets: `disklordz-command-center`, `push`, Instruments website, non-main VST fixes

## Forbidden

- Merge to main, production deploy, Stripe live, vault merge without operator

## VST

FORGE-style checks in PR body: pluginval reminder, Windows CI link.

## Hermes Codex

If Hermes ran `/bg` Codex overnight, dev pod morning job: link PR, run REVIEW checklist, Telegram summary.