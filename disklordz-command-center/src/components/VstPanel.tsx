import { VST_FLEET } from "../data/seed";

function ciBadge(status: (typeof VST_FLEET)[number]["ciStatus"]): string {
  return `ci ci--${status}`;
}

export function VstPanel() {
  return (
    <section className="panel">
      <h2 className="panel__title">VST fleet — Instruments main</h2>
      <p className="panel__desc">
        HEAD <code>a1a1c298</code> (2026-11-15). Latest dedicated workflow run per
        SKU where available.
      </p>
      <table className="data-table">
        <thead>
          <tr>
            <th>SKU</th>
            <th>Path</th>
            <th>Workflow</th>
            <th>CI</th>
            <th>Date</th>
          </tr>
        </thead>
        <tbody>
          {VST_FLEET.map((row) => (
            <tr key={row.name}>
              <td>{row.name}</td>
              <td>
                <code>{row.path}</code>
              </td>
              <td>{row.workflow}</td>
              <td>
                <span className={ciBadge(row.ciStatus)}>{row.ciStatus}</span>
              </td>
              <td>{row.ciDate ?? "—"}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </section>
  );
}