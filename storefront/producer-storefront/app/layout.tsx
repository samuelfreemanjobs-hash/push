import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "Underground Phonk & Trap — Neural VST3 | DiskLordz",
  description:
    "Local GPU-accelerated Trap & Memphis Phonk VST3 plus royalty-free WAVs. Live beat pad preview, 3-tier pricing, free starter kit.",
};

export default function RootLayout({ children }: { children: React.ReactNode }) {
  return (
    <html lang="en">
      <body className="antialiased">{children}</body>
    </html>
  );
}