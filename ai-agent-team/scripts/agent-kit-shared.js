export const LICENSE = `COMMERCIAL USE LICENSE

Purchaser may use these files for their own business and client work.

You may NOT resell, redistribute, or share the raw files (free or paid), or list derivatives as your own template product without a separate license from the seller.

Questions: contact the seller via Etsy messages.
`;

export function workflowFile(id, title, job, inputs, brief, qualityChecks) {
  const inputBlock = inputs.map((i) => `- ${i}`).join('\n');
  const qc = qualityChecks.map((c) => `- [ ] ${c}`).join('\n');
  return `# Workflow ${id} — ${title}

## Job
${job}

## Inputs
${inputBlock}

## Agent brief (copy everything below into your AI tool)

---

${brief}

## Quality check
${qc}
`;
}

export function writeKit(root, kit) {
  const base = `${root}/products/${kit.slug}`;
  const fs = require('fs');
  const path = require('path');
  const mkdir = (p) => fs.mkdirSync(p, { recursive: true });
  const write = (rel, content) => {
    const full = path.join(base, rel);
    mkdir(path.dirname(full));
    fs.writeFileSync(full, content);
  };

  write('LICENSE.txt', LICENSE);
  write('README-START-HERE.md', kit.readme);
  write('AGENT-PROFILE.md', kit.profile);
  write('01-playbook/playbook.md', kit.playbook);
  write('05-implementation/setup-guide.md', kit.setup);
  kit.workflows.forEach((w, i) => {
    const num = String(i + 1).padStart(2, '0');
    write(`02-workflows/${num}-${w.slug}.md`, workflowFile(num, w.title, w.job, w.inputs, w.brief, w.qc));
  });
  if (kit.templates) {
    Object.entries(kit.templates).forEach(([name, content]) => write(`03-templates/${name}`, content));
  }
  return base;
}
