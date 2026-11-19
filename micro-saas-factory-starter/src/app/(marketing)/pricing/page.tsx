import { PricingTable } from "@/components/marketing/pricing-table";

export default function PricingPage() {
  return (
    <section className="mx-auto max-w-6xl px-4 py-20">
      <div className="mb-10 text-center">
        <h1 className="text-4xl font-bold text-slate-900">Simple pricing</h1>
        <p className="mt-3 text-slate-600">
          One plan to start. Upgrade complexity later when customers ask for it.
        </p>
      </div>
      <PricingTable />
    </section>
  );
}
