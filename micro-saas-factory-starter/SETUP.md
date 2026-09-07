# Setup Guide — Deploy in 20 Minutes

Follow these steps in order. Each step should take 2–5 minutes.

**Prerequisites:** Node.js 20+, a [Supabase](https://supabase.com) account, a [Lemon Squeezy](https://lemonsqueezy.com) account, and a [Vercel](https://vercel.com) account.

---

## Step 1 — Clone and install (2 min)

```bash
git clone <your-template-repo-url>
cd micro-saas-factory-starter
cp .env.example .env.local
npm install
```

Fill in the app section of `.env.local`:

```env
NEXT_PUBLIC_APP_NAME="Your Product"
NEXT_PUBLIC_APP_URL="http://localhost:3000"
NEXT_PUBLIC_TAGLINE="Your one-line value proposition"
NEXT_PUBLIC_PLAN_NAME="Pro"
NEXT_PUBLIC_PLAN_PRICE="$29"
NEXT_PUBLIC_PLAN_INTERVAL="month"
```

Start the dev server:

```bash
npm run dev
```

Visit [http://localhost:3000](http://localhost:3000) — you should see the landing page.

---

## Step 2 — Supabase setup (5 min)

### 2a. Create a project

1. Go to [supabase.com/dashboard](https://supabase.com/dashboard)
2. **New project** → pick a name and password → wait for provisioning

### 2b. Run the migration

1. Open **SQL Editor** in your Supabase dashboard
2. Paste the contents of `supabase/migrations/001_initial.sql`
3. Click **Run**

This creates `profiles`, `subscriptions`, and row-level security policies.

### 2c. Copy API keys

1. Go to **Project Settings → API**
2. Copy **Project URL** → `NEXT_PUBLIC_SUPABASE_URL`
3. Copy **anon public** key → `NEXT_PUBLIC_SUPABASE_ANON_KEY`
4. Copy **service_role** key → `SUPABASE_SERVICE_ROLE_KEY` (keep secret — server only)

### 2d. Enable Google OAuth (optional, +2 min)

1. Go to **Authentication → Providers → Google**
2. Enable and add your Google OAuth credentials
3. Add redirect URL: `http://localhost:3000/auth/callback`
4. Later add your production URL too: `https://yourdomain.com/auth/callback`

### 2e. Configure auth redirect URLs

1. Go to **Authentication → URL Configuration**
2. Site URL: `http://localhost:3000` (change to production URL after deploy)
3. Redirect URLs: add `http://localhost:3000/auth/callback`

### 2f. Test auth

1. Restart `npm run dev`
2. Go to [http://localhost:3000/signup](http://localhost:3000/signup)
3. Create an account → you should land on `/dashboard`

---

## Step 3 — Lemon Squeezy billing (5 min)

### 3a. Create a product

1. Go to [app.lemonsqueezy.com](https://app.lemonsqueezy.com)
2. **Products → New product**
3. Create a subscription (e.g. $29/month)
4. Note the **Variant ID** (from the variant URL or API)

### 3b. Get API keys

1. Go to **Settings → API**
2. Create an API key → `LEMONSQUEEZY_API_KEY`
3. Note your **Store ID** → `LEMONSQUEEZY_STORE_ID`
4. Set `LEMONSQUEEZY_VARIANT_ID` to your variant ID

### 3c. Configure webhook (local testing with ngrok or deploy first)

For production (after Vercel deploy):

1. Go to **Settings → Webhooks**
2. Add webhook URL: `https://yourdomain.com/api/webhooks/lemonsqueezy`
3. Subscribe to: `subscription_created`, `subscription_updated`, `subscription_cancelled`, `subscription_expired`, `subscription_resumed`
4. Copy signing secret → `LEMONSQUEEZY_WEBHOOK_SECRET`

### 3d. Test checkout

1. Log in at [http://localhost:3000/login](http://localhost:3000/login)
2. Go to [http://localhost:3000/pricing](http://localhost:3000/pricing)
3. Click **Subscribe now** → Lemon Squeezy checkout opens
4. Complete a **test mode** purchase
5. After webhook fires, dashboard paywall should disappear

> **Note:** Webhooks won't reach localhost without a tunnel (ngrok) or deploying first. Deploy to Vercel (Step 4), then configure the webhook.

---

## Step 4 — Deploy to Vercel (5 min)

### 4a. Push to GitHub

Push this repo to a private GitHub repository.

### 4b. Import to Vercel

1. Go to [vercel.com/new](https://vercel.com/new)
2. Import your repo
3. Root directory: `micro-saas-factory-starter` (if nested in a monorepo)
4. Add **all** environment variables from `.env.local`
5. Update `NEXT_PUBLIC_APP_URL` to your Vercel URL (e.g. `https://your-app.vercel.app`)
6. Deploy

### 4c. Update external services

After deploy, update URLs in:

| Service | What to update |
|---------|----------------|
| **Supabase** | Site URL + redirect URLs with production domain |
| **Lemon Squeezy** | Webhook URL with production domain |
| **Google OAuth** | Authorized redirect URIs (if using Google) |

### 4d. Smoke test production

- [ ] Landing page loads
- [ ] Signup works
- [ ] Login works
- [ ] Checkout redirects to Lemon Squeezy
- [ ] Webhook updates subscription (check Supabase `subscriptions` table)
- [ ] Dashboard unlocks after payment

---

## Step 5 — Customize your product (5 min)

1. Edit `src/config/product.ts` — name, tagline, features, FAQs
2. Edit marketing copy in `src/components/marketing/hero.tsx` if needed
3. Complete **Part 1** of `CHECKLIST.md` — choose your Micro SaaS idea
4. Use `CURSOR.md` to build your dashboard feature

---

## Troubleshooting

| Problem | Fix |
|---------|-----|
| "Invalid API key" on auth | Check Supabase URL and anon key in `.env.local` |
| Redirect loop on login | Verify Supabase redirect URLs match your domain |
| Checkout 500 error | Verify all `LEMONSQUEEZY_*` env vars are set |
| Paid but still paywalled | Check webhook URL, signing secret, and Supabase `subscriptions` table |
| Google login fails | Enable Google provider + add redirect URI |

---

## Next steps

1. `CHECKLIST.md` — validate your idea and get your first customer in 7 days
2. `CURSOR.md` — build your v1 dashboard feature
3. Custom domain on Vercel when ready

**You're live when:** production URL + auth + billing all work. Then build your one feature.
