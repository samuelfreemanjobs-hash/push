export function workflow(slug, title, job, inputs, brief, qc) {
  return { slug, title, job, inputs, brief, qc };
}

export function buildKit({ slug, name, tagline, audience, outcome, kpi, playbookFocus, setupSteps, workflowDefs }) {
  const readme = `# ${name} — [YOUR BRAND]

${tagline}

## What you get
- **AGENT-PROFILE.md** — role, inputs, outputs, KPIs
- **01-playbook/** — operating rhythm
- **02-workflows/** — 10 copy-paste agent briefs
- **05-implementation/** — 15-minute setup

## Who this is for
${audience}

## Quick start
${setupSteps.map((s, i) => `${i + 1}. ${s}`).join('\n')}

Replace every \`[YOUR BRAND]\` placeholder before you run workflows.
`;

  const profile = `# Agent profile — ${name}

**Outcome:** ${outcome}

**Inputs:** Offer, audience, brand voice, and context noted in each workflow.

**Outputs:** Drafts ready for human review before sending or publishing.

**KPI:** ${kpi}

**Guardrails:** No guarantees, no medical/legal advice; follow platform policies (Etsy, email, ads).
`;

  const playbook = `# ${name} — Playbook

## Purpose
${playbookFocus}

## Weekly rhythm (60–90 min)
- **Plan:** Pick one workflow tied to a live business priority.
- **Run:** Execute agent brief; save output in a dated folder.
- **Review:** Complete quality checks; edit for voice and facts.
- **Ship:** Publish, send, or file in your ops system.

## Quality bar
Every external-facing output must name the buyer's problem, state what's included, and end with one clear next step. Reject generic AI tone and unverifiable claims.

## Escalation
Legal, safety, or harassment issues → pause automation and handle manually.
`;

  const setup = `# 15-minute setup — ${name}

## 0–5 min
Create **[YOUR BRAND] ${name} context** doc: offer, audience, tone (3 adjectives), words to avoid, support email.

## 5–10 min
Pin **AGENT-PROFILE.md** in your AI tool. Create folder \`agents/${slug}/\` for saved runs.

## 10–15 min
Run workflow **01** on a real scenario (not fictional). Edit the draft before any send/publish.

You are ready for the full playbook loop.
`;

  const workflows = workflowDefs.map((d) => {
    const brief = d.briefIsFull
      ? d.brief
      : `You are a specialist operator for [YOUR BRAND]. ${d.brief}

Deliver in markdown with clear headings. Be specific to the inputs provided. Do not invent facts, metrics, or testimonials. Flag assumptions in a short "Assumptions" section at the end.`;
    return workflow(d.slug, d.title, d.job, d.inputs, brief, d.qc || [
      'Reads naturally in brand voice',
      'One clear CTA or next step',
      'No policy-violating claims',
    ]);
  });

  return { slug, readme, profile, playbook, setup, workflows, templates: undefined };
}
