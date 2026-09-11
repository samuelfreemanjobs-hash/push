const ETSY_API_BASE = 'https://openapi.etsy.com/v3';

export class EtsyClient {
  constructor({ apiKey, accessToken, shopId }) {
    if (!apiKey) {
      throw new Error('ETSY_API_KEY (keystring) is required');
    }
    this.apiKey = apiKey;
    this.accessToken = accessToken;
    this.shopId = shopId;
  }

  headers() {
    const h = {
      'x-api-key': this.apiKey,
      Accept: 'application/json',
    };
    if (this.accessToken) {
      h.Authorization = `Bearer ${this.accessToken}`;
    }
    return h;
  }

  async request(path, { method = 'GET', body } = {}) {
    const res = await fetch(`${ETSY_API_BASE}${path}`, {
      method,
      headers: {
        ...this.headers(),
        ...(body ? { 'Content-Type': 'application/json' } : {}),
      },
      body: body ? JSON.stringify(body) : undefined,
    });

    const text = await res.text();
    let data;
    try {
      data = text ? JSON.parse(text) : {};
    } catch {
      data = { raw: text };
    }

    if (!res.ok) {
      const err = new Error(data.error || data.error_description || `Etsy API ${res.status}`);
      err.status = res.status;
      err.details = data;
      throw err;
    }

    return data;
  }

  async getShop() {
    if (!this.shopId) {
      throw new Error('ETSY_SHOP_ID is required');
    }
    return this.request(`/application/shops/${this.shopId}`);
  }

  async getActiveListings(limit = 25, offset = 0) {
    return this.request(
      `/application/shops/${this.shopId}/listings/active?limit=${limit}&offset=${offset}`,
    );
  }

  async createDraftListing(listing) {
    return this.request(`/application/shops/${this.shopId}/listings`, {
      method: 'POST',
      body: listing,
    });
  }

  async updateListing(listingId, fields) {
    return this.request(`/application/listings/${listingId}`, {
      method: 'PATCH',
      body: fields,
    });
  }

  async getReceipts({ minCreated, maxCreated, limit = 25, offset = 0 } = {}) {
    const params = new URLSearchParams({ limit, offset });
    if (minCreated) params.set('min_created', minCreated);
    if (maxCreated) params.set('max_created', maxCreated);
    return this.request(`/application/shops/${this.shopId}/receipts?${params}`);
  }
}

export function createEtsyClientFromEnv() {
  return new EtsyClient({
    apiKey: process.env.ETSY_API_KEY,
    accessToken: process.env.ETSY_ACCESS_TOKEN,
    shopId: process.env.ETSY_SHOP_ID,
  });
}
