/**
 * Interactive beat pad — Web Audio + bass-reactive backdrop.
 * Swap demo URLs to CDN after kit_mastering / dist_sample_pack export.
 * vgpu upgrade: replace #bassPulse CSS with @vercel/vgpu when deployed on Vercel.
 */

export const KIT_PADS = [
  { id: "808", label: "Blown-out 808 (F)", src: "../disklordz-lite/demos/808.wav", color: "#ff1e56", key: "1" },
  { id: "cowbell", label: "Memphis cowbell 145", src: "../disklordz-lite/demos/hat.wav", color: "#ffac41", key: "2" },
  { id: "trap_bell", label: "Dark trap bell", src: "../disklordz-lite/demos/snare.wav", color: "#7f5af0", key: "3" },
  { id: "snare", label: "Crunchy phonk snare", src: "../disklordz-lite/demos/snare.wav", color: "#2cb67d", key: "4" },
];

let audioCtx;
let analyser;
let bassEnergy = 0;

const pulseEl = document.getElementById("bassPulse");
const grid = document.getElementById("padGrid");

function ensureAudio() {
  if (audioCtx) return;
  audioCtx = new AudioContext();
  analyser = audioCtx.createAnalyser();
  analyser.fftSize = 256;
  analyser.connect(audioCtx.destination);
  requestAnimationFrame(drawBassPulse);
}

async function triggerSample(url) {
  ensureAudio();
  if (audioCtx.state === "suspended") await audioCtx.resume();
  try {
    const res = await fetch(url);
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    const buf = await audioCtx.decodeAudioData(await res.arrayBuffer());
    const source = audioCtx.createBufferSource();
    source.buffer = buf;
    const g = audioCtx.createGain();
    g.gain.value = 0.95;
    source.connect(g);
    g.connect(analyser);
    source.start(0);
  } catch (err) {
    console.warn("Demo missing — drop WAV under storefront/disklordz-lite/demos/", err);
  }
}

function drawBassPulse() {
  if (analyser) {
    const data = new Uint8Array(analyser.frequencyBinCount);
    analyser.getByteFrequencyData(data);
    let sum = 0;
    for (let i = 1; i < 12; i++) sum += data[i];
    bassEnergy = sum / (12 * 255);
    const scale = 1 + bassEnergy * 0.12;
    pulseEl.style.transform = `scale(${scale})`;
    pulseEl.style.opacity = String(0.25 + bassEnergy * 0.5);
  }
  requestAnimationFrame(drawBassPulse);
}

function renderPads() {
  KIT_PADS.forEach((pad) => {
    const btn = document.createElement("button");
    btn.type = "button";
    btn.className = "pad-btn";
    btn.style.background = `linear-gradient(160deg, ${pad.color}33, #14141a)`;
    btn.style.borderColor = pad.color;
    btn.textContent = pad.label;
    btn.addEventListener("click", () => triggerSample(pad.src));
    grid.appendChild(btn);
  });
}

window.addEventListener("keydown", (e) => {
  const pad = KIT_PADS.find((p) => p.key === e.key);
  if (pad) triggerSample(pad.src);
});

renderPads();