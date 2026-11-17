import type { OpsSnapshot } from "./opsSnapshot";

type PartialOps = Partial<
  Pick<
    OpsSnapshot,
    | "kanban"
    | "vaultPrs"
    | "launchGates"
    | "tripwireSteps"
    | "syncErrors"
    | "generatedAt"
    | "vstFleet"
  >
>;

export function mergeOpsSnapshot(
  base: OpsSnapshot,
  live: PartialOps,
): OpsSnapshot {
  return {
    ...base,
    ...live,
    kanban: live.kanban ?? base.kanban,
    vaultPrs: live.vaultPrs?.length ? live.vaultPrs : base.vaultPrs,
    launchGates: live.launchGates ?? base.launchGates,
    tripwireSteps: live.tripwireSteps ?? base.tripwireSteps,
    syncErrors: [...base.syncErrors, ...(live.syncErrors ?? [])],
    generatedAt: live.generatedAt ?? base.generatedAt,
    vstFleet: live.vstFleet ?? base.vstFleet,
  };
}