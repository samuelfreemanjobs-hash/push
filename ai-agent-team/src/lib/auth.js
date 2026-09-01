export function verifyN8nSecret(req) {
  const expected = process.env.N8N_WEBHOOK_SECRET;
  if (!expected) {
    return true;
  }
  const provided =
    req.headers['x-n8n-secret'] ||
    req.headers['authorization']?.replace(/^Bearer\s+/i, '');
  return provided === expected;
}

export function n8nAuthMiddleware(req, res, next) {
  if (!verifyN8nSecret(req)) {
    return res.status(401).json({ error: 'Invalid n8n webhook secret' });
  }
  return next();
}
