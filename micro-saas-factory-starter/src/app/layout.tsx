import type { Metadata } from "next";
import { Inter } from "next/font/google";
import "./globals.css";
import { productConfig } from "@/config/product";

const inter = Inter({ subsets: ["latin"] });

export const metadata: Metadata = {
  title: {
    default: productConfig.name,
    template: `%s | ${productConfig.name}`,
  },
  description: productConfig.tagline,
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="en">
      <body className={inter.className}>{children}</body>
    </html>
  );
}
