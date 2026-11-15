import type { KanbanCard, KanbanColumnId } from "../data/types";

const COLUMNS: { id: KanbanColumnId; title: string }[] = [
  { id: "backlog", title: "Backlog" },
  { id: "ready", title: "Ready" },
  { id: "in_progress", title: "In progress" },
  { id: "blocked", title: "Blocked" },
  { id: "done", title: "Done" },
];

type KanbanBoardProps = {
  cards: KanbanCard[];
  wipLimit: number;
  onMove: (cardId: string, column: KanbanColumnId) => void;
  onReset: () => void;
};

function priorityClass(p: KanbanCard["priority"]): string {
  switch (p) {
    case "P0":
      return "pill pill--p0";
    case "P1":
      return "pill pill--p1";
    default:
      return "pill pill--p2";
  }
}

export function KanbanBoard({
  cards,
  wipLimit,
  onMove,
  onReset,
}: KanbanBoardProps) {
  const inProgress = cards.filter((c) => c.column === "in_progress").length;

  return (
    <section className="panel">
      <header className="panel__head">
        <div>
          <h2 className="panel__title">Solo kanban</h2>
          <p className="panel__desc">
            Synced from vault <code>agents/disklordz/kanban.yaml</code>. Moves
            persist in this browser. WIP limit: {wipLimit} ({inProgress} active).
          </p>
        </div>
        <button type="button" className="btn btn--ghost" onClick={onReset}>
          Reset to vault seed
        </button>
      </header>
      <div className="kanban">
        {COLUMNS.map((col) => {
          const colCards = cards.filter((c) => c.column === col.id);
          return (
            <div key={col.id} className="kanban__col">
              <div className="kanban__col-head">
                <span>{col.title}</span>
                <span className="kanban__count">{colCards.length}</span>
              </div>
              <div className="kanban__cards">
                {colCards.map((card) => (
                  <article key={card.id} className="kanban-card">
                    <div className="kanban-card__meta">
                      <span className="kanban-card__id">{card.id}</span>
                      <span className={priorityClass(card.priority)}>
                        {card.priority}
                      </span>
                      <span className="lane">{card.lane}:</span>
                    </div>
                    <h3 className="kanban-card__title">{card.title}</h3>
                    {card.next ? (
                      <p className="kanban-card__next">Next: {card.next}</p>
                    ) : null}
                    {card.blocker ? (
                      <p className="kanban-card__blocker">
                        Blocker: {card.blocker}
                      </p>
                    ) : null}
                    {card.note ? (
                      <p className="kanban-card__note">{card.note}</p>
                    ) : null}
                    <div className="kanban-card__moves">
                      {COLUMNS.filter((c) => c.id !== card.column).map((c) => (
                        <button
                          key={c.id}
                          type="button"
                          className="btn btn--tiny"
                          onClick={() => onMove(card.id, c.id)}
                          title={`Move to ${c.title}`}
                        >
                          → {c.title}
                        </button>
                      ))}
                    </div>
                  </article>
                ))}
              </div>
            </div>
          );
        })}
      </div>
    </section>
  );
}