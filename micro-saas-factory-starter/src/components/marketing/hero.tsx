import Link from "next/link";
import { productConfig } from "@/config/product";
import { Button } from "@/components/ui/button";

export function Hero() {
  return (
    <section className="mx-auto max-w-6xl px-4 py-20 text-center">
      <p className="mb-4 text-sm font-medium uppercase tracking-wide text-brand-600">
        Micro SaaS Factory Starter
      </p>
      <h1 className="mx-auto max-w-3xl text-4xl font-bold tracking-tight text-slate-900 sm:text-5xl">
        {productConfig.tagline}
      </h1>
      <p className="mx-auto mt-6 max-w-2xl text-lg text-slate-600">
        Auth, billing, and a deploy-ready dashboard shell — so you add one
        feature and launch this week.
      </p>
      <div className="mt-8 flex flex-col items-center justify-center gap-3 sm:flex-row">
        <Link href="/signup">
          <Button size="lg">Start free — add billing later</Button>
        </Link>
        <Link href="/pricing">
          <Button variant="secondary" size="lg">
            View pricing
          </Button>
        </Link>
      </div>
    </section>
  );
}
