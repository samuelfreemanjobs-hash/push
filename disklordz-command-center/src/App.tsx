import { useState } from "react";
import { AgentsPanel } from "./components/AgentsPanel";
import { GatesPanel } from "./components/GatesPanel";
import { KanbanBoard } from "./components/KanbanBoard";
import { Overview } from "./components/Overview";
import { Sidebar } from "./components/Sidebar";
import type { SectionId } from "./components/Sidebar.types";
import { TripwirePanel } from "./components/TripwirePanel";
import { VaultPanel } from "./components/VaultPanel";
import { VstPanel } from "./components/VstPanel";
import { useKanbanStorage } from "./hooks/useKanbanStorage";

export default function App() {
  const [section, setSection] = useState<SectionId>("overview");
  const { cards, moveCard, resetKanban, wipLimit } = useKanbanStorage();

  return (
    <div className="app-shell">
      <Sidebar active={section} onSelect={setSection} />
      <main className="main">
        {section === "overview" ? <Overview /> : null}
        {section === "kanban" ? (
          <KanbanBoard
            cards={cards}
            wipLimit={wipLimit}
            onMove={moveCard}
            onReset={resetKanban}
          />
        ) : null}
        {section === "tripwire" ? <TripwirePanel /> : null}
        {section === "gates" ? <GatesPanel /> : null}
        {section === "vault" ? <VaultPanel /> : null}
        {section === "vst" ? <VstPanel /> : null}
        {section === "agents" ? <AgentsPanel /> : null}
      </main>
    </div>
  );
}