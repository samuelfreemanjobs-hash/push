import { useOpsData } from "../context/OpsDataContext";
import type { LaunchGate } from "../data/types";

function gateClass(status: LaunchGate["status"]): string {
  return `gate gate--${status}`;
}

export function GatesPanel() {
  const { snapshot } = useOpsData();

  return (
    <section className="panel">
      <h2 className="panel__title">Launch gates (automated from kanban)</h2>
      <p className="panel__desc">
        Revenue before parallel marketing. Do not spend on MKTLAUNCH until G5 +
        tripwire converts.
      </p>
      <ul className="gate-list">
        {snapshot.launchGates.map((g) => (
          <li key={g.id} className={gateClass(g.status)}>
            <span className="gate__id">{g.id}</span>
            <span className="gate__title">{g.title}</span>
            <span className="gate__status">{g.status}</span>
            {g.doc ? <code className="gate__doc">{g.doc}</code> : null}
          </li>
        ))}
      </ul>
    </section>
  );
}