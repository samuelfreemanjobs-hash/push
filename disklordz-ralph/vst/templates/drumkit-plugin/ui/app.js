/**
 * WebView2 native integration — IDs must match C++ WebSliderRelay names.
 */
export const CONTROL_IDS = {
  outputGain: "outputGain",
};

const NEURAL_EVENT = "disklordz:neuralGenerate";

function bindRange(id) {
  const el = document.getElementById(id);
  if (!el) return;
  el.addEventListener("input", () => {
    console.debug(`[WebView] ${id} = ${el.value}`);
  });
}

function bindNeuralPanel() {
  const prompt = document.getElementById("neuralPrompt");
  const bpm = document.getElementById("neuralBpm");
  const bpmLabel = document.getElementById("neuralBpmLabel");
  const btn = document.getElementById("neuralGenerate");
  const status = document.getElementById("neuralStatus");
  if (!prompt || !bpm || !btn || !status) return;

  bpm.addEventListener("input", () => {
    if (bpmLabel) bpmLabel.textContent = bpm.value;
  });

  btn.addEventListener("click", () => {
    const detail = {
      prompt: prompt.value.trim() || "dark drift phonk cowbell loop",
      bpm: Number.parseInt(bpm.value, 10) || 145,
    };
    status.textContent = `Queued: ${detail.prompt} @ ${detail.bpm} BPM`;
    window.dispatchEvent(new CustomEvent(NEURAL_EVENT, { detail }));
    if (window.chrome?.webview?.postMessage) {
      window.chrome.webview.postMessage({ type: "neuralGenerate", ...detail });
    }
    console.info("[NeuralPanel]", detail);
  });
}

bindRange(CONTROL_IDS.outputGain);
bindNeuralPanel();