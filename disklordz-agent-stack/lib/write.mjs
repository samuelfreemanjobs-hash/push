import { mkdir, writeFile } from "node:fs/promises";
import { dirname, join } from "node:path";
import { hermesOutDir } from "./paths.mjs";

export async function writeHermesOut(relPath, data, { json = false } = {}) {
  const base = hermesOutDir();
  const full = join(base, relPath);
  await mkdir(dirname(full), { recursive: true });
  const body = json ? `${JSON.stringify(data, null, 2)}\n` : data;
  await writeFile(full, body, "utf8");
  return full;
}