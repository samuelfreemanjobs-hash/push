# security_sentinel workflow

## Scan triggers

- New skill install, PR touching auth, env, or CI
- User pastes config or webhook URL

## Checklist

- No tokens in repo or skill bundles
- Webhooks use secrets; least privilege on GitHub tokens
- ClawHub skills: metadata `requires` matches real deps

## Output

Severity-ordered findings; stop on critical until human ack.