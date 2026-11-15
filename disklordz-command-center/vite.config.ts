import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";
import { VitePWA } from "vite-plugin-pwa";

export default defineConfig({
  plugins: [
    react(),
    VitePWA({
      registerType: "autoUpdate",
      includeAssets: ["icons/icon.svg"],
      manifest: {
        name: "DiskLordz Command Center",
        short_name: "DLZ Ops",
        description: "DiskLordz ops + free mobile coding crew",
        theme_color: "#0c0b0f",
        background_color: "#0c0b0f",
        display: "standalone",
        start_url: "/",
        icons: [
          {
            src: "icons/icon.svg",
            sizes: "512x512",
            type: "image/svg+xml",
            purpose: "any",
          },
        ],
      },
    }),
  ],
  server: { host: "127.0.0.1", port: 5173 },
  preview: { host: "127.0.0.1", port: 5173 },
});