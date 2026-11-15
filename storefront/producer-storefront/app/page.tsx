"use client";

import React, { useState } from "react";
import InteractiveKitPreviewer from "@/components/InteractiveKitPreviewer";

export default function StorefrontLandingPage() {
  const [email, setEmail] = useState("");
  const [freeKitSent, setFreeKitSent] = useState(false);

  const [submitting, setSubmitting] = useState(false);
  const [leadError, setLeadError] = useState<string | null>(null);

  const handleLeadMagnet = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!email.trim()) return;
    setSubmitting(true);
    setLeadError(null);
    try {
      const res = await fetch("/api/lead-magnet", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ email: email.trim() }),
      });
      if (!res.ok) throw new Error("submit_failed");
      setFreeKitSent(true);
    } catch {
      setLeadError("Could not submit — try again or email support.");
    } finally {
      setSubmitting(false);
    }
  };

  return (
    <div className="min-h-screen bg-[#09090b] font-sans text-zinc-100 selection:bg-[#ff1e56] selection:text-white">
      <div className="bg-gradient-to-r from-[#ff1e56] via-[#7f5af0] to-[#ff1e56] px-4 py-2 text-center text-xs font-bold uppercase tracking-wider text-white">
        100% Royalty-Free Commercial License • Keep 100% of Your BeatStars &amp; Label Placements
      </div>

      <header className="mx-auto max-w-6xl px-6 pb-20 pt-14 text-center">
        <span className="mb-5 inline-block rounded-full border border-zinc-800 bg-zinc-900 px-3 py-1 text-xs font-semibold uppercase tracking-wide text-[#ffac41]">
          VST3 / AU + 24-Bit WAV &amp; MIDI • Windows &amp; macOS
        </span>

        <h1 className="mx-auto max-w-4xl text-4xl font-extrabold leading-tight tracking-tight md:text-6xl">
          Infinite Dark Trap &amp; Memphis Phonk Stems —{" "}
          <span className="bg-gradient-to-r from-[#ff1e56] to-[#ffac41] bg-clip-text text-transparent">
            Generated Locally Inside Your DAW.
          </span>
        </h1>

        <p className="mx-auto mt-5 max-w-2xl text-base leading-relaxed text-zinc-400 md:text-lg">
          Stop digging through recycled sample packs. Smash the pads below to hear our GPU-accelerated VST3 engine
          paired with analog soft-clipped 808s, distorted cowbells, and dark trap bells.
        </p>

        <div className="mt-10 flex justify-center">
          <InteractiveKitPreviewer />
        </div>

        <div className="mt-8 flex flex-wrap justify-center gap-4">
          <a
            href="#pricing"
            className="rounded-xl bg-[#ff1e56] px-8 py-4 text-base font-bold text-white shadow-lg shadow-[#ff1e56]/25 transition hover:bg-[#e0174a]"
          >
            Get The Producer All-Access Bundle ($69)
          </a>
          <a
            href="#free-kit"
            className="rounded-xl border border-zinc-700 bg-zinc-900 px-8 py-4 text-base font-semibold text-zinc-200 transition hover:bg-zinc-800"
          >
            Download 25 Free Phonk &amp; Trap Sounds
          </a>
        </div>
      </header>

      <section className="border-y border-zinc-900 bg-zinc-950/60 px-6 py-20">
        <div className="mx-auto max-w-6xl">
          <div className="mx-auto mb-14 max-w-2xl text-center">
            <h2 className="text-3xl font-bold">Built For Producers Who Need Placements Fast</h2>
            <p className="mt-3 text-zinc-400">Every sound is pre-clipped, pitch-tagged, and cleared for commercial release.</p>
          </div>

          <div className="grid gap-8 md:grid-cols-3">
            <FeatureCard
              tag="01 // LOCAL NEURAL VST3"
              tagColor="#ff1e56"
              title="Zero Cloud Lag. Zero Subscriptions."
              body="Runs directly on your machine inside FL Studio, Ableton, or MPC. Generate fresh Memphis cowbell melodies and dark trap loops in seconds with one click."
            />
            <FeatureCard
              tag="02 // ANALOG SOFT-CLIPPED DSP"
              tagColor="#ffac41"
              title="Mix-Ready Out The Box"
              body="No weak, thin samples. Every 808, kick, and snare is processed through tape saturation and tanh soft-clipping so your drums knock straight through the mix."
            />
            <FeatureCard
              tag="03 // 100% ROYALTY-FREE"
              tagColor="#7f5af0"
              title="Zero Clearance Headaches"
              body="Trained and synthesized strictly on commercial-safe open weights and custom DSP. Upload to Spotify, BeatStars, and YouTube with zero copyright strikes."
            />
          </div>
        </div>
      </section>

      <section id="pricing" className="mx-auto max-w-6xl px-6 py-24">
        <div className="mx-auto mb-16 max-w-2xl text-center">
          <h2 className="text-3xl font-bold md:text-4xl">Choose Your Sound Arsenal</h2>
          <p className="mt-3 text-zinc-400">Instant digital download. Lifetime updates included.</p>
        </div>

        <div className="grid items-center gap-8 md:grid-cols-3">
          <PricingTier
            name="Underground Drum & Loop Kit"
            price="$29"
            subtitle="Essential blown-out 808s, Memphis cowbells, and MIDI patterns."
            items={[
              "250+ Pitch-Tagged 24-Bit WAV One-Shots",
              "50 BPM-Locked Phonk & Trap Loops",
              "40 Hi-Hat & 808 MIDI Patterns",
              "100% Royalty-Free License",
            ]}
            cta="Buy Drum Kit ($29)"
            highlight={false}
          />

          <PricingTier
            name="Producer All-Access Bundle"
            price="$69"
            strike="$154"
            badge="Most Popular • Save 55%"
            subtitle="The complete VST3 neural engine plus our entire Trap & Phonk catalog."
            items={[
              { text: "Neural Phonk & Trap VST3 / AU Plugin", bold: true },
              { text: "450+ WAVs & 120+ Construction Stems", bold: true },
              { text: "150 Factory Synth & 808 VST Presets", bold: false },
              { text: "80 Trap & Drift Phonk MIDI Files", bold: false },
              { text: "Free Monthly Expansion Drops", bold: false },
            ]}
            cta="Get Instant Access ($69)"
            highlight
          />

          <PricingTier
            name="Neural VST3 Engine Only"
            price="$49"
            subtitle="Generate unlimited royalty-free stems directly inside your DAW."
            items={[
              "VST3 & AU Formats (Win / Mac)",
              "Local GPU Inference (No Internet Needed)",
              "Built-In Tape Grit & Soft-Clipper DSP",
              "75 Starter Presets Included",
            ]}
            cta="Buy VST3 Plugin ($49)"
            highlight={false}
          />
        </div>
      </section>

      <section id="free-kit" className="mx-auto max-w-4xl px-6 pb-24">
        <div className="rounded-3xl border border-zinc-800 bg-zinc-900 p-8 text-center md:p-12">
          <h2 className="text-2xl font-bold md:text-3xl">Test Our Sounds In Your Next Beat — Free</h2>
          <p className="mx-auto mt-3 max-w-xl text-sm text-zinc-400 md:text-base">
            Grab our <strong className="text-zinc-200">2026 Underground Phonk &amp; Trap Starter Pack</strong> (10 blown-out
            808s, 5 Memphis cowbell loops, and 10 MIDI files). Zero strings attached.
          </p>

          {freeKitSent ? (
            <p className="mt-8 text-lg font-medium text-[#ffac41]">
              Check your inbox! Your free starter kit download link is on the way.
            </p>
          ) : (
            <form onSubmit={(e) => void handleLeadMagnet(e)} className="mx-auto mt-8 flex max-w-md flex-col gap-3 sm:flex-row">
              <input
                type="email"
                required
                placeholder="Enter your producer email..."
                value={email}
                onChange={(e) => setEmail(e.target.value)}
                className="flex-1 rounded-xl border border-zinc-700 bg-black px-4 py-3.5 text-white placeholder-zinc-500 focus:border-[#ff1e56] focus:outline-none"
              />
              <button
                type="submit"
                disabled={submitting}
                className="rounded-xl bg-[#7f5af0] px-6 py-3.5 font-bold text-white transition hover:bg-[#6c4bd4] disabled:opacity-60"
              >
                {submitting ? "Sending…" : "Send Free Kit"}
              </button>
              {leadError ? <p className="text-sm text-[#ff1e56] sm:basis-full">{leadError}</p> : null}
            </form>
          )}
        </div>
      </section>

      <footer className="border-t border-zinc-900 px-6 py-10 text-center text-sm text-zinc-600">
        <a href="../interactive-lab/index.html" className="text-zinc-500 hover:text-zinc-300">
          Static beat-pad lab
        </a>
        {" · "}
        <a href="../index.html" className="text-zinc-500 hover:text-zinc-300">
          Legacy storefront
        </a>
      </footer>
    </div>
  );
}

