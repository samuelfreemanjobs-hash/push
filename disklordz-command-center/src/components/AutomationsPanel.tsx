import { useOpsData } from "../context/OpsDataContext";

const PIPELINE = [
  {
    name: "Vault → snapshot",
    schedule: "Every 6h + on demand (GitHub Actions)",
    detail: "Pulls kanban.yaml, open vault PRs, Instruments workflow CI → ops-snapshot.json",
  },
  {
    name: "Command Center",
    schedule: "On load + Refresh",
    detail: "Merges live snapshot with your kanban column overrides in localStorage",
  },
  {
    name: "Daily brief engine",
    schedule: "Each page view",
    detail: "Derives P0 actions from gates, WIP, tripwire card K-012, PR #126, nightly QA",
  },
  {
    name: "Republish",
    schedule: "After merge to default branch",
    detail: "Confirm publish in Cursor so https://disklordz-command-center.apps.grok.bot picks up new snapshot",
  },
];

export function AutomationsPanel() {
  const { snapshot, loading, refresh, brief } = useOpsData();
  const generated = new Date(snapshot.generatedAt).toLocaleString();
  const hasErrors = snapshot.syncErrors.length > 0;

  return (
    <div className="stack">
      <section className="panel">
        <div className="panel__head">
          <div>
            <h2 className="panel__title">Business automation</h2>
            <p className="panel__desc">
              Vault and GitHub stay authoritative; this app automates ingest and
              daily prioritization. Your board moves are local until you commit
              them back in the vault kanban.
            </p>
          </div>
          <button type="button" className="btn" onClick={refresh} disabled={loading}>
            {loading ? "Refreshing…" : "Refresh snapshot"}
          </button>
        </div>
        <p className="sync-meta">
          Snapshot: <time dateTime={snapshot.generatedAt}>{generated}</time>
          {hasErrors ? (
            <span className="sync-meta__warn"> — partial sync (using fallback where needed)</span>
          ) : null}
        </p>
      </section>

      <section className="panel">
        <h2 className="panel__title">Today&apos;s automated brief</h2>
        <p className="panel__desc mono">North star: {brief.northStar}</p>
        <ul className="action-list">
          {brief.actions.map((a) => (
            <li key={a.id} className={`action-item action-item--${a.severity}`}>
              <span className="action-item__lane">{a.lane}</span>
              <strong>{a.title}</strong>
              <p>{a.detail}</p>
            </li>
          ))}
        </ul>
        {brief.actions.length === 0 ? (
          <p className="panel__desc">No automated blockers — pick the top P0 ready card.</p>
        ) : null}
      </section>

      <section className="panel">
        <h2 className="panel__title">15-minute daily ritual</h2>
        <ol className="numbered">
          {brief.rituals.map((r) => (
            <li key={r}>{r}</li>
          ))}
        </ol>
      </section>

      <section className="panel">
        <h2 className="panel__title">Automation pipeline</h2>
        <table className="data-table">
          <thead>
            <tr>
              <th>Stage</th>
              <th>Cadence</th>
              <th>What runs</th>
            </tr>
          </thead>
          <tbody>
            {PIPELINE.map((p) => (
              <tr key={p.name}>
                <td>{p.name}</td>
                <td>{p.schedule}</td>
                <td>{p.detail}</td>
              </tr>
            ))}
          </tbody>
        </table>
        <p className="panel__desc">
          Workflow file: <code>.github/workflows/disklordz-ops-sync.yml</code> ·
          Local sync: <code>npm run sync:ops</code>
        </p>
      </section>
    </div>
  );
}