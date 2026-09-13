/** Build a 100-point scorecard markdown table. */
export function buildScorecard(agentName, criteria, passThreshold = 85) {
  const rows = criteria
    .map(
      (c) =>
        `| ${c.name} | ${c.weight} | ___ / ${c.weight} | ${c.hint} |`,
    )
    .join('\n');
  const weights = criteria.reduce((s, c) => s + c.weight, 0);
  return `# Scorecard — ${agentName}

**Pass threshold:** ${passThreshold}/100 before you ship to customers or publish.

| Criterion | Weight | Your score | How to score |
|-----------|--------|------------|--------------|
${rows}
| **Total** | **${weights}** | **___ / ${weights}** | |

## Reviewer sign-off

- [ ] I replaced all \`[YOUR BRAND]\` placeholders  
- [ ] I completed quality checks on the workflow used  
- [ ] Facts, prices, and policies are accurate  
- [ ] A second human skimmed customer-facing copy  

**Reviewer:** _______________ **Date:** _______________
`;
}
