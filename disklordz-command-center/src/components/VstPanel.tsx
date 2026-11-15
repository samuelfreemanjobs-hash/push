import { useOpsData } from "../context/OpsDataContext";
import type { VstSku } from "../data/types";

function ciBadge(status: VstSku["ciStatus"]): string {
  return `ci ci--${status}`;
}

export function VstPanel() {
  const { snapshot } = useOpsData();

  return (
    <section className="panel">
      <h2 className="panel__title">VST fleet — Instruments main</h2>
      <p className="panel__desc">
        Latest dedicated workflow run per SKU from GitHub Actions (automated sync).
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
          {snapshot.vstFleet.map((row) => (
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