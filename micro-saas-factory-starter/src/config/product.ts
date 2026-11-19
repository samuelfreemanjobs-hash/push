export const productConfig = {
  name: process.env.NEXT_PUBLIC_APP_NAME ?? "Your Product",
  tagline:
    process.env.NEXT_PUBLIC_TAGLINE ??
    "Ship your Micro SaaS in 20 minutes — not 20 weeks.",
  url: process.env.NEXT_PUBLIC_APP_URL ?? "http://localhost:3000",
  plan: {
    name: process.env.NEXT_PUBLIC_PLAN_NAME ?? "Pro",
    price: process.env.NEXT_PUBLIC_PLAN_PRICE ?? "$29",
    interval: process.env.NEXT_PUBLIC_PLAN_INTERVAL ?? "month",
  },
  features: [
    "One core feature — yours to build",
    "Secure auth and user accounts",
    "Subscription billing included",
    "Deploy-ready on Vercel",
  ],
  faqs: [
    {
      question: "Do I need coding experience?",
      answer:
        "Yes. This starter is built for solo technical founders who are comfortable with JavaScript/TypeScript and using Cursor or a similar editor.",
    },
    {
      question: "Can I change the stack later?",
      answer:
        "Absolutely. This is a launch lane, not a cage. Ship first, refactor later.",
    },
    {
      question: "What do I build on top?",
      answer:
        "One dashboard feature for one niche customer. Complete CHECKLIST.md before writing product code.",
    },
  ],
} as const;
