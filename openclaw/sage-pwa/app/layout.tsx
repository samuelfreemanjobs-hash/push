import type { ReactNode } from "react";

export const metadata = {
  title: "DiskLordz Sage",
  description: "Telegram front door for DiskLordz OpenClaw",
};

export default function RootLayout({ children }: { children: ReactNode }) {
  return (
    <html lang="en">
      <body style={{ fontFamily: "system-ui", margin: "2rem", maxWidth: 640 }}>
        {children}
      </body>
    </html>
  );
}