function FeatureCard({
  tag,
  tagColor,
  title,
  body,
}: {
  tag: string;
  tagColor: string;
  title: string;
  body: string;
}) {
  return (
    <div className="rounded-2xl border border-zinc-800 bg-zinc-900/70 p-6">
      <div className="mb-2 font-mono text-sm font-bold" style={{ color: tagColor }}>
        {tag}
      </div>
      <h3 className="mb-2 text-xl font-bold">{title}</h3>
      <p className="text-sm leading-relaxed text-zinc-400">{body}</p>
    </div>
  );
}

function PricingTier({
  name,
  price,
  strike,
  badge,
  subtitle,
  items,
  cta,
  highlight,
}: {
  name: string;
  price: string;
  strike?: string;
  badge?: string;
  subtitle: string;
  items: Array<string | { text: string; bold: boolean }>;
  cta: string;
  highlight: boolean;
}) {
  const shell = highlight
    ? "relative rounded-2xl border-2 border-[#ff1e56] bg-gradient-to-b from-zinc-900 to-[#14101c] p-8 shadow-2xl shadow-[#ff1e56]/15"
    : "rounded-2xl border border-zinc-800 bg-zinc-900/50 p-8";

  const buttonClass = highlight
    ? "mt-8 w-full rounded-xl bg-[#ff1e56] py-4 text-base font-bold text-white shadow-lg shadow-[#ff1e56]/30 transition hover:bg-[#e0174a]"
    : "mt-8 w-full rounded-xl bg-zinc-800 py-3 text-sm font-semibold transition hover:bg-zinc-700";

  return (
    <div className={shell}>
      {badge ? (
        <span className="absolute -top-3.5 left-1/2 -translate-x-1/2 rounded-full bg-[#ff1e56] px-3 py-1 text-xs font-extrabold uppercase tracking-wider text-white">
          {badge}
        </span>
      ) : null}
      <h3 className={`text-lg font-bold ${highlight ? "text-white" : "text-zinc-300"}`}>{name}</h3>
      <div className="mt-4 flex items-baseline gap-2">
        <span className={`font-extrabold ${highlight ? "text-5xl text-white" : "text-4xl"}`}>{price}</span>
        {strike ? <span className="text-lg text-zinc-500 line-through">{strike}</span> : null}
        {!strike && !highlight ? <span className="text-sm text-zinc-500">one-time</span> : null}
      </div>
      <p className={`mt-3 text-sm ${highlight ? "text-zinc-300" : "text-zinc-400"}`}>{subtitle}</p>
      <ul className={`mt-6 space-y-3 text-sm ${highlight ? "text-zinc-200" : "text-zinc-300"}`}>
        {items.map((item) => {
          const label = typeof item === "string" ? item : item.text;
          const key = label;
          const content =
            typeof item === "string" ? (
              label
            ) : item.bold ? (
              <strong className="text-white">{label}</strong>
            ) : (
              label
            );
          return (
            <li key={key}>
              ✓ {content}
            </li>
          );
        })}
      </ul>
      <button type="button" className={buttonClass}>
        {cta}
      </button>
    </div>
  );
}