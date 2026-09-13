import { readFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import yaml from 'yaml';
import { Router } from 'express';

const __dirname = dirname(fileURLToPath(import.meta.url));
const backlogPath = join(__dirname, '../../pm/backlog.yaml');

export function createPmRouter() {
  const router = Router();

  router.get('/backlog', (_req, res) => {
    try {
      const raw = readFileSync(backlogPath, 'utf8');
      const data = yaml.parse(raw);
      res.json({ success: true, backlog: data });
    } catch (error) {
      res.status(500).json({
        success: false,
        error: 'Failed to load PM backlog',
        details: error.message,
      });
    }
  });

  router.get('/matrix', (_req, res) => {
    try {
      const matrixPath = join(__dirname, '../../pm/product-matrix.md');
      const markdown = readFileSync(matrixPath, 'utf8');
      res.json({ success: true, format: 'markdown', content: markdown });
    } catch (error) {
      res.status(500).json({
        success: false,
        error: 'Failed to load product matrix',
        details: error.message,
      });
    }
  });

  return router;
}
