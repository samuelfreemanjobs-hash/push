import { Zap, Shield, CreditCard, Rocket } from "lucide-react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";

const features = [
  {
    icon: Rocket,
    title: "Launch lane, not a cage",
    description:
      "Marketing site, auth, dashboard shell, and deploy config — ready in one session.",
  },
  {
    icon: Shield,
    title: "Auth included",
    description:
      "Email and Google sign-in via Supabase. Protected routes wired from day one.",
  },
  {
    icon: CreditCard,
    title: "Billing included",
    description:
      "Lemon Squeezy checkout, webhooks, and paywall logic — one plan to start.",
  },
  {
    icon: Zap,
    title: "Cursor-ready",
    description:
      "Rules and prompts in CURSOR.md so you extend the app without breaking core flows.",
  },
];

export function Features() {
  return (
    <section className="mx-auto max-w-6xl px-4 py-16">
      <div className="mb-10 text-center">
        <h2 className="text-3xl font-bold text-slate-900">
          Everything between idea and live URL
        </h2>
        <p className="mt-3 text-slate-600">
          You bring one niche feature. The starter handles the infrastructure.
        </p>
      </div>
      <div className="grid gap-6 sm:grid-cols-2">
        {features.map((feature) => (
          <Card key={feature.title}>
            <CardHeader>
              <feature.icon className="h-8 w-8 text-brand-600" />
              <CardTitle>{feature.title}</CardTitle>
            </CardHeader>
            <CardContent>
              <p className="text-sm text-slate-600">{feature.description}</p>
            </CardContent>
          </Card>
        ))}
      </div>
    </section>
  );
}
