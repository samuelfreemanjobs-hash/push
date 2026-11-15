#!/usr/bin/env node
import { readdir, readFile } from "node:fs/promises";
import { join, basename } from "node:path";
import { fileURLToPath } from "node:url";
import { parse as parseYaml } from "yaml";

const root = join(fileURLToPath(new URL(".", import.meta.url)), "..", "shared");
let errors = 0;

const dirs = await readdir(root, { withFileTypes: true });
for (const ent of dirs) {
  if (!ent.isDirectory()) continue;
  const name = ent.name;
  const skillPath = join(root, name, "SKILL.md");
  const refPath = join(root, name, "references", "workflow.md");
  let raw;
  try {
    raw = await readFile(skillPath, "utf8");
  } catch {
    console.error(name, "missing SKILL.md");
    errors++;
    continue;
  }
  const m = raw.match(/^---\r?\n([\s\S]*?)\r?\n---/);
  if (!m) {
    console.error(name, "missing frontmatter");
    errors++;
    continue;
  }
  const fm = parseYaml(m[1]) ?? {};
  if (fm.name !== name) {
    console.error(name, `frontmatter name must be ${name}, got ${fm.name}`);
    errors++;
  }
  if (!fm.description || fm.description.length < 12) {
    console.error(name, "description too short");
    errors++;
  }
  if (!fm.version) {
    console.error(name, "missing version in frontmatter (ClawHub)");
    errors++;
  }
  try {
    await readFile(refPath, "utf8");
  } catch {
    console.error(name, "missing references/workflow.md");
    errors++;
  }
}

if (errors) {
  console.error(`${errors} issue(s)`);
  process.exit(1);
}
console.log(`OK ${dirs.filter((d) => d.isDirectory()).length} skills ready for ClawHub`);