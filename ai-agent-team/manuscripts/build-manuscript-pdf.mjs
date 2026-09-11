import { readFileSync, writeFileSync, mkdirSync } from 'fs';
import { join, dirname } from 'path';
import { fileURLToPath } from 'url';
import { marked } from 'marked';
import puppeteer from 'puppeteer';

const __dirname = dirname(fileURLToPath(import.meta.url));
const bookDir = join(__dirname, 'building-ai-agents-with-claude');
const outDir = join(__dirname, 'output');

const FILES = [
  '00-introduction.md',
  '01-chapter-end-of-overhead.md',
  '02-chapter-claude-advantage.md',
  '03-chapter-digital-worker-architecture.md',
  '04-chapter-mcp-integrations.md',
  '05-chapter-multi-agent-choreography.md',
  '06-chapter-revenue-engine.md',
  '07-chapter-back-office-engine.md',
  '08-chapter-client-delivery.md',
  '09-chapter-guardrail-protocol.md',
  '10-chapter-synthetic-workforce.md',
  '11-chapter-30-day-roadmap.md',
  'appendix-implementation-toolkit.md',
];

const css = readFileSync(join(__dirname, 'manuscript-pdf.css'), 'utf8');

function buildMarkdown() {
  const title = `# Building AI Agents with Claude

## How to Build a Digital Workforce That Automates Grunt Work, Cuts Overhead, and Scales Your Business

**Review Draft** — Manuscript Master

*Publication-ready first draft for author review*

\\newpage

`;

  const bodies = FILES.map((file) => {
    const text = readFileSync(join(bookDir, file), 'utf8');
    return `\n\n---\n\n${text}\n`;
  });

  return title + bodies.join('\n');
}

function buildHtml(markdown) {
  const body = marked.parse(markdown, { gfm: true, breaks: false });
  return `<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <title>Building AI Agents with Claude — Review Draft</title>
  <style>${css}</style>
</head>
<body>
${body}
</body>
</html>`;
}

mkdirSync(outDir, { recursive: true });

const markdown = buildMarkdown();
const mdPath = join(outDir, 'building-ai-agents-with-claude-review.md');
const htmlPath = join(outDir, 'building-ai-agents-with-claude-review.html');
const pdfPath = join(outDir, 'Building-AI-Agents-with-Claude-Review-Draft.pdf');

writeFileSync(mdPath, markdown, 'utf8');
const html = buildHtml(markdown);
writeFileSync(htmlPath, html, 'utf8');

const browser = await puppeteer.launch({
  headless: true,
  args: ['--no-sandbox', '--disable-setuid-sandbox'],
});

const page = await browser.newPage();
await page.goto(`file://${htmlPath}`, { waitUntil: 'networkidle0' });

await page.pdf({
  path: pdfPath,
  format: 'Letter',
  margin: { top: '20mm', right: '18mm', bottom: '22mm', left: '18mm' },
  printBackground: true,
  displayHeaderFooter: true,
  headerTemplate: '<div></div>',
  footerTemplate:
    '<div style="font-size:8px;width:100%;text-align:center;color:#666;"><span class="pageNumber"></span> / <span class="totalPages"></span></div>',
});

await browser.close();

console.log('Wrote:', pdfPath);
console.log('HTML:', htmlPath);
console.log('Markdown:', mdPath);
