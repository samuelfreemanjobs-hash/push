import { LAUNCH_GATES } from "../data/seed";

function gateClass(status: (typeof LAUNCH_GATES)[number]["status"]): string {
  return `gate gate--${status}`;
}

export function GatesPanel() {
  return (
    <section className="panel">
      <h2 className="panel__title">Launch gates (business-os.yaml)</h2>
      <p className="panel__desc">
        Revenue before parallel marketing. Do not spend on MKTLAUNCH until G5 +
        tripwire converts.
      </p>
      <ul className="gate-list">
        {LAUNCH_GATES.map((g) => (
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