import type {
  AgentLane,
  KanbanCard,
  LaunchGate,
  RepoLink,
  TripwireStep,
  VaultPr,
  VstSku,
} from "./types";
import {
  AGENT_LANES,
  KANBAN_CARDS,
  KANBAN_WIP,
  LAUNCH_GATES,
  NORTH_STAR,
  REPOS,
  TRIPWIRE_STEPS,
  VAULT_PRS,
  VST_FLEET,
} from "./seed";

export type OpsSnapshot = {
  generatedAt: string;
  sources: {
    kanban: string;
    businessOs: string;
    instrumentsMain: string;
  };
  syncErrors: string[];
  northStar: typeof NORTH_STAR;
  kanban: { wipLimit: number; cards: KanbanCard[] };
  launchGates: LaunchGate[];
  tripwireSteps: TripwireStep[];
  vaultPrs: VaultPr[];
  vstFleet: VstSku[];
};

export const FALLBACK_SNAPSHOT: OpsSnapshot = {
  generatedAt: "1970-01-01T00:00:00.000Z",
  sources: {
    kanban:
      "https://github.com/samuelfreemanjobs-hash/Revenue-Systems-Engineer-Master-Vault/blob/main/agents/disklordz/kanban.yaml",
    businessOs:
      "https://github.com/samuelfreemanjobs-hash/Revenue-Systems-Engineer-Master-Vault/blob/main/agents/disklordz/business-os.yaml",
    instrumentsMain: "https://github.com/samuelfreemanjobs-hash/Instruments/tree/main",
  },
  syncErrors: [],
  northStar: NORTH_STAR,
  kanban: { wipLimit: KANBAN_WIP, cards: KANBAN_CARDS },
  launchGates: LAUNCH_GATES,
  tripwireSteps: TRIPWIRE_STEPS,
  vaultPrs: VAULT_PRS,
  vstFleet: VST_FLEET,
};

export const STATIC_REPOS: RepoLink[] = REPOS;
export const STATIC_AGENT_LANES: AgentLane[] = AGENT_LANES;