import { createEtsyClientFromEnv } from './client.js';

/**
 * Map AI listing JSON to Etsy createListing payload (draft).
 * Digital listings still need files uploaded separately via Etsy UI or upload API.
 */
export function toEtsyCreatePayload(listingJson) {
  const priceCents = Math.round((listingJson.priceUsd ?? 0) * 100);
  const isDigital = listingJson.type === 'download';

  return {
    quantity: listingJson.quantity ?? 999,
    title: (listingJson.title || '').slice(0, 140),
    description: listingJson.description || '',
    price: priceCents / 100,
    who_made: listingJson.who_made || 'i_did',
    when_made: listingJson.when_made || '2020_2025',
    taxonomy_id: listingJson.taxonomy_id || process.env.ETSY_DEFAULT_TAXONOMY_ID,
    is_supply: listingJson.is_supply ?? false,
    type: isDigital ? 'download' : 'physical',
    tags: (listingJson.tags || []).slice(0, 13),
    materials: listingJson.materials || [],
    state: 'draft',
  };
}

export async function publishDraftListing(listingJson) {
  const client = createEtsyClientFromEnv();
  const payload = toEtsyCreatePayload(listingJson);

  if (!payload.taxonomy_id) {
    throw new Error(
      'Set ETSY_DEFAULT_TAXONOMY_ID or include taxonomy_id on the listing JSON',
    );
  }

  const created = await client.createDraftListing(payload);
  return { payload, created };
}
