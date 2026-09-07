import { readFile } from 'fs/promises';
import { dirname, join } from 'path';
import { fileURLToPath } from 'url';

const __dirname = dirname(fileURLToPath(import.meta.url));
const PROMPTS_DIR = join(__dirname, '../../../prompts/manuscript-master');

/**
 * Load an XML prompt module and hydrate variable placeholders.
 */
export async function loadPromptModule(moduleId, variables = {}) {
  const filename = `${moduleId}.xml`;
  const filepath = join(PROMPTS_DIR, 'modules', filename);

  let template;
  try {
    template = await readFile(filepath, 'utf-8');
  } catch {
    throw new Error(`Prompt module not found: ${filename}`);
  }

  return hydrateVariables(template, variables);
}

export async function loadSystemDirective() {
  const filepath = join(PROMPTS_DIR, 'system-directive.xml');
  return readFile(filepath, 'utf-8');
}

export async function loadMetaPromptGenerator(variables = {}) {
  const filepath = join(PROMPTS_DIR, 'meta-prompt-generator.xml');
  const template = await readFile(filepath, 'utf-8');
  return hydrateVariables(template, variables);
}

/**
 * Replace [INSERT ...] style placeholders and {var} tokens in prompt XML.
 */
export function hydrateVariables(template, variables) {
  let result = template;

  for (const [key, value] of Object.entries(variables)) {
    const strValue = String(value ?? '');
    result = result.replaceAll(`{${key}}`, strValue);
    result = result.replace(
      new RegExp(`\\[INSERT[^\\]]*${key.replace(/_/g, '[\\s_/]*')}[^\\]]*\\]`, 'gi'),
      strValue
    );
  }

  return result;
}

/**
 * Build the full prompt sent to the model: system directive + module + state context.
 */
export async function buildFullPrompt(moduleId, variables, stateContext = null) {
  const [systemDirective, modulePrompt] = await Promise.all([
    loadSystemDirective(),
    loadPromptModule(moduleId, variables),
  ]);

  const parts = [
    systemDirective,
    '',
    '---',
    '',
    modulePrompt,
  ];

  if (stateContext) {
    parts.push(
      '',
      '---',
      '',
      '<session_state>',
      JSON.stringify(stateContext, null, 2),
      '</session_state>',
      '',
      'Honor established narrative threads, jargon, and completed milestones from session state.',
    );
  }

  return parts.join('\n');
}
