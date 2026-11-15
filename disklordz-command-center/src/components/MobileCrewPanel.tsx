import { useCallback, useMemo, useRef, useState } from "react";
import {
  DEFAULT_MOBILE_AGENT_ID,
  ELITE_AGENTS,
  agentApiBase,
  sendToEliteAgent,
} from "../data/eliteAgents";

type ChatLine = {
  id: string;
  role: "user" | "agent";
  prefix?: string;
  text: string;
};

export function MobileCrewPanel() {
  const apiUrl = agentApiBase();
  const [selectedId, setSelectedId] = useState(DEFAULT_MOBILE_AGENT_ID);
  const [input, setInput] = useState("");
  const [busy, setBusy] = useState(false);
  const [lines, setLines] = useState<ChatLine[]>([]);
  const listRef = useRef<HTMLDivElement>(null);

  const selected = useMemo(
    () => ELITE_AGENTS.find((a) => a.id === selectedId) ?? ELITE_AGENTS[0],
    [selectedId],
  );

  const pushLine = useCallback((line: ChatLine) => {
    setLines((prev) => [...prev, line]);
    requestAnimationFrame(() => {
      listRef.current?.scrollTo({ top: listRef.current.scrollHeight });
    });
  }, []);

  const onSend = async () => {
    const text = input.trim();
    if (!text || busy) return;
    setInput("");
    const prefixed = text.includes(":") ? text : `${selected.prefix}: ${text}`;
    pushLine({ id: crypto.randomUUID(), role: "user", text: prefixed });
    setBusy(true);
    try {
      const { aiResponse, prefix } = await sendToEliteAgent(
        prefixed,
        selectedId,
      );
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
        prefix: "SYS",
        text:
          msg === "offline"
            ? "Set VITE_AGENT_API_URL to your ai-agent-team URL (Gemini free key). See openclaw/disklordz-elite-team/README.md."
            : msg,
      });
    } finally {
      setBusy(false);
    }
  };

  return (
    <section className="panel mobile-crew">
      <h2 className="panel__title">Mobile crew (free coding)</h2>
      <p className="panel__desc">
        Elite agents with <strong>CODE</strong> as the default free coding lane.
        Messages use <code>PREFIX:</code> routing — same as Slack vault protocol.
        Install this app to your home screen for PWA use.
      </p>

      {!apiUrl ? (
        <div className="mobile-crew__banner">
          API not configured. Run <code>ai-agent-team</code> with{" "}
          <code>GEMINI_API_KEY</code>, then build with{" "}
          <code>VITE_AGENT_API_URL=http://your-host:3000</code>.
        </div>
      ) : (
        <p className="panel__desc mobile-crew__api">
          Connected: <code>{apiUrl}</code>
        </p>
      )}

      <div className="mobile-crew__roster">
        {ELITE_AGENTS.map((a) => (
          <button
            key={a.id}
            type="button"
            className={
              a.id === selectedId
                ? "mobile-crew__chip mobile-crew__chip--active"
                : "mobile-crew__chip"
            }
            onClick={() => setSelectedId(a.id)}
          >
            <span className="mobile-crew__chip-prefix">{a.prefix}</span>
            {a.coding ? <span className="mobile-crew__badge">code</span> : null}
          </button>
        ))}
      </div>

      <div className="mobile-crew__selected">
        <h3 className="agent-group__title">{selected.displayName}</h3>
        <p className="panel__desc">{selected.iAm}</p>
        <p className="mobile-crew__model">
          Free model: <code>{selected.freeModel}</code>
        </p>
      </div>

      <div ref={listRef} className="mobile-crew__chat" aria-live="polite">
        {lines.length === 0 ? (
          <p className="mobile-crew__placeholder">
            Try: <code>CODE: fix TypeScript error in vite.config</code>
          </p>
        ) : null}
        {lines.map((line) => (
          <div
            key={line.id}
            className={
              line.role === "user"
                ? "mobile-crew__line mobile-crew__line--user"
                : "mobile-crew__line mobile-crew__line--agent"
            }
          >
            {line.prefix ? (
              <span className="mobile-crew__line-prefix">{line.prefix}</span>
            ) : null}
            <pre className="mobile-crew__line-body">{line.text}</pre>
          </div>
        ))}
      </div>

      <form
        className="mobile-crew__composer"
        onSubmit={(e) => {
          e.preventDefault();
          void onSend();
        }}
      >
        <input
          className="mobile-crew__input"
          type="text"
          enterKeyHint="send"
          autoComplete="off"
          placeholder={`${selected.prefix}: your task…`}
          value={input}
          onChange={(e) => setInput(e.target.value)}
          disabled={busy}
        />
        <button type="submit" className="mobile-crew__send" disabled={busy}>
          {busy ? "…" : "Send"}
        </button>
      </form>
    </section>
  );
}