# REA prompt — multi-out bus layout (DLZ-VST-009)

**Scope:** `isBusesLayoutSupported`, `processBlock` bus routing, strings `Kick` / `Sub` / `Hats`.

**Target:** RelWithDebInfo `.vst3` from `rea-targets.json`.

**Ask:**

1. List output bus count and channel sets exported to host.
2. Trace whether per-bus buffers use preallocated scratch (no malloc in `processBlock`).
3. Compare to `starter-cpp/TrapMultiOutLayout.h` — observation / inference / unknown.

**FL Studio:** note PDC differences when enabling optional outputs.

Save findings as `multiout-layout-latest.md`.