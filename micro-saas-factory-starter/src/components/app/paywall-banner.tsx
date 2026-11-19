import Link from "next/link";
import { AlertCircle } from "lucide-react";
import { Button } from "@/components/ui/button";

export function PaywallBanner() {
  return (
    <div className="mb-6 flex items-start gap-3 rounded-xl border border-amber-200 bg-amber-50 p-4">
      <AlertCircle className="mt-0.5 h-5 w-5 shrink-0 text-amber-600" />
      <div className="flex-1">
        <h3 className="font-medium text-amber-900">Subscription required</h3>
        <p className="mt-1 text-sm text-amber-800">
          Subscribe to unlock the full dashboard. During development you can
          test checkout in Lemon Squeezy test mode.
        </p>
        <Link href="/pricing" className="mt-3 inline-block">
          <Button size="sm">View pricing</Button>
        </Link>
      </div>
    </div>
  );
}
