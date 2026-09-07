import { NextResponse } from "next/server";
import { createCheckout } from "@/lib/lemonsqueezy";
import { createClient } from "@/lib/supabase/server";

export async function POST() {
  try {
    const supabase = await createClient();
    const {
      data: { user },
    } = await supabase.auth.getUser();

    if (!user?.email) {
      return NextResponse.json(
        { error: "You must be logged in to checkout" },
        { status: 401 }
      );
    }

    const required = [
      "LEMONSQUEEZY_API_KEY",
      "LEMONSQUEEZY_STORE_ID",
      "LEMONSQUEEZY_VARIANT_ID",
      "NEXT_PUBLIC_APP_URL",
    ];

    for (const key of required) {
      if (!process.env[key]) {
        return NextResponse.json(
          { error: `Missing environment variable: ${key}` },
          { status: 500 }
        );
      }
    }

    const url = await createCheckout(user.id, user.email);
    return NextResponse.json({ url });
  } catch (error) {
    console.error("Checkout error:", error);
    return NextResponse.json(
      {
        error:
          error instanceof Error ? error.message : "Failed to create checkout",
      },
      { status: 500 }
    );
  }
}
