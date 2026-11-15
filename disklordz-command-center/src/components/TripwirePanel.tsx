import { useOpsData } from "../context/OpsDataContext";
import type { TripwireStep } from "../data/types";

function statusClass(status: TripwireStep["status"]): string {
  switch (status) {
    case "done":
      return "step step--done";
    case "active":
      return "step step--active";
    case "blocked":
      return "step step--blocked";
    case "queued":
      return "step step--queued";
    default: {
      const _exhaustive: never = status;
      void _exhaustive;
      return "step step--queued";
    }
  }
}

export function TripwirePanel() {
  const { snapshot } = useOpsData();

  return (
    <section className="panel">
      <header className="panel__head">
        <div>
          <h2 className="panel__title">Tripwire — Starter Trap Pack</h2>
          <p className="panel__desc">
            SKU <code>tripwire-sample-v1</code> · $9 · +75 credits · 5 WAVs. Not
            a VST — WAV pack only. Grok Drum Kit Architect → kit-spec; OpenClaw
            factory team → <code>openclaw/drum-kit-factory/</code>.
          </p>
        </div>
      </header>
      <ol className="tripwire-steps">
        {snapshot.tripwireSteps.map((step) => (
          <li key={step.id} className={statusClass(step.status)}>
            <div className="step__head">
              <span className="step__id">{step.id}</span>
              <span className="step__owner">{step.owner}</span>
              <span className="step__status">{step.status}</span>
            </div>
            <div className="step__title">{step.title}</div>
            {step.detail ? (
              <p className="step__detail">{step.detail}</p>
            ) : null}
          </li>
        ))}
      </ol>
      <div className="callout">
        <strong>Smoke test:</strong> sign in → Get starter pack → Stripe test card
        → credits + download ZIP with 5 WAVs + README.
      </div>
    </section>
  );
}