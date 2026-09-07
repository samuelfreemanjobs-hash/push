# Cursor Guide — Extend the Starter

Use this file with Cursor to build your Micro SaaS feature without breaking auth, billing, or webhooks.

---

## Before you prompt Cursor

1. Complete **Part 1** of `CHECKLIST.md`
2. Know your one-sentence pitch and v1 feature scope
3. Have the app running locally (`npm run dev`)

---

## Chapter mapping (*The Micro SaaS Factory*)

| Book chapter theme | Starter location |
|--------------------|------------------|
| Pick your niche | `CHECKLIST.md` |
| Landing page copy | `src/config/product.ts`, `src/components/marketing/` |
| Auth & accounts | `src/lib/supabase/`, `src/app/(auth)/` — **don't rewrite** |
| Billing | `src/lib/lemonsqueezy.ts`, `src/app/api/` — **extend carefully** |
| Core product feature | `src/app/(app)/dashboard/page.tsx` — **your main work** |
| Settings & account | `src/app/(app)/settings/page.tsx` |
| Deploy | `SETUP.md` |

---

## Starter prompt (paste into Cursor)

```
I'm building on the Micro SaaS Factory Starter.

Product:
- Name: [YOUR PRODUCT NAME]
- Customer: [WHO PAYS]
- One-sentence pitch: [FROM CHECKLIST]
- v1 dashboard feature: [ONE FEATURE ONLY]

Task:
Add my v1 feature to the dashboard at src/app/(app)/dashboard/page.tsx.
Use Supabase for data storage with a new migration in supabase/migrations/.
Match existing UI components in src/components/ui/ and styling patterns.

Rules:
- Do NOT modify auth routes, middleware, or webhook handlers
- Do NOT change Lemon Squeezy checkout/webhook logic
- Keep the paywall — only subscribed users should use the feature
- Out of scope for v1: [LIST FROM CHECKLIST]
```

---

## Common build prompts

### Replace marketing copy

```
Update src/config/product.ts and the hero component for a product that helps
[ROLE] do [TASK]. Keep the same layout and components.
```

### Add a Supabase-backed CRUD feature

```
Add a [entity name] manager to the dashboard:
- New Supabase table via migration
- List, create, and delete rows for the logged-in user only (RLS)
- Use existing Card, Button, Input components
- Respect the subscription paywall
```

### Add an AI feature

```
Add an AI [summarize/generate/transform] panel to the dashboard.
User submits [input type], we call [Claude/OpenAI] server-side via a new
API route in src/app/api/. Store history in Supabase. Paywall required.
```

### Add a simple upload flow

```
Add file upload to the dashboard for [file type]. Store in Supabase Storage.
Show a list of uploads for the current user. Paywall required.
```

---

## Files you SHOULD edit

| File | Purpose |
|------|---------|
| `src/config/product.ts` | Name, tagline, plan display, FAQs |
| `src/app/(app)/dashboard/page.tsx` | Your core product UI |
| `src/components/marketing/*` | Landing page sections |
| `supabase/migrations/00X_*.sql` | New tables for your feature |
| `src/app/api/[your-feature]/route.ts` | New API routes for your feature |

---

## Files you should NOT edit (until experienced)

| File | Why |
|------|-----|
| `src/middleware.ts` | Auth routing |
| `src/lib/supabase/middleware.ts` | Session handling |
| `src/app/api/webhooks/lemonsqueezy/route.ts` | Billing sync |
| `src/app/api/checkout/route.ts` | Checkout creation |
| `src/lib/lemonsqueezy.ts` | Lemon Squeezy API |

---

## Paywall pattern

Your feature should only be fully usable when the user has an active subscription. Follow the pattern in `dashboard/page.tsx`:

```tsx
const subscription = await getUserSubscription(user.id);
const hasAccess = isSubscriptionActive(subscription);

if (!hasAccess) {
  return <PaywallBanner />;
}

// Your feature UI below
```

For API routes, check subscription server-side before processing requests.

---

## New Supabase table checklist

When adding a table:

1. Create `supabase/migrations/002_your_feature.sql`
2. Enable RLS: `alter table ... enable row level security`
3. Add policy: users can only access their own rows (`auth.uid() = user_id`)
4. Run migration in Supabase SQL editor
5. Add TypeScript types if needed

---

## Pre-ship checklist

- [ ] Marketing page says who it's for and what they get
- [ ] Dashboard feature works for a subscribed user
- [ ] Paywall blocks unsubscribed users
- [ ] New API routes validate auth + subscription
- [ ] Tested signup → checkout → feature access flow
- [ ] `.env.local` secrets are NOT committed

---

## When you're stuck

| Problem | Ask Cursor |
|---------|------------|
| Feature too big | "Split this into the smallest v1 — one screen, one action" |
| Ugly UI | "Restyle using existing Card/Button components — minimal, clean" |
| RLS errors | "Fix Supabase RLS so users only see their own rows" |
| Webhook broke | Stop — revert auth/billing files, compare with original starter |

Build one feature. Ship it. Talk to customers. Then open `CHECKLIST.md` Part 2.
