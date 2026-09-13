/**
 * Minimal Canva Connect API client (autofill + export).
 * @see https://www.canva.dev/docs/connect/
 */

const API_BASE = 'https://api.canva.com/rest/v1';

export class CanvaApiError extends Error {
  constructor(message, status, body) {
    super(message);
    this.status = status;
    this.body = body;
  }
}

async function parseJson(res) {
  const text = await res.text();
  try {
    return text ? JSON.parse(text) : {};
  } catch {
    return { raw: text };
  }
}

async function request(accessToken, method, path, body) {
  const res = await fetch(`${API_BASE}${path}`, {
    method,
    headers: {
      Authorization: `Bearer ${accessToken}`,
      'Content-Type': 'application/json',
    },
    body: body ? JSON.stringify(body) : undefined,
  });
  const data = await parseJson(res);
  if (!res.ok) {
    throw new CanvaApiError(data?.message || `Canva API ${res.status}`, res.status, data);
  }
  return data;
}

/** Map flat strings to Canva autofill data payload (text fields). */
export function toAutofillData(fields, fieldMap) {
  const data = {};
  for (const [ourKey, canvaKey] of Object.entries(fieldMap)) {
    const val = fields[ourKey];
    if (val == null || val === '') continue;
    data[canvaKey] = { type: 'text', text: String(val) };
  }
  return data;
}

export async function createAutofillJob(accessToken, { brandTemplateId, title, data }) {
  return request(accessToken, 'POST', '/autofills', {
    brand_template_id: brandTemplateId,
    title,
    data,
  });
}

export async function getAutofillJob(accessToken, jobId) {
  return request(accessToken, 'GET', `/autofills/${jobId}`);
}

export async function createDesignExport(accessToken, { designId, format = 'png', quality = 'regular' }) {
  return request(accessToken, 'POST', '/exports', {
    design_id: designId,
    format: { type: format, quality },
  });
}

export async function getExportJob(accessToken, exportId) {
  return request(accessToken, 'GET', `/exports/${exportId}`);
}

export async function pollUntilDone(fetchFn, { intervalMs = 2000, maxAttempts = 60 } = {}) {
  let last;
  for (let i = 0; i < maxAttempts; i++) {
    last = await fetchFn();
    const status = last?.job?.status || last?.status;
    if (status === 'success' || status === 'completed') return last;
    if (status === 'failed' || status === 'error') {
      throw new CanvaApiError('Canva job failed', 500, last);
    }
    await new Promise((r) => setTimeout(r, intervalMs));
  }
  throw new CanvaApiError('Canva job timed out', 408, last);
}
