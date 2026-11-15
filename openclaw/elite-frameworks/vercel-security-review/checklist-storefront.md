# Vercel / Next storefront security review

Applies to `storefront/producer-storefront/` and future license portal.

- [ ] No secrets in client bundles (`NEXT_PUBLIC_*` only for non-sensitive flags)
- [ ] Lead magnet API validates email server-side; rate limit at edge when live
- [ ] Checkout webhooks verify provider signature (Stripe / Lemon Squeezy)
- [ ] Download URLs are signed or time-limited; no open S3 buckets
- [ ] CSP headers on production deploy
- [ ] Dependencies: `npm audit` triaged before paid launch
- [ ] Run Cursor **security-review** Task on branch before merging portal auth
- [ ] Optional: Strix skill `strix_vst_audit` for API routes

Reference: `openclaw/skills/shared/strix_vst_audit/`.