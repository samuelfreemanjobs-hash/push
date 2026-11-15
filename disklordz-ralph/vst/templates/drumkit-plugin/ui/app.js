/**
 * WebView2 native integration — IDs must match C++ WebSliderRelay names.
 * Serena: index ui/*.js with tsserver; use find_referencing_symbols when renaming APVTS ids.
 */
export const CONTROL_IDS = {
  outputGain: "outputGain",
};

function bindRange(id) {
  const el = document.getElementById(id);
  if (!el) return;
  el.addEventListener("input", () => {
    // JUCE WebSliderRelay reads element state via native integration when id matches relay name.
    console.debug(`[WebView] ${id} = ${el.value}`);
  });
}

bindRange(CONTROL_IDS.outputGain);