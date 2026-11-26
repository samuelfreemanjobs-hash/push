"use client";

import { useState } from "react";
import { Check } from "lucide-react";
import { productConfig } from "@/config/product";
import { Button } from "@/components/ui/button";
import {
  Card,
  CardContent,
  CardDescription,
  CardHeader,
  CardTitle,
} from "@/components/ui/card";

export function PricingTable() {
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  async function handleCheckout() {
    setLoading(true);
    setError(null);

    try {
      const response = await fetch("/api/checkout", { method: "POST" });
      const data = await response.json();

      if (!response.ok) {
        throw new Error(data.error ?? "Checkout failed");
      }

      window.location.href = data.url;
    } catch (err) {
      setError(err instanceof Error ? err.message : "Checkout failed");
      setLoading(false);
    }
  }

  return (
    <Card className="mx-auto max-w-md">
      <CardHeader className="text-center">
        <CardTitle>{productConfig.plan.name}</CardTitle>
        <CardDescription>Everything you need to launch</CardDescription>
        <div className="pt-4">
          <span className="text-4xl font-bold text-slate-900">
            {productConfig.plan.price}
          </span>
          <span className="text-slate-500">/{productConfig.plan.interval}</span>
        </div>
      </CardHeader>
      <CardContent className="space-y-4">
        <ul className="space-y-3">
          {productConfig.features.map((feature) => (
            <li key={feature} className="flex items-start gap-2 text-sm">
              <Check className="mt-0.5 h-4 w-4 shrink-0 text-brand-600" />
              <span>{feature}</span>
            </li>
          ))}
        </ul>
        {error && <p className="text-sm text-red-600">{error}</p>}
        <Button className="w-full" onClick={handleCheckout} disabled={loading}>
          {loading ? "Redirecting..." : "Subscribe now"}
        </Button>
        <p className="text-center text-xs text-slate-500">
          Log in first if you don&apos;t have an account yet.
        </p>
      </CardContent>
    </Card>
  );
}
