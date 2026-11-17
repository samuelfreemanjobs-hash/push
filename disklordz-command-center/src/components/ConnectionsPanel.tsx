import { useState } from "react";
import { useConnections } from "../context/ConnectionsContext";
import { useOpsData } from "../context/OpsDataContext";

function StatusDot({ ok, label }: { ok: boolean; label: string }) {
  return (
    <span className={`conn-dot ${ok ? "conn-dot--ok" : "conn-dot--bad"}`}>
      {label}: {ok ? "connected" : "offline"}
    </span>
  );
}

export function ConnectionsPanel() {
  const { snapshot } = useOpsData();
  const {
    agentApiUrl,
    setAgentApiUrl,
    autoVaultSync,
    setAutoVaultSync,
    status,
    probing,
    lastVaultPullAt,
    lastVaultError,
    probeNow,
    pullVaultNow,
  } = useConnections();
  const [draftUrl, setDraftUrl] = useState(agentApiUrl ?? "");

  return (
    <div className="stack">
      <section className="panel">
        <div className="panel__head">
          <div>
            <h2 className="panel__title">Vault &amp; agent connections</h2>
            <p className="panel__desc">
              Point the PWA at your deployed <code>ai-agent-team</code> service.
              When vault credentials are configured on that server, kanban and open
              vault PRs auto-merge into the live snapshot every 10 minutes (or on
              demand below).
            </p>
          </div>
          <button
            type="button"
            className="btn"
            onClick={() => void probeNow()}
            disabled={probing}
          >
            {probing ? "Checking…" : "Test connections"}
          </button>
        </div>

        <label className="field">
          <span className="field__label">Agent API base URL</span>
          <div className="field__row">
            <input
              className="field__input"
              type="url"
              placeholder="https://your-ai-agent-team.example.com"
              value={draftUrl}
              onChange={(e) => setDraftUrl(e.target.value)}
            />
            <button
              type="button"
              className="btn btn--ghost"
              onClick={() => setAgentApiUrl(draftUrl)}
            >
              Save
            </button>
          </div>
          <span className="field__hint">
            Build-time <code>VITE_AGENT_API_URL</code> still wins when set; otherwise
            this URL is stored in your browser.
          </span>
        </label>

        <label className="checkbox-row">
          <input
            type="checkbox"
            checked={autoVaultSync}
            onChange={(e) => setAutoVaultSync(e.target.checked)}
          />
          Auto-pull vault kanban when connection test succeeds
        </label>
      </section>

      <section className="panel">
        <h3 className="panel__title">Live status</h3>
        {status ? (
          <div className="conn-status">
            <StatusDot ok={status.agent.reachable} label="Agent API" />
            <StatusDot ok={status.agent.gemini} label="Gemini" />
            <StatusDot ok={status.vault.connected} label="Vault" />
            <StatusDot
              ok={status.github.workflowDispatch}
              label="Workflow dispatch"
            />
            {status.vault.reason && !status.vault.connected ? (
              <p className="panel__desc sync-meta__warn">{status.vault.reason}</p>
            ) : null}
            {status.vault.cardCount != null && status.vault.connected ? (
              <p className="panel__desc">
                Vault kanban: {status.vault.cardCount} cards visible to the agent
                service.
              </p>
            ) : null}
            <p className="sync-meta">
              Checked {new Date(status.checkedAt).toLocaleString()}
            </p>
          </div>
        ) : (
          <p className="panel__desc">Run a connection test to see status.</p>
        )}
        {lastVaultPullAt ? (
          <p className="sync-meta">
            Last vault pull merged at{" "}
            <time dateTime={lastVaultPullAt}>
              {new Date(lastVaultPullAt).toLocaleString()}
            </time>{" "}
            — snapshot now shows {snapshot.kanban.cards.length} cards.
          </p>
        ) : null}
        {lastVaultError ? (
          <p className="sync-meta__warn">{lastVaultError}</p>
        ) : null}
        <button
          type="button"
          className="btn btn--ghost"
          onClick={() => void pullVaultNow()}
          disabled={!agentApiUrl}
        >
          Pull vault now
        </button>
      </section>

      <section className="panel">
        <h3 className="panel__title">Deploy checklist</h3>
        <ol className="numbered">
          <li>
            On <code>ai-agent-team</code>: set <code>GEMINI_API_KEY</code>, optional{" "}
            <code>GITHUB_DISPATCH_TOKEN</code>, and{" "}
            <code>DISKDLORDZ_OPS_TOKEN</code> (vault read).
          </li>
          <li>
            GitHub Actions on <code>push</code> still writes{" "}
            <code>ops-snapshot.json</code> every 6h — live pull complements that when
            you need fresher kanban between runs.
          </li>
          <li>
            Actions, agent tasks, and mobile crew use the same saved Agent API URL.
          </li>
        </ol>
      </section>
    </div>
  );
}