"use client";

import React, { useCallback, useEffect, useRef, useState } from "react";

export type KitPad = {
  id: string;
  label: string;
  src: string;
  color: string;
  key: string;
};

export const KIT_PADS: KitPad[] = [
  { id: "808", label: "BLOWN-OUT 808 (F)", src: "/demos/808.wav", color: "#ff1e56", key: "1" },
  { id: "cowbell", label: "MEMPHIS COWBELL 145", src: "/demos/cowbell.wav", color: "#ffac41", key: "2" },
  { id: "trap_bell", label: "DARK TRAP BELL", src: "/demos/trap_bell.wav", color: "#7f5af0", key: "3" },
  { id: "snare", label: "CRUNCHY PHONK SNARE", src: "/demos/snare.wav", color: "#2cb67d", key: "4" },
];

type BuyBundleDrawerProps = {
  headline: string;
  offerText: string;
  ctaLabel: string;
  checkoutUrl: string;
};

function BuyBundleDrawer({ headline, offerText, ctaLabel, checkoutUrl }: BuyBundleDrawerProps) {
  return (
    <aside className="rounded-xl border border-zinc-800 bg-black/50 p-4">
      <h2 className="mb-1 font-semibold text-zinc-100">{headline}</h2>
      <p className="mb-3 text-sm text-zinc-400">{offerText}</p>
      <a
        href={checkoutUrl}
        className="inline-block rounded-lg bg-[#ff1e56] px-4 py-2.5 text-sm font-bold text-white shadow-lg shadow-[#ff1e56]/25 transition hover:bg-[#e0174a]"
      >
        {ctaLabel}
      </a>
    </aside>
  );
}

export default function InteractiveKitPreviewer() {
  const audioCtxRef = useRef<AudioContext | null>(null);
  const analyserRef = useRef<AnalyserNode | null>(null);
  const pulseRef = useRef<HTMLDivElement | null>(null);
  const rafRef = useRef<number | null>(null);
  const [bpm] = useState(145);
  const [drawerOpen, setDrawerOpen] = useState(false);

  const drawBassPulse = useCallback(() => {
    const analyser = analyserRef.current;
    const pulse = pulseRef.current;
    if (analyser && pulse) {
      const data = new Uint8Array(analyser.frequencyBinCount);
      analyser.getByteFrequencyData(data);
      let sum = 0;
      for (let i = 1; i < 12; i++) sum += data[i];
      const bassEnergy = sum / (12 * 255);
      const scale = 1 + bassEnergy * 0.14;
      pulse.style.transform = `scale(${scale})`;
      pulse.style.opacity = String(0.22 + bassEnergy * 0.55);
    }
    rafRef.current = requestAnimationFrame(drawBassPulse);
  }, []);

  const ensureAudio = useCallback(() => {
    if (audioCtxRef.current) return;
    const ctx = new AudioContext();
    const node = ctx.createAnalyser();
    node.fftSize = 256;
    node.connect(ctx.destination);
    audioCtxRef.current = ctx;
    analyserRef.current = node;
    rafRef.current = requestAnimationFrame(drawBassPulse);
  }, [drawBassPulse]);

  const triggerSample = useCallback(
    async (url: string) => {
      ensureAudio();
      const ctx = audioCtxRef.current;
      const analyser = analyserRef.current;
      if (!ctx || !analyser) return;
      if (ctx.state === "suspended") await ctx.resume();
      try {
        const res = await fetch(url);
        if (!res.ok) throw new Error(`HTTP ${res.status}`);
        const buf = await ctx.decodeAudioData(await res.arrayBuffer());
        const source = ctx.createBufferSource();
        source.buffer = buf;
        const gain = ctx.createGain();
        gain.gain.value = 0.95;
        source.connect(gain);
        gain.connect(analyser);
        source.start(0);
      } catch {
        console.warn("Demo WAV missing — add files under public/demos/");
      }
    },
    [ensureAudio],
  );

  useEffect(() => {
    const onKeyDown = (e: KeyboardEvent) => {
      const pad = KIT_PADS.find((p) => p.key === e.key);
      if (pad) void triggerSample(pad.src);
    };
    window.addEventListener("keydown", onKeyDown);
    return () => {
      window.removeEventListener("keydown", onKeyDown);
      if (rafRef.current !== null) cancelAnimationFrame(rafRef.current);
    };
  }, [triggerSample]);

  return (
    <div className="relative w-full max-w-3xl overflow-hidden rounded-2xl border border-zinc-800 bg-black p-6">
      <div
        ref={pulseRef}
        aria-hidden
        className="bass-pulse-glow pointer-events-none absolute inset-0 opacity-40"
      />

      <div className="relative z-10 space-y-6">
        <header className="text-left">
          <p className="text-xs font-semibold uppercase tracking-widest text-[#ffac41]">Live tester</p>
          <h2 className="text-lg font-bold text-white sm:text-xl">
            2026 Underground Phonk &amp; Trap Engine — Live Tester
          </h2>
          <p className="text-sm text-zinc-500">
            BPM <span className="text-zinc-300">{bpm}</span> · keys 1–4
          </p>
        </header>

        <div className="grid grid-cols-2 gap-3 sm:grid-cols-4">
          {KIT_PADS.map((pad) => (
            <button
              key={pad.id}
              type="button"
              className="pad-btn min-h-[5.5rem] rounded-xl border px-2 py-3 text-xs font-bold uppercase leading-tight text-zinc-100 sm:text-[11px]"
              style={{
                background: `linear-gradient(160deg, ${pad.color}33, #14141a)`,
                borderColor: pad.color,
              }}
              onClick={() => void triggerSample(pad.src)}
            >
              {pad.label}
            </button>
          ))}
        </div>

        <div className="flex items-center justify-between gap-2 border-t border-zinc-800 pt-4">
          <button
            type="button"
            className="text-sm text-zinc-400 underline-offset-2 hover:text-zinc-200 hover:underline"
            onClick={() => setDrawerOpen((v) => !v)}
          >
            {drawerOpen ? "Hide bundle offer" : "Show bundle offer"}
          </button>
          <span className="text-xs text-zinc-600">WebGPU shader: swap in @vercel/vgpu on deploy</span>
        </div>

        {drawerOpen ? (
          <BuyBundleDrawer
            headline="Like how hard those 808s hit?"
            offerText="Get the Neural VST3 Plugin + 450 Royalty-Free WAVs & MIDIs ($69)"
            ctaLabel="Unlock Full Producer Bundle"
            checkoutUrl="#pricing"
          />
        ) : null}
      </div>
    </div>
  );
}