import Link from "next/link";
import { CheckoutButton } from "@/components/app/checkout-button";
import { Button } from "@/components/ui/button";
import {
  Card,
  CardContent,
  CardDescription,
  CardHeader,
  CardTitle,
} from "@/components/ui/card";
import { productConfig } from "@/config/product";
import { getCustomerPortalUrl } from "@/lib/lemonsqueezy";
import { createClient } from "@/lib/supabase/server";
import { getUserSubscription, isSubscriptionActive } from "@/lib/subscription";

export default async function SettingsPage() {
  const supabase = await createClient();
  const {
    data: { user },
  } = await supabase.auth.getUser();

  const subscription = user ? await getUserSubscription(user.id) : null;
  const hasAccess = isSubscriptionActive(subscription);
  const portalUrl =
    subscription?.lemon_squeezy_customer_id &&
    getCustomerPortalUrl(subscription.lemon_squeezy_customer_id);

  return (
    <div className="p-8">
      <div className="mb-8">
        <h1 className="text-2xl font-bold text-slate-900">Settings</h1>
        <p className="mt-1 text-slate-600">
          Manage your account and subscription.
        </p>
      </div>

      <div className="max-w-2xl space-y-6">
        <Card>
          <CardHeader>
            <CardTitle>Account</CardTitle>
            <CardDescription>Your login details</CardDescription>
          </CardHeader>
          <CardContent className="space-y-2 text-sm">
            <p>
              <span className="font-medium text-slate-700">Email:</span>{" "}
              {user?.email}
            </p>
            <p>
              <span className="font-medium text-slate-700">User ID:</span>{" "}
              <code className="rounded bg-slate-100 px-1 py-0.5 text-xs">
                {user?.id}
              </code>
            </p>
          </CardContent>
        </Card>

        <Card>
          <CardHeader>
            <CardTitle>Subscription</CardTitle>
            <CardDescription>
              {productConfig.plan.name} — {productConfig.plan.price}/
              {productConfig.plan.interval}
            </CardDescription>
          </CardHeader>
          <CardContent className="space-y-4">
            <div className="text-sm">
              <p>
                <span className="font-medium text-slate-700">Status:</span>{" "}
                {subscription?.status ?? "none"}
              </p>
              {subscription?.current_period_end && (
                <p className="mt-1">
                  <span className="font-medium text-slate-700">
                    Current period ends:
                  </span>{" "}
                  {new Date(subscription.current_period_end).toLocaleDateString()}
                </p>
              )}
            </div>

            {!hasAccess && (
              <div className="flex flex-wrap gap-3">
                <CheckoutButton />
                <Link href="/pricing">
                  <Button variant="secondary">View pricing page</Button>
                </Link>
              </div>
            )}

            {hasAccess && portalUrl && (
              <a href={portalUrl} target="_blank" rel="noopener noreferrer">
                <Button variant="secondary">Manage billing</Button>
              </a>
            )}
          </CardContent>
        </Card>
      </div>
    </div>
  );
}
