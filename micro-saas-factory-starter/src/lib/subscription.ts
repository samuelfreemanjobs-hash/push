import { createClient } from "@/lib/supabase/server";

export type SubscriptionStatus =
  | "active"
  | "on_trial"
  | "paused"
  | "past_due"
  | "unpaid"
  | "cancelled"
  | "expired";

export interface Subscription {
  id: string;
  user_id: string;
  status: SubscriptionStatus;
  plan_name: string | null;
  lemon_squeezy_subscription_id: string | null;
  lemon_squeezy_customer_id: string | null;
  current_period_end: string | null;
}

const ACTIVE_STATUSES: SubscriptionStatus[] = ["active", "on_trial"];

export function isSubscriptionActive(
  subscription: Subscription | null
): boolean {
  if (!subscription) return false;
  return ACTIVE_STATUSES.includes(subscription.status);
}

export async function getUserSubscription(
  userId: string
): Promise<Subscription | null> {
  const supabase = await createClient();

  const { data, error } = await supabase
    .from("subscriptions")
    .select("*")
    .eq("user_id", userId)
    .order("created_at", { ascending: false })
    .limit(1)
    .maybeSingle();

  if (error) {
    console.error("Failed to fetch subscription:", error.message);
    return null;
  }

  return data as Subscription | null;
}
