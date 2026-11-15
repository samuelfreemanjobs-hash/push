import { AGENT_LANES, KANBAN_CARDS, NORTH_STAR, REPOS, VST_FLEET } from "../data/seed";

export function Overview() {
  const p0Ready = KANBAN_CARDS.filter(
    (c) => c.priority === "P0" && c.column !== "done",
  ).length;
  const blocked = KANBAN_CARDS.filter((c) => c.column === "blocked").length;
  const vstGreen = VST_FLEET.filter((v) => v.ciStatus === "success").length;
  const vstRed = VST_FLEET.filter(
    (v) => v.ciStatus === "failure" || v.ciStatus === "stale",
  ).length;

  return (
    <div className="stack">
      <section className="hero">
        <h1 className="hero__title">DiskLordz Command Center</h1>
        <p className="hero__lead">
          Single pane for solo founder ops: revenue path (tripwire), factory
          throughput, vault merge queue, and agent lanes. Data snapshot: 2026-11-15.
        </p>
      </section>

      <div className="stat-grid">
        <StatCard label="Open P0 cards" value={String(p0Ready)} />
        <StatCard label="Blocked cards" value={String(blocked)} />
        <StatCard label="VST CI green" value={`${vstGreen} SKUs`} />
        <StatCard label="CI attention" value={`${vstRed} items`} />
      </div>

      <section className="panel">
        <h2 className="panel__title">North star</h2>
        <p className="panel__desc mono">{NORTH_STAR.metric}</p>
        <ul className="tag-list">
          {NORTH_STAR.supporting.map((s) => (
            <li key={s} className="tag">
              {s}
            </li>
          ))}
        </ul>
      </section>

      <section className="panel">
        <h2 className="panel__title">Source repos</h2>
        <ul className="repo-list">
          {REPOS.map((r) => (
            <li key={r.href}>
              <a href={r.href} target="_blank" rel="noreferrer">
                {r.label}
              </a>
              <span>{r.role}</span>
            </li>
          ))}
        </ul>
      </section>

      <section className="panel">
        <h2 className="panel__title">Today&apos;s three (from SAGE meeting)</h2>
        <ol className="numbered">
          <li>
            <strong>DRUM</strong> — QC tripwire WAV pack → launch assets
          </li>
          <li>
            <strong>SAAS</strong> — Apply tripwire checkout to Instruments +
            Vercel env
          </li>
          <li>
            <strong>FLOW</strong> — Push integration bundle (Instruments PAT)
          </li>
        </ol>
      </section>

      <section className="panel panel--compact">
        <h2 className="panel__title">Active agent lanes (sample)</h2>
        <p className="panel__desc">
          Full roster: vault <code>agents/disklordz/slack-agents.yaml</code> (16
          lanes).
        </p>
        <div className="agent-chips">
          {AGENT_LANES.slice(0, 6).map((a) => (
            <span key={a.id} className="agent-chip">
              {a.prefix}: {a.displayName}
            </span>
          ))}
        </div>
      </section>
    </div>
  );
}

function StatCard({ label, value }: { label: string; value: string }) {
  return (
    <div className="stat-card">
      <div className="stat-card__value">{value}</div>
      <div className="stat-card__label">{label}</div>
    </div>
  );
}