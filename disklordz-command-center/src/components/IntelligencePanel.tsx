import { useCallback, useRef, useState } from "react";
import { sendToEliteAgent } from "../data/eliteAgents";
import { useIntelligence } from "../context/IntelligenceContext";

type ChatLine = {
  id: string;
  role: "user" | "agent";
  prefix?: string;
  text: string;
};

function priorityClass(p: string) {
  if (p === "immediate") return "intel-priority intel-priority--now";
  if (p === "next") return "intel-priority intel-priority--next";
  return "intel-priority intel-priority--later";
}

export function IntelligencePanel() {
  const { intelligence, loading, refresh } = useIntelligence();
  const [input, setInput] = useState(intelligence.quickPrompts[0] ?? "RADAR: review latest AI Dev Signals issue");
  const [busy, setBusy] = useState(false);
  const [lines, setLines] = useState<ChatLine[]>([]);
  const listRef = useRef<HTMLDivElement>(null);

  const pushLine = useCallback((line: ChatLine) => {
    setLines((prev) => [...prev, line]);
    requestAnimationFrame(() => {
      listRef.current?.scrollTo({ top: listRef.current.scrollHeight });
    });
  }, []);

  const onSend = async (textOverride?: string) => {
    const text = (textOverride ?? input).trim();
    if (!text || busy) return;
    if (!textOverride) setInput("");
    const prefixed = text.startsWith("RADAR:")
      ? text
      : `RADAR: ${text}`;
    pushLine({ id: crypto.randomUUID(), role: "user", text: prefixed });
    setBusy(true);
    try {
      const { aiResponse, prefix } = await sendToEliteAgent(prefixed, "radar");
      pushLine({
        id: crypto.randomUUID(),
        role: "agent",
        prefix,
        text: aiResponse,
      });
    } catch (e) {
      const msg = e instanceof Error ? e.message : "Request failed";
      pushLine({
        id: crypto.randomUUID(),
        role: "agent",
        prefix: "RADAR",
        text:
          msg === "offline"
            ? "Mobile RADAR needs VITE_AGENT_API_URL (ai-agent-team). On VPS use OpenClaw: RADAR: … and npm run tech-radar for intake."
            : msg,
      });
    } finally {
      setBusy(false);
    }
  };

  const snapshotDate = new Date(intelligence.generatedAt).toLocaleString();
  const m1 = intelligence.factoryMilestone;
  const m1Done = m1.stories.filter((s) => s.passes).length;
  const m1Total = m1.stories.length;
  const radar = intelligence.techRadar;

  return (
    <div className="stack intelligence">
      <section className="hero hero--compact">
        <h1 className="hero__title">{intelligence.agent.displayName}</h1>
        <p className="hero__lead">
          <strong>{intelligence.agent.prefix}</strong> screens{" "}
          <a href={intelligence.agent.sourceUrl} target="_blank" rel="noreferrer">
            {intelligence.agent.source}
          </a>{" "}
          against twelve DiskLordz opportunities — no article hoarding.{" "}
          {loading ? "Refreshing snapshot…" : `Data ${snapshotDate}.`}
        </p>
        <IntelRefresh onRefresh={refresh} busy={busy} />
      </section>

      <IntelStatGrid
        feedError={radar.feed_error}
        inboxPending={radar.inbox_pending}
        m1Done={m1Done}
        m1Total={m1Total}
        complete={m1.complete}
      />

      <section className="panel">
        <h2 className="panel__title">RADAR agent</h2>
        <p className="panel__desc">
          OpenClaw skill <code>{intelligence.agent.skill}</code>. Intake:{" "}
          <code>disklordz-agent-stack → npm run tech-radar</code>. Docs:{" "}
          <code>{intelligence.agent.doc}</code>.
        </p>
        <div className="intel-prompts">
          {intelligence.quickPrompts.map((p) => (
            <button
              key={p}
              type="button"
              className="intel-prompts__chip"
              disabled={busy}
              onClick={() => void onSend(p)}
            >
              {p}
            </button>
          ))}
        </div>
        <div className="mobile-crew__chat" ref={listRef}>
          {lines.length === 0 ? (
            <p className="panel__desc">
              Ask RADAR to review the latest newsletter or map a tool to
              opportunity ranks 1–5.
            </p>
          ) : (
            lines.map((line) => (
              <div
                key={line.id}
                className={
                  line.role === "user"
                    ? "mobile-crew__line mobile-crew__line--user"
                    : "mobile-crew__line mobile-crew__line--agent"
                }
              >
                {line.prefix ? (
                  <span className="mobile-crew__prefix">{line.prefix}:</span>
                ) : null}
                <span>{line.text}</span>
              </div>
            ))
          )}
        </div>
        <form
          className="mobile-crew__form"
          onSubmit={(e) => {
            e.preventDefault();
            void onSend();
          }}
        >
          <input
            className="mobile-crew__input"
            value={input}
            onChange={(e) => setInput(e.target.value)}
            placeholder="RADAR: review latest issue…"
            disabled={busy}
          />
          <button type="submit" className="mobile-crew__send" disabled={busy}>
            {busy ? "…" : "Send"}
          </button>
        </form>
      </section>

      <section className="panel">
        <h2 className="panel__title">Latest feed</h2>
        {radar.latest_issue ? (
          <>
            <p className="panel__desc">
              {radar.latest_issue.link ? (
                <a href={radar.latest_issue.link} target="_blank" rel="noreferrer">
                  {radar.latest_issue.title}
                </a>
              ) : (
                radar.latest_issue.title
              )}
            </p>
            <ul className="intel-feed">
              {radar.recent_items.map((item) => (
                <li key={item.title}>
                  {item.link ? (
                    <a href={item.link} target="_blank" rel="noreferrer">
                      {item.title}
                    </a>
                  ) : (
                    item.title
                  )}
                </li>
              ))}
            </ul>
          </>
        ) : (
          <p className="panel__desc">
            No RSS ingest yet. Run <code>npm run tech-radar</code> on your VPS or
            paste inbox at <code>hermes-out/research/tech-radar/inbox/issue.md</code>.
            {radar.feed_error ? ` (${radar.feed_error})` : null}
          </p>
        )}
      </section>

      <section className="panel">
        <h2 className="panel__title">Factory milestone M1</h2>
        <p className="panel__desc">{m1.title}</p>
        <ul className="intel-milestone">
          {m1.stories.map((s) => (
            <li
              key={s.id}
              className={s.passes ? "intel-milestone__done" : "intel-milestone__open"}
            >
              <code>{s.id}</code> {s.title}
            </li>
          ))}
        </ul>
      </section>

      <section className="panel">
        <h2 className="panel__title">Opportunity stack</h2>
        <div className="intel-table-wrap">
          <table className="intel-table">
            <thead>
              <tr>
                <th>Rank</th>
                <th>Opportunity</th>
                <th>Benefit</th>
                <th>Build</th>
              </tr>
            </thead>
            <tbody>
              {intelligence.opportunities.map((o) => (
                <tr key={o.id}>
                  <td>{o.rank}</td>
                  <td>{o.title}</td>
                  <td>{o.businessBenefit}</td>
                  <td>
                    <span className={priorityClass(o.buildPriority)}>
                      {o.buildPriority}
                    </span>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </section>

      <section className="panel panel--compact">
        <h2 className="panel__title">Screening categories</h2>
        <ul className="intel-categories">
          {intelligence.categories.map((c) => (
            <li key={c.id}>
              <strong>{c.label}</strong> — {c.watchFor}
            </li>
          ))}
        </ul>
      </section>
    </div>
  );
}

function IntelRefresh({
  onRefresh,
  busy,
}: {
  onRefresh: () => void;
  busy: boolean;
}) {
  return (
    <button
      type="button"
      className="intel-refresh"
      onClick={onRefresh}
      disabled={busy}
    >
      Refresh snapshot
    </button>
  );
}

function IntelStatGrid(props: {
  feedError: string | null;
  inboxPending: boolean;
  m1Done: number;
  m1Total: number;
  complete: boolean;
}) {
  const feedLabel = props.feedError
    ? "RSS blocked / paste inbox"
    : props.inboxPending
      ? "Inbox ready"
      : "Feed idle";
  return (
    <div className="stat-grid">
      <div className="stat-card">
        <span className="stat-card__label">Feed status</span>
        <span className="stat-card__value">{feedLabel}</span>
      </div>
      <div className="stat-card">
        <span className="stat-card__label">Factory M1</span>
        <span className="stat-card__value">
          {props.m1Done}/{props.m1Total}
        </span>
      </div>
      <div className="stat-card">
        <span className="stat-card__label">M1 ship gate</span>
        <span className="stat-card__value">
          {props.complete ? "Complete" : "In progress"}
        </span>
      </div>
    </div>
  );
}