import { useMemo, useState } from "react";
import {
  OPS_ACTIONS,
  actionForBriefId,
} from "../data/actionCatalog";
import { useCommandCenter } from "../context/CommandCenterContext";
import { useOpsData } from "../context/OpsDataContext";

const GROUP_ORDER = ["FLOW", "DRUM", "SAAS", "SAGE", "QA", "CODE", "PM", "OPS"];

export function ActionConsolePanel() {
  const { brief } = useOpsData();
  const { triggerAction, tasks, clearDoneTasks } = useCommandCenter();
  const [filter, setFilter] = useState<string>("all");

  const grouped = useMemo(() => {
    const list =
      filter === "all"
        ? OPS_ACTIONS
        : OPS_ACTIONS.filter((a) => a.lane === filter);
    const map = new Map<string, typeof OPS_ACTIONS>();
    for (const a of list) {
      const g = map.get(a.lane) ?? [];
      g.push(a);
      map.set(a.lane, g);
    }
    return [...map.entries()].sort(
      (a, b) => GROUP_ORDER.indexOf(a[0]) - GROUP_ORDER.indexOf(b[0]),
    );
  }, [filter]);

  const lanes = useMemo(
    () => [...new Set(OPS_ACTIONS.map((a) => a.lane))].sort(),
    [],
  );

  return (
    <div className="stack">
      <section className="panel">
        <h2 className="panel__title">Action console</h2>
        <p className="panel__desc">
          Trigger workflows, copy run commands, route tasks to elite agents, or
          nudge the local kanban. Queue history is saved in this browser.
        </p>
        <div className="filter-chips">
          <button
            type="button"
            className={
              filter === "all" ? "filter-chip filter-chip--active" : "filter-chip"
            }
            onClick={() => setFilter("all")}
          >
            All lanes
          </button>
          {lanes.map((lane) => (
            <button
              key={lane}
              type="button"
              className={
                filter === lane
                  ? "filter-chip filter-chip--active"
                  : "filter-chip"
              }
              onClick={() => setFilter(lane)}
            >
              {lane}
            </button>
          ))}
        </div>
      </section>

      <section className="panel">
        <h2 className="panel__title">Run from today&apos;s brief</h2>
        <ul className="action-list">
          {brief.actions.map((a) => {
            const linked = actionForBriefId(a.id);
            return (
              <li key={a.id} className={`action-item action-item--${a.severity}`}>
                <span className="action-item__lane">{a.lane}</span>
                <strong>{a.title}</strong>
                <p>{a.detail}</p>
                <div className="action-row">
                  {linked ? (
                    <button
                      type="button"
                      className="btn"
                      onClick={() =>
                        void triggerAction(linked.id, {
                          title: linked.title,
                          lane: linked.lane,
                        })
                      }
                    >
                      Run: {linked.title}
                    </button>
                  ) : (
                    <button
                      type="button"
                      className="btn btn--ghost"
                      onClick={() =>
                        void triggerAction("sage-daily-standup", {
                          title: a.title,
                          lane: a.lane,
                        })
                      }
                    >
                      Delegate to SAGE
                    </button>
                  )}
                </div>
              </li>
            );
          })}
        </ul>
      </section>

      {grouped.map(([lane, actions]) => (
        <section key={lane} className="panel">
          <h2 className="panel__title">{lane} actions</h2>
          <ul className="trigger-grid">
            {actions.map((action) => (
              <li key={action.id} className="trigger-card">
                <div className="trigger-card__head">
                  <span className="trigger-card__kind">{action.kind}</span>
                </div>
                <h3 className="trigger-card__title">{action.title}</h3>
                <p className="trigger-card__desc">{action.description}</p>
                <button
                  type="button"
                  className="btn"
                  onClick={() =>
                    void triggerAction(action.id, {
                      title: action.title,
                      lane: action.lane,
                    })
                  }
                >
                  Trigger
                </button>
              </li>
            ))}
          </ul>
        </section>
      ))}

      <section className="panel">
        <div className="panel__head">
          <h2 className="panel__title">Task queue</h2>
          <button type="button" className="btn btn--ghost" onClick={clearDoneTasks}>
            Clear done
          </button>
        </div>
        {tasks.length === 0 ? (
          <p className="panel__desc">No tasks yet — trigger an action above.</p>
        ) : (
          <table className="data-table">
            <thead>
              <tr>
                <th>When</th>
                <th>Lane</th>
                <th>Action</th>
                <th>Status</th>
                <th>Result</th>
              </tr>
            </thead>
            <tbody>
              {tasks.map((t) => (
                <tr key={t.id}>
                  <td>{new Date(t.createdAt).toLocaleString()}</td>
                  <td>{t.lane}</td>
                  <td>
                    <code>{t.actionId}</code>
                  </td>
                  <td>
                    <span className={`task-status task-status--${t.status}`}>
                      {t.status}
                    </span>
                  </td>
                  <td>{t.result ?? "—"}</td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
      </section>
    </div>
  );
}