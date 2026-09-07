import { createHmac, timingSafeEqual } from "crypto";
import { NextResponse } from "next/server";
import { createServiceClient } from "@/lib/supabase/admin";

type LemonSqueezyWebhook = {
  meta: {
    event_name: string;
    custom_data?: {
      user_id?: string;
    };
  };
  data: {
    id: string;
    attributes: {
      status: string;
      variant_name?: string;
      customer_id?: number;
      renews_at?: string;
      ends_at?: string | null;
    };
  };
};

function verifySignature(rawBody: string, signature: string | null): boolean {
  const secret = process.env.LEMONSQUEEZY_WEBHOOK_SECRET;
  if (!secret || !signature) return false;

  const digest = createHmac("sha256", secret).update(rawBody).digest("hex");

  try {
    return timingSafeEqual(Buffer.from(digest), Buffer.from(signature));
  } catch {
    return false;
  }
}

async function upsertSubscription(payload: LemonSqueezyWebhook) {
  const userId = payload.meta.custom_data?.user_id;
  if (!userId) {
    console.error("Webhook missing user_id in custom_data");
    return;
  }

  const supabase = createServiceClient();
  const attrs = payload.data.attributes;

  const { error } = await supabase.from("subscriptions").upsert(
    {
      user_id: userId,
      status: attrs.status,
      plan_name: attrs.variant_name ?? "Pro",
      lemon_squeezy_subscription_id: payload.data.id,
      lemon_squeezy_customer_id: attrs.customer_id
        ? String(attrs.customer_id)
        : null,
      current_period_end: attrs.renews_at ?? attrs.ends_at ?? null,
      updated_at: new Date().toISOString(),
    },
    { onConflict: "lemon_squeezy_subscription_id" }
  );

  if (error) {
    console.error("Failed to upsert subscription:", error.message);
  }
}

export async function POST(request: Request) {
  const rawBody = await request.text();
  const signature = request.headers.get("x-signature");

  if (!verifySignature(rawBody, signature)) {
    return NextResponse.json({ error: "Invalid signature" }, { status: 401 });
  }

  const payload = JSON.parse(rawBody) as LemonSqueezyWebhook;
  const eventName = payload.meta.event_name;

  const subscriptionEvents = [
    "subscription_created",
    "subscription_updated",
    "subscription_cancelled",
    "subscription_expired",
    "subscription_resumed",
  ];

  if (subscriptionEvents.includes(eventName)) {
    await upsertSubscription(payload);
  }

  return NextResponse.json({ received: true });
}
