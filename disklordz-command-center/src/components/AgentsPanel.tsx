import { AGENT_LANES } from "../data/seed";

export function AgentsPanel() {
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
      </p>
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