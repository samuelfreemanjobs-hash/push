import { ELITE_AGENTS } from "../data/eliteAgents";
import { AGENT_LANES } from "../data/seed";

export function AgentsPanel() {
  const eliteByGroup = ELITE_AGENTS.reduce<
    Record<string, typeof ELITE_AGENTS>
  >((acc, agent) => {
    const list = acc[agent.group] ?? [];
    list.push(agent);
    acc[agent.group] = list;
    return acc;
  }, {});
  const byGroup = AGENT_LANES.reduce<Record<string, typeof AGENT_LANES>>(
    (acc, agent) => {
      const list = acc[agent.group] ?? [];
      list.push(agent);
      acc[agent.group] = list;
      return acc;
    },
    {},
  );

  return (
    <section className="panel">
      <h2 className="panel__title">Slack agent lanes</h2>
      <p className="panel__desc">
        Speak in <code>#all-disklordz</code> or{" "}
        <code>#disklordz-meeting</code> with prefix + identity contract (vault
        MEETING-PROTOCOL.md). Showing core lanes; full roster has 16 entries.
        For phone coding when Cursor is down, use{" "}
        <strong>Mobile crew</strong> or OpenClaw <code>CODE:</code>.
      </p>
      <h3 className="agent-group__title">Elite roster (free tier)</h3>
      <p className="panel__desc">
        Config: <code>openclaw/disklordz-elite-team/</code>
      </p>
      {Object.entries(eliteByGroup).map(([group, agents]) => (
        <div key={`elite-${group}`} className="agent-group">
          <h3 className="agent-group__title">Elite · {group}</h3>
          <ul className="agent-list">
            {agents.map((a) => (
              <li key={a.id} className="agent-list__item">
                <span className="agent-list__prefix">{a.prefix}:</span>
                <span className="agent-list__name">{a.displayName}</span>
                <span className="agent-list__iam">{a.iAm}</span>
                {a.coding ? (
                  <span className="mobile-crew__badge">code</span>
                ) : null}
              </li>
            ))}
          </ul>
        </div>
      ))}
      <h3 className="agent-group__title">Slack lanes</h3>
      {Object.entries(byGroup).map(([group, agents]) => (
        <div key={group} className="agent-group">
          <h3 className="agent-group__title">{group}</h3>
          <ul className="agent-list">
            {agents.map((a) => (
              <li key={a.id} className="agent-list__item">
                <span className="agent-list__prefix">{a.prefix}:</span>
                <span className="agent-list__name">{a.displayName}</span>
                <span className="agent-list__iam">{a.iAm}</span>
              </li>
            ))}
          </ul>
        </div>
      ))}
    </section>
  );
}