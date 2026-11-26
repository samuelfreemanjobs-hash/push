interface CheckoutResponse {
  data: {
    attributes: {
      url: string;
    };
  };
}

export async function createCheckout(
  userId: string,
  email: string
): Promise<string> {
  const response = await fetch("https://api.lemonsqueezy.com/v1/checkouts", {
    method: "POST",
    headers: {
      Accept: "application/vnd.api+json",
      "Content-Type": "application/vnd.api+json",
      Authorization: `Bearer ${process.env.LEMONSQUEEZY_API_KEY}`,
    },
    body: JSON.stringify({
      data: {
        type: "checkouts",
        attributes: {
          checkout_data: {
            email,
            custom: {
              user_id: userId,
            },
          },
          product_options: {
            redirect_url: `${process.env.NEXT_PUBLIC_APP_URL}/dashboard?checkout=success`,
          },
        },
        relationships: {
          store: {
            data: {
              type: "stores",
              id: process.env.LEMONSQUEEZY_STORE_ID,
            },
          },
          variant: {
            data: {
              type: "variants",
              id: process.env.LEMONSQUEEZY_VARIANT_ID,
            },
          },
        },
      },
    }),
  });

  if (!response.ok) {
    const errorText = await response.text();
    throw new Error(`Lemon Squeezy checkout failed: ${errorText}`);
  }

  const payload = (await response.json()) as CheckoutResponse;
  return payload.data.attributes.url;
}

export function getCustomerPortalUrl(customerId: string): string {
  return `https://app.lemonsqueezy.com/my-orders/${customerId}`;
}
