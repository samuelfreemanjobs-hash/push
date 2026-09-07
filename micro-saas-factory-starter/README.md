# Micro SaaS Factory Starter

The official launch shell from *The Micro SaaS Factory* — auth, billing, marketing site, and dashboard ready to deploy in 20 minutes.

## What's included

- **Next.js 15** app with TypeScript and Tailwind
- **Marketing site** — landing page, pricing, FAQ
- **Supabase auth** — email/password + Google OAuth
- **Lemon Squeezy billing** — checkout, webhooks, paywall
- **Dashboard shell** — protected app area for your feature
- **Cursor rules** — extend without breaking auth/billing

## Quick start

```bash
cd micro-saas-factory-starter
cp .env.example .env.local
npm install
npm run dev
```

Open [http://localhost:3000](http://localhost:3000).

**Before building your feature:** complete `CHECKLIST.md`  
**Before deploying:** follow `SETUP.md` step by step  
**When customizing:** use `CURSOR.md` and `.cursor/rules/`

## 20-minute deploy path

| Step | Doc |
|------|-----|
| 1. Clone & install | This README |
| 2. Supabase + auth | `SETUP.md` → Step 2 |
| 3. Lemon Squeezy billing | `SETUP.md` → Step 3 |
| 4. Deploy to Vercel | `SETUP.md` → Step 4 |
| 5. Choose your idea | `CHECKLIST.md` |
| 6. Build your feature | `CURSOR.md` |

## Project structure

```
src/
├── app/
│   ├── (marketing)/     # Public pages
│   ├── (auth)/            # Login & signup
│   ├── (app)/             # Protected dashboard
│   └── api/               # Checkout + webhooks
├── components/
├── config/product.ts      # ← Edit your name, tagline, pricing display
└── lib/                   # Supabase, billing, subscriptions
supabase/migrations/       # Run in Supabase SQL editor
docs/                      # Split setup guides
```

## Customize first

Edit `.env.local` and `src/config/product.ts`:

- Product name and tagline
- Plan name and display price
- FAQ copy

Do **not** edit auth or webhook routes until you understand the flow.

## Scripts

```bash
npm run dev      # Local development
npm run build    # Production build
npm run start    # Run production build
npm run lint     # ESLint
```

## Support

Stuck on setup? Re-read `SETUP.md` for the step you're on. Stuck on what to build? Complete Part 1 of `CHECKLIST.md`.

## License

Paid starter kit — use for unlimited projects you build. See your purchase terms for agency/redistribution rules.
