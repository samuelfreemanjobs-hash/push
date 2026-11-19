import Link from "next/link";
import { productConfig } from "@/config/product";
import { Button } from "@/components/ui/button";

export function MarketingHeader() {
  return (
    <header className="border-b border-slate-200 bg-white">
      <div className="mx-auto flex h-16 max-w-6xl items-center justify-between px-4">
        <Link href="/" className="font-semibold text-slate-900">
          {productConfig.name}
        </Link>
        <nav className="flex items-center gap-3">
          <Link
            href="/pricing"
            className="text-sm text-slate-600 hover:text-slate-900"
          >
            Pricing
          </Link>
          <Link href="/login">
            <Button variant="ghost" size="sm">
              Log in
            </Button>
          </Link>
          <Link href="/signup">
            <Button size="sm">Get started</Button>
          </Link>
        </nav>
      </div>
    </header>
  );
}

export function MarketingFooter() {
  return (
    <footer className="border-t border-slate-200 bg-white py-10">
      <div className="mx-auto flex max-w-6xl flex-col gap-4 px-4 text-sm text-slate-500 sm:flex-row sm:items-center sm:justify-between">
        <p>
          © {new Date().getFullYear()} {productConfig.name}. Built with Micro
          SaaS Factory Starter.
        </p>
        <div className="flex gap-4">
          <Link href="/pricing" className="hover:text-slate-900">
            Pricing
          </Link>
          <Link href="/login" className="hover:text-slate-900">
            Log in
          </Link>
        </div>
      </div>
    </footer>
  );
}
