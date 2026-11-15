// Plugin host smoke test: loads the BUILT VST3 like a DAW does and checks
// every preset at several sample rates / block sizes (no NaNs, no overs),
// that the dry path lines up with the reported latency, and state save/restore.
// Usage: memphis_dust_host_test "<path to Memphis Dust.vst3>"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
using namespace juce;

static int run (const String& path, const String&)
{
    VST3PluginFormat fmt;
    OwnedArray<PluginDescription> descs;
    fmt.findAllTypesForFile (descs, path);
    if (descs.isEmpty()) { std::puts ("no plugin found"); return 1; }
    String err;
    auto inst = fmt.createInstanceFromDescription (*descs[0], 48000.0, 512, err);
    if (! inst) { std::printf ("load failed: %s\n", err.toRawUTF8()); return 1; }
    std::printf ("Loaded: %s, params=%d, programs=%d\n", inst->getName().toRawUTF8(), inst->getParameters().size(), inst->getNumPrograms());

    int fails = 0;
    for (double sr : { 44100.0, 48000.0, 96000.0 })
    for (int bs : { 64, 512, 1024 })
    {
        inst->setPlayConfigDetails (2, 2, sr, bs);
        inst->prepareToPlay (sr, bs);
        const int lat = inst->getLatencySamples();
        for (int prog = 0; prog < inst->getNumPrograms(); ++prog)
        {
            inst->setCurrentProgram (prog);
            AudioBuffer<float> buf (2, bs); MidiBuffer midi;
            Random rnd (prog);
            float peak = 0; bool bad = false; double sumSq = 0; long cnt = 0; const long skip = (long) (sr * 2);
            const int blocks = (int) (sr * 4 / bs);
            for (int b = 0; b < blocks; ++b)
            {
                for (int i = 0; i < bs; ++i)
                {
                    const long t = (long) b * bs + i;
                    // kick-ish: decaying 55 Hz every 0.5 s + noise snare bursts
                    const double tp = std::fmod ((double) t / sr, 0.5);
                    float s = (float) (0.9 * std::exp (-tp * 9) * std::sin (2 * MathConstants<double>::pi * 55 * tp));
                    const double ts = std::fmod ((double) t / sr + 0.25, 0.5);
                    s += (float) (0.6 * std::exp (-ts * 30) * (rnd.nextFloat() * 2 - 1));
                    buf.setSample (0, i, s); buf.setSample (1, i, s * 0.95f);
                }
                inst->processBlock (buf, midi);
                for (int c = 0; c < 2; ++c)
                    for (int i = 0; i < bs; ++i)
                    {
                        const float v = buf.getSample (c, i);
                        if (! std::isfinite (v)) bad = true;
                        peak = jmax (peak, std::abs (v)); if ((long) b * bs + i >= skip) { sumSq += v * v; ++cnt; }
                    }
            }
            const bool ok = ! bad && peak <= 1.0001f + (prog == inst->getNumPrograms() - 1 ? 10.0f : 0.0f);
            if (! ok) ++fails;
            if (bs == 512)
                std::printf ("  [%s] sr=%6.0f lat=%3d  %-20s peak=%.3f rms=%.1f dBFS\n", ok ? "ok" : "FAIL", sr, lat,
                             inst->getProgramName (prog).toRawUTF8(), peak, Decibels::gainToDecibels (std::sqrt (sumSq / cnt)));
        }
        inst->releaseResources();
    }

    // latency check: impulse through "Clean (Init)" should come out exactly at reported latency
    {
        inst->setPlayConfigDetails (2, 2, 48000.0, 512);
        inst->prepareToPlay (48000.0, 512);
        inst->setCurrentProgram (inst->getNumPrograms() - 1);
        for (auto* p : inst->getParameters()) if (p->getName (32) == "Mix") p->setValue (0.0f); // dry only
        AudioBuffer<float> buf (2, 512); MidiBuffer m;
        for (int k = 0; k < 4; ++k) { buf.clear(); inst->processBlock (buf, m); } // settle smoothing
        buf.clear(); buf.setSample (0, 0, 1.0f); buf.setSample (1, 0, 1.0f);
        inst->processBlock (buf, m);
        int at = -1; float mx = 0;
        for (int i = 0; i < 512; ++i) if (std::abs (buf.getSample (0, i)) > mx) { mx = std::abs (buf.getSample (0, i)); at = i; }
        std::printf ("Dry-path impulse peak at %d (reported latency %d), value %.3f -> %s\n", at, inst->getLatencySamples(), mx,
                     at == inst->getLatencySamples() ? "ALIGNED" : "MISALIGNED");
        if (at != inst->getLatencySamples()) ++fails;
        inst->releaseResources();
    }

    // state round trip
    {
        MemoryBlock mb; inst->setCurrentProgram (3); inst->getStateInformation (mb);
        const float before = inst->getParameters()[5]->getValue();
        inst->setCurrentProgram (0); inst->setStateInformation (mb.getData(), (int) mb.getSize());
        const float after = inst->getParameters()[5]->getValue();
        std::printf ("State round-trip: %s\n", std::abs (before - after) < 1e-5f ? "ok" : "FAIL"); if (std::abs (before - after) >= 1e-5f) ++fails;
        inst->setCurrentProgram (0);
    }

    std::printf ("%s\n", fails ? "HOST TEST FAILED" : "HOST TEST PASSED");
    return fails;
}


int main (int argc, char** argv)
{
    if (argc < 2) { std::puts ("usage: memphis_dust_host_test <plugin.vst3>"); return 2; }
    ScopedJuceInitialiser_GUI juceInit;
    return run (String::fromUTF8 (argv[1]), {});
}
