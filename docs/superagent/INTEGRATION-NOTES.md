# Store ↔ Superagent integration (planned)

Use this when the custom storefront backend is chosen so Base44 can query orders without exposing binaries.

## Likely stack (from DiskLordz ops)

- **Checkout:** Stripe (tripwire test mode in kanban K-014)
- **App / auth / credits:** Supabase (kanban K-013)
- **Hosting:** Vercel (Instruments / tripwire checkout export)

Confirm actual deployment before implementing.

## Recommended read-only API (implement on storefront)

`GET /api/support/order-lookup?email=&order_id=` (server-only secret header)

Response shape for agents:

```json
{
  "order_id": "ord_…",
  "status": "paid | refunded | pending",
  "email_verified": true,
  "line_items": [{ "sku": "tripwire-sample-v1", "name": "Starter Trap Pack" }],
  "license": { "key_last4": "…8f2a", "activations_used": 1, "activations_max": 2 },
  "downloads": { "remaining": 3, "expires_at": "ISO-8601" }
}
```

**Never** include presigned URLs in this JSON.

## Webhooks

- Stripe `checkout.session.completed` → idempotent fulfillment (see root `.cursorrules`)
- Optional: notify Superagent only via internal event bus — do not expose webhook secrets to Base44

## Next step

Reply with: backend choice (Supabase vs Firebase vs Postgres-only) and whether licenses use a third party (e.g., Lemon Squeezy). A minimal `support-order-lookup` route spec can be added to the storefront repo.