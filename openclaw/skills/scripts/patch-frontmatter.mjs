#!/usr/bin/env node
import { readdir, readFile, writeFile } from "node:fs/promises";
import { join } from "node:path";
import { fileURLToPath } from "node:url";

const shared = join(fileURLToPath(new URL(".", import.meta.url)), "..", "shared");
const baseUrl =
  "https://github.com/samuelfreemanjobs-hash/push/tree/main/openclaw/skills/shared";
const marker = "Before acting, read `references/workflow.md`";

for (const ent of await readdir(shared, { withFileTypes: true })) {
  if (!ent.isDirectory()) continue;
  const name = ent.name;
  const path = join(shared, name, "SKILL.md");
  let raw = await readFile(path, "utf8");
  const m = raw.match(/^---\r?\n([\s\S]*?)\r?\n---\r?\n([\s\S]*)$/);
  if (!m) continue;
  let fm = m[1];
  let body = m[2];
  if (!fm.includes("version:")) {
    fm = fm.replace(/^(description:[^\n]+)\n/m, `$1\nversion: 1.0.0\n`);
  }
  if (!fm.includes("homepage:")) {
    fm = `${fm.trimEnd()}\nhomepage: ${baseUrl}/${name}\n`;
  }
  if (!body.includes(marker)) {
    body = `${marker} (and sibling files under \`references/\`) via skills_read.\n\n${body}`;
  }
  await writeFile(path, `---\n${fm.trimEnd()}\n---\n\n${body.replace(/^\n+/, "")}`, "utf8");
}
console.log("Patched SKILL.md frontmatter + reference hooks");