import type { SectionId } from "./Sidebar.types";

type SidebarProps = {
  active: SectionId;
  onSelect: (id: SectionId) => void;
};

const ITEMS: { id: SectionId; label: string; hint: string }[] = [
  { id: "overview", label: "Overview", hint: "Ops snapshot" },
  { id: "automations", label: "Automation", hint: "Daily brief + sync" },
  { id: "actions", label: "Actions", hint: "Triggers + task queue" },
  { id: "kanban", label: "Kanban", hint: "Solo WIP board" },
  { id: "tripwire", label: "Tripwire", hint: "$9 starter pack" },
  { id: "gates", label: "Launch gates", hint: "G1–G6" },
  { id: "vault", label: "Vault PRs", hint: "Merge plan" },
  { id: "vst", label: "VST fleet", hint: "CI on main" },
  { id: "agents", label: "Agent lanes", hint: "Slack prefixes" },
  { id: "mobile", label: "Mobile crew", hint: "Free CODE agent" },
];

export function Sidebar({ active, onSelect }: SidebarProps) {
  return (
    <nav className="sidebar" aria-label="Command center sections">
      <div className="sidebar__brand">
        <span className="sidebar__logo">DLZ</span>
        <div>
          <div className="sidebar__title">Command Center</div>
          <div className="sidebar__sub">DiskLordz business ops</div>
        </div>
      </div>
      <ul className="sidebar__list">
        {ITEMS.map((item) => (
          <li key={item.id}>
            <button
              type="button"
              className={
                active === item.id
                  ? "sidebar__item sidebar__item--active"
                  : "sidebar__item"
              }
              onClick={() => onSelect(item.id)}
            >
              <span className="sidebar__item-label">{item.label}</span>
              <span className="sidebar__item-hint">{item.hint}</span>
            </button>
          </li>
        ))}
      </ul>
    </nav>
  );
}