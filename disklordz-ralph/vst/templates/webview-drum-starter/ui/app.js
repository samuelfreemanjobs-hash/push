document.getElementById("kickBtn")?.addEventListener("click", () => {
  if (window.__JUCE__?.backend?.emitEvent) {
    window.__JUCE__.backend.emitEvent("triggerKick", {});
  }
});