import { useOpsData } from "../context/OpsDataContext";

export function Overview() {
  const { snapshot, brief, agentLanes, repos, loading } = useOpsData();
  const cards = snapshot.kanban.cards;

  const p0Open = cards.filter(
    (c) => c.priority === "P0" && c.column !== "done",
  ).length;
  const blocked = cards.filter((c) => c.column === "blocked").length;
  const vstGreen = snapshot.vstFleet.filter((v) => v.ciStatus === "success").length;
  const vstRed = snapshot.vstFleet.filter(
    (v) => v.ciStatus === "failure" || v.ciStatus === "stale",
  ).length;

  const snapshotDate = new Date(snapshot.generatedAt).toLocaleDateString();
  const topActions = brief.actions.filter((a) => a.severity === "critical").slice(0, 3);

  return (
    <div className="stack">
      <section className="hero">
        <h1 className="hero__title">DiskLordz Command Center</h1>
        <p className="hero__lead">
          Automated solo founder ops: live vault kanban ingest, GitHub CI, merge
          queue, and daily brief. {loading ? "Refreshing…" : `Snapshot ${snapshotDate}.`}
        </p>
      </section>

      <div className="stat-grid">
        <StatCard label="Open P0 cards" value={String(p0Open)} />
        <StatCard label="Blocked cards" value={String(blocked)} />
        <StatCard label="VST CI green" value={`${vstGreen} SKUs`} />
        <StatCard label="CI attention" value={`${vstRed} items`} />
      </div>

      <section className="panel">
        <h2 className="panel__title">North star</h2>
        <p className="panel__desc mono">{snapshot.northStar.metric}</p>
        <ul className="tag-list">
          {snapshot.northStar.supporting.map((s) => (
            <li key={s} className="tag">
              {s}
            </li>
          ))}
        </ul>
      </section>

      <section className="panel">
        <h2 className="panel__title">Automated focus (now)</h2>
        {topActions.length > 0 ? (
          <ol className="numbered">
            {topActions.map((a) => (
              <li key={a.id}>
                <strong>{a.lane}</strong> — {a.title}
              </li>
            ))}
          </ol>
        ) : (
          <p className="panel__desc">No critical automated items — check Kanban ready column.</p>
        )}
      </section>

      <section className="panel">
        <h2 className="panel__title">Source repos</h2>
        <ul className="repo-list">
          {repos.map((r) => (
            <li key={r.href}>
              <a href={r.href} target="_blank" rel="noreferrer">
                {r.label}
              </a>
              <span>{r.role}</span>
            </li>
          ))}
        </ul>
      </section>

      <section className="panel panel--compact">
        <h2 className="panel__title">Active agent lanes (sample)</h2>
        <p className="panel__desc">
          Full roster: vault <code>agents/disklordz/slack-agents.yaml</code> (16
          lanes).
        </p>
        <div className="agent-chips">
          {agentLanes.slice(0, 6).map((a) => (
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