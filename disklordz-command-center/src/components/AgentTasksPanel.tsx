import { useMemo, useState } from "react";
import { ELITE_AGENTS } from "../data/eliteAgents";
import { useCommandCenter } from "../context/CommandCenterContext";
import { useOpsData } from "../context/OpsDataContext";
import { actionForBriefId } from "../data/actionCatalog";

export function AgentTasksPanel() {
  const { brief } = useOpsData();
  const {
    assignments,
    assignTask,
    runAssignment,
    runAllAssigned,
    removeAssignment,
    clearCompletedAssignments,
  } = useCommandCenter();

  const [title, setTitle] = useState("");
  const [instructions, setInstructions] = useState("");
  const [agentId, setAgentId] = useState("sage");
  const [expandedId, setExpandedId] = useState<string | null>(null);

  const selectedAgent = useMemo(
    () => ELITE_AGENTS.find((a) => a.id === agentId) ?? ELITE_AGENTS[0],
    [agentId],
  );

  const openCount = assignments.filter(
    (a) => a.status === "assigned" || a.status === "running",
  ).length;

  const onAssign = (runNow: boolean) => {
    const t = title.trim();
    const body = instructions.trim();
    if (!t || !body) return;
    assignTask(
      {
        title: t,
        instructions: body,
        agentId,
        lane: selectedAgent.prefix,
      },
      { runNow },
    );
    setTitle("");
    setInstructions("");
  };

  const assignFromBrief = (briefId: string, briefTitle: string, lane: string) => {
    const linked = actionForBriefId(briefId);
    const agentIdGuess =
      linked?.agentId ??
      (lane === "FLOW"
        ? "flow"
        : lane === "DRUM"
          ? "trip"
          : lane === "QA"
            ? "review"
            : "sage");
    assignTask({
      title: briefTitle,
      instructions: linked?.agentMessage ?? briefTitle,
      agentId: agentIdGuess,
      lane,
      briefActionId: briefId,
    });
  };

  return (
    <div className="stack">
      <section className="panel">
        <h2 className="panel__title">Assign tasks to agents</h2>
        <p className="panel__desc">
          Create a task, pick an elite agent, and run it. The agent returns a
          deliverable you can act on (checklist, steps, draft). Stored in this
          browser; execution uses <code>ai-agent-team</code> when{" "}
          <code>VITE_AGENT_API_URL</code> is set.
        </p>
        <p className="sync-meta">
          Open assignments: {openCount} · Total: {assignments.length}
        </p>
      </section>

      <section className="panel">
        <h2 className="panel__title">New assignment</h2>
        <div className="task-form">
          <label className="task-form__label">
            Title
            <input
              className="task-form__input"
              value={title}
              onChange={(e) => setTitle(e.target.value)}
              placeholder="e.g. Resolve vault PR #126 conflict plan"
            />
          </label>
          <label className="task-form__label">
            Instructions
            <textarea
              className="task-form__textarea"
              value={instructions}
              onChange={(e) => setInstructions(e.target.value)}
              rows={4}
              placeholder="What should the agent produce? Be specific about repos and done criteria."
            />
          </label>
          <label className="task-form__label">
            Agent
            <select
              className="task-form__input"
              value={agentId}
              onChange={(e) => setAgentId(e.target.value)}
            >
              {ELITE_AGENTS.map((a) => (
                <option key={a.id} value={a.id}>
                  {a.prefix} — {a.displayName}
                </option>
              ))}
            </select>
          </label>
          <p className="panel__desc">{selectedAgent.iAm}</p>
          <div className="task-form__actions">
            <button
              type="button"
              className="btn"
              disabled={!title.trim() || !instructions.trim()}
              onClick={() => onAssign(false)}
            >
              Assign
            </button>
            <button
              type="button"
              className="btn"
              disabled={!title.trim() || !instructions.trim()}
              onClick={() => onAssign(true)}
            >
              Assign &amp; run
            </button>
          </div>
        </div>
      </section>

      <section className="panel">
        <h2 className="panel__title">Assign from daily brief</h2>
        <ul className="action-list">
          {brief.actions.map((a) => (
            <li key={a.id} className={`action-item action-item--${a.severity}`}>
              <span className="action-item__lane">{a.lane}</span>
              <strong>{a.title}</strong>
              <p>{a.detail}</p>
              <div className="action-row">
                <button
                  type="button"
                  className="btn btn--tiny"
                  onClick={() => assignFromBrief(a.id, a.title, a.lane)}
                >
                  Assign
                </button>
                <button
                  type="button"
                  className="btn btn--tiny btn--ghost"
                  onClick={() => {
                    const linked = actionForBriefId(a.id);
                    assignTask(
                      {
                        title: a.title,
                        instructions:
                          linked?.agentMessage ?? `${a.title}\n\n${a.detail}`,
                        agentId:
                          linked?.agentId ??
                          (a.lane === "FLOW" ? "flow" : "sage"),
                        lane: a.lane,
                        briefActionId: a.id,
                      },
                      { runNow: true },
                    );
                  }}
                >
                  Assign &amp; run
                </button>
              </div>
            </li>
          ))}
        </ul>
      </section>

      <section className="panel">
        <div className="panel__head">
          <h2 className="panel__title">Agent inbox</h2>
          <div className="task-form__actions">
            <button type="button" className="btn btn--ghost" onClick={() => void runAllAssigned()}>
              Run all assigned
            </button>
            <button
              type="button"
              className="btn btn--ghost"
              onClick={clearCompletedAssignments}
            >
              Clear completed
            </button>
          </div>
        </div>
        {assignments.length === 0 ? (
          <p className="panel__desc">No assignments yet.</p>
        ) : (
          <ul className="assignment-list">
            {assignments.map((task) => (
              <li key={task.id} className="assignment-card">
                <div className="assignment-card__head">
                  <span className="assignment-card__lane">
                    {task.lane ?? task.agentId}
                  </span>
                  <span
                    className={`task-status task-status--${
                      task.status === "completed" ? "done" : task.status
                    }`}
                  >
                    {task.status}
                  </span>
                </div>
                <h3 className="assignment-card__title">{task.title}</h3>
                <p className="assignment-card__instr">{task.instructions}</p>
                <div className="action-row">
                  {(task.status === "assigned" || task.status === "failed") && (
                    <button
                      type="button"
                      className="btn btn--tiny"
                      onClick={() => void runAssignment(task.id)}
                    >
                      Run
                    </button>
                  )}
                  {task.output ? (
                    <button
                      type="button"
                      className="btn btn--tiny btn--ghost"
                      onClick={() =>
                        setExpandedId(expandedId === task.id ? null : task.id)
                      }
                    >
                      {expandedId === task.id ? "Hide output" : "View output"}
                    </button>
                  ) : null}
                  <button
                    type="button"
                    className="btn btn--tiny btn--ghost"
                    onClick={() => removeAssignment(task.id)}
                  >
                    Remove
                  </button>
                </div>
                {task.error ? (
                  <p className="assignment-card__error">{task.error}</p>
                ) : null}
                {expandedId === task.id && task.output ? (
                  <pre className="assignment-card__output">{task.output}</pre>
                ) : null}
              </li>
            ))}
          </ul>
        )}
      </section>
    </div>
  );
}