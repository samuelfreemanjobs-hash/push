import Link from "next/link";
import { Sparkles } from "lucide-react";
import { PaywallBanner } from "@/components/app/paywall-banner";
import { Button } from "@/components/ui/button";
import {
  Card,
  CardContent,
  CardDescription,
  CardHeader,
  CardTitle,
} from "@/components/ui/card";
import { createClient } from "@/lib/supabase/server";
import { getUserSubscription, isSubscriptionActive } from "@/lib/subscription";

export default async function DashboardPage() {
  const supabase = await createClient();
  const {
    data: { user },
  } = await supabase.auth.getUser();

  const subscription = user ? await getUserSubscription(user.id) : null;
  const hasAccess = isSubscriptionActive(subscription);

  return (
    <div className="p-8">
      <div className="mb-8">
        <h1 className="text-2xl font-bold text-slate-900">Dashboard</h1>
        <p className="mt-1 text-slate-600">
          Your Micro SaaS shell is live. Add your one core feature here.
        </p>
      </div>

      {!hasAccess && <PaywallBanner />}

      <Card>
        <CardHeader>
          <div className="flex items-center gap-2">
            <Sparkles className="h-5 w-5 text-brand-600" />
            <CardTitle>Your first feature goes here</CardTitle>
          </div>
          <CardDescription>
            Complete CHECKLIST.md, then use CURSOR.md to build your v1 feature
            in this dashboard.
          </CardDescription>
        </CardHeader>
        <CardContent className="space-y-4">
          <div className="rounded-lg border border-dashed border-slate-300 bg-slate-50 p-8 text-center">
            <p className="text-sm text-slate-600">
              Replace this placeholder with your product&apos;s core UI — a
              form, table, generator, or AI tool for your niche customer.
            </p>
          </div>
          <div className="space-y-2">
            <p className="text-sm font-medium text-slate-900">
              Suggested Cursor prompts:
            </p>
            <ul className="list-inside list-disc space-y-1 text-sm text-slate-600">
              <li>
                Add a CRUD table for [your entity] using Supabase and existing
                components
              </li>
              <li>
                Add an AI input/output panel that calls Claude for [your use
                case]
              </li>
              <li>
                Add a simple upload + report generator for [your customer type]
              </li>
            </ul>
          </div>
          <Link href="/settings">
            <Button variant="secondary">Account & billing</Button>
          </Link>
        </CardContent>
      </Card>
    </div>
  );
}
