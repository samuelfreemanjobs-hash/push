import { useEffect } from "react";
import { ActionConsolePanel } from "./components/ActionConsolePanel";
import { AgentTasksPanel } from "./components/AgentTasksPanel";
import { AgentsPanel } from "./components/AgentsPanel";
import { AutomationsPanel } from "./components/AutomationsPanel";
import { ConnectionsPanel } from "./components/ConnectionsPanel";
import { GatesPanel } from "./components/GatesPanel";
import { KanbanBoard } from "./components/KanbanBoard";
import { MobileCrewPanel } from "./components/MobileCrewPanel";
import { Overview } from "./components/Overview";
import { Sidebar } from "./components/Sidebar";
import { ToastBar } from "./components/ToastBar";
import { TripwirePanel } from "./components/TripwirePanel";
import { VaultPanel } from "./components/VaultPanel";
import { VstPanel } from "./components/VstPanel";
import {
  CommandCenterProvider,
  useCommandCenter,
} from "./context/CommandCenterContext";
import { ConnectionsProvider } from "./context/ConnectionsContext";
import { OpsDataProvider } from "./context/OpsDataContext";
import type { KanbanColumnId } from "./data/types";
import { useKanbanStorage } from "./hooks/useKanbanStorage";

function CommandCenterMain() {
  const {
    section,
    setSection,
    kanbanMoveRequest,
    clearKanbanMoveRequest,
  } = useCommandCenter();
  const { cards, moveCard, resetKanban, wipLimit } = useKanbanStorage();

  useEffect(() => {
    if (!kanbanMoveRequest) return;
    moveCard(
      kanbanMoveRequest.cardId,
      kanbanMoveRequest.column as KanbanColumnId,
    );
    clearKanbanMoveRequest();
  }, [kanbanMoveRequest, moveCard, clearKanbanMoveRequest]);

  return (
    <div className="app-shell">
      <Sidebar active={section} onSelect={setSection} />
      <main className="main">
        <ToastBar />
        {section === "overview" ? <Overview /> : null}
        {section === "automations" ? <AutomationsPanel /> : null}
        {section === "connections" ? <ConnectionsPanel /> : null}
        {section === "actions" ? <ActionConsolePanel /> : null}
        {section === "tasks" ? <AgentTasksPanel /> : null}
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
        {section === "mobile" ? <MobileCrewPanel /> : null}
      </main>
    </div>
  );
}

export default function App() {
  return (
    <OpsDataProvider>
      <ConnectionsProvider>
        <CommandCenterProvider>
          <CommandCenterMain />
        </CommandCenterProvider>
      </ConnectionsProvider>
    </OpsDataProvider>
  );
}