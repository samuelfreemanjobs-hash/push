export class TokenBudget {
  constructor(maxTokens = 8000) {
    this.max = maxTokens;
    this.used = 0;
    this.log = [];
  }

  record(agent, inputTokens, outputTokens, cacheReadTokens = 0) {
    const entry = { agent, inputTokens, outputTokens, cacheReadTokens };
    this.log.push(entry);
    this.used += (inputTokens - cacheReadTokens) + outputTokens;
  }

  remaining() {
    return this.max - this.used;
  }

  hasCapacity(estimatedTokens) {
    return this.remaining() > estimatedTokens;
  }

  summary() {
    return {
      maxTokens: this.max,
      usedTokens: this.used,
      remainingTokens: this.remaining(),
      agents: this.log,
    };
  }
}
