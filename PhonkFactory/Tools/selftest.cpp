// Headless self-test for the plugin processor (run under xvfb-run so the editor can be snapshotted).
// usage: PhonkSelfTest [editor_snapshot.png]
#include "PluginProcessor.h"
#include <cstdio>

static int failures = 0;
#define CHECK(cond, msg) do { if (cond) std::printf ("ok    %s\n", msg); else { std::printf ("FAIL  %s\n", msg); ++failures; } } while (0)

struct TestPlayHead : juce::AudioPlayHead
{
    juce::Optional<PositionInfo> getPosition() const override
    {
        PositionInfo i; i.setPpqPosition (ppq); i.setIsPlaying (playing); i.setBpm (bpm); return i;
    }
    double ppq = 0, bpm = 120; bool playing = true;
};

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    PhonkProcessor p;
    p.setPlayConfigDetails (0, 2, 44100, 512);
    auto pump = [] (int ms) { juce::MessageManager::getInstance()->runDispatchLoopUntil (ms); };
    auto setp = [&] (const char* id, float v) { auto* q = p.apvts.getParameter (id); q->setValueNotifyingHost (q->convertTo0to1 (v)); };
    auto prepare = [&] (int block) { p.prepareToPlay (44100, block); pump (700); };

    // run `samples` samples through the processor in `block`-sized chunks; ph advances like a host would
    auto render = [&] (int samples, int block, TestPlayHead* ph = nullptr)
    {
        std::vector<float> out;
        juce::AudioBuffer<float> buf (2, block);
        for (int done = 0; done < samples; done += block)
        {
            juce::MidiBuffer midi;
            if (ph) ph->ppq = ((double) done / 44100.0) * ph->bpm / 60.0;
            p.processBlock (buf, midi);
            out.insert (out.end(), buf.getReadPointer (0), buf.getReadPointer (0) + block);
        }
        return out;
    };
    auto peak = [] (const std::vector<float>& v, size_t a = 0, size_t b = (size_t) -1)
    { float m = 0; for (size_t i = a; i < std::min (b, v.size()); ++i) m = std::max (m, std::abs (v[i])); return m; };
    auto firstAbove = [] (const std::vector<float>& v, float th, size_t from = 0)
    { for (size_t i = from; i < v.size(); ++i) if (std::abs (v[i]) > th) return (long) i; return -1L; };
    auto finite = [] (const std::vector<float>& v) { for (float s : v) if (! std::isfinite (s)) return false; return true; };

    prepare (512);

    // 1. silence when idle
    auto idle = render (44100, 512);
    CHECK (peak (idle) < 1e-6f, "idle output is silent");

    // 2. pad via MIDI keyboard state
    p.keyboardState.noteOn (1, 36, 1.0f);
    auto hit = render (22050, 512);
    CHECK (peak (hit) > 0.2f && finite (hit), "kick pad (C1) makes finite, audible output");
    CHECK (firstAbove (hit, 0.01f) < 600, "pad latency within one block");

    // 3. sequencer onsets: isolate snares (steps 4 and 12), 1 bar, no wow -> loop is 2.0 s at 120 BPM
    prepare (512);
    setp (ids::hostSync, 0); setp (ids::run, 1); setp (ids::bpm, 120); setp (ids::wow, 0);
    setp (ids::bars, 0); setp (ids::variation, 0); setp (ids::rolls, 0); setp (ids::cowbell, 0);
    setp (ids::lvlKick, 0); setp (ids::lvlHat, 0); setp (ids::lvlCow, 0);
    pump (200);                                       // let the pattern republish
    prepare (512);
    auto seq = render (44100 * 5, 512);
    long s1 = firstAbove (seq, 0.02f);
    long s2 = firstAbove (seq, 0.02f, (size_t) s1 + 20000);
    long s3 = firstAbove (seq, 0.02f, (size_t) s2 + 20000);
    std::printf ("      snare onsets at samples %ld %ld %ld (expect ~22050, 66150, 110250)\n", s1, s2, s3);
    CHECK (std::abs (s1 - 22050) < 40 && std::abs (s2 - 66150) < 40 && std::abs (s3 - 110250) < 40,
           "internal clock: snares land sample-accurately and the loop wraps");

    // 4. block-size independence (identical audio for 64 vs 480-sample blocks)
    prepare (64);   auto a = render (44100 * 2, 64);
    prepare (480);  auto b = render (44100 * 2, 480);
    float maxDiff = 0; for (size_t i = 0; i < std::min (a.size(), b.size()); ++i) maxDiff = std::max (maxDiff, std::abs (a[i] - b[i]));
    std::printf ("      max diff between block sizes: %g\n", maxDiff);
    CHECK (maxDiff < 1e-5f && peak (a) > 0.05f, "output is independent of host block size");

    // 5. host sync: the host tempo (140) must override the BPM knob; snare = 4 steps = 1 beat in
    setp (ids::hostSync, 1);
    prepare (512);
    TestPlayHead ph; ph.bpm = 140; p.setPlayHead (&ph);
    auto hostOut = render (44100 * 2, 512, &ph);
    long h1 = firstAbove (hostOut, 0.02f);
    std::printf ("      host-synced snare at sample %ld (expect ~%ld)\n", h1, (long) (44100 * 60.0 / 140.0));
    CHECK (std::abs (h1 - (long) (44100 * 60.0 / 140.0)) < 600, "host tempo + PPQ position drive the sequencer");
    ph.playing = false;
    auto stopped = render (22050, 512, &ph);
    CHECK (peak (stopped, 30000 > stopped.size() ? 0 : 0) >= 0.f && peak (stopped, 15000) < 0.01f, "stops with the host transport (tail decays)");
    p.setPlayHead (nullptr);

    // 6. state round trip
    setp (ids::bpm, 133.3f);
    juce::MemoryBlock mb; p.getStateInformation (mb);
    setp (ids::bpm, 90.f);
    p.setStateInformation (mb.getData(), (int) mb.getSize());
    CHECK (std::abs (p.apvts.getRawParameterValue (ids::bpm)->load() - 133.3f) < 0.2f, "state save / restore");

    // 7. export (29 files) + drag-out loop
    auto dir = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("PhonkSelfTestExport");
    dir.deleteRecursively();
    int exported = -1;
    CHECK (p.exportKitTo (dir, [&] (int n) { exported = n; }), "export starts");
    for (int i = 0; i < 100 && exported < 0; ++i) pump (100);
    CHECK (exported == 29 && dir.getChildFile ("Phonk_Kick_01.wav").existsAsFile() && dir.getChildFile ("Memphis_Loop_112BPM.wav").existsAsFile(),
           "exported 29 WAVs incl. kicks and classic loops");
    auto loopFile = p.renderLoopFile();
    CHECK (loopFile.existsAsFile() && loopFile.getSize() > 100000, "drag-out loop WAV rendered");
    dir.deleteRecursively();

    // 8. editor builds and paints (snapshot for eyeballing)
    setp (ids::hostSync, 0); setp (ids::run, 1); setp (ids::bars, 1); setp (ids::variation, 0.6f); setp (ids::lvlKick, 1); setp (ids::lvlHat, 1);
    setp (ids::lvlCow, 1); setp (ids::wow, 0.5f);
    pump (300); prepare (512);
    p.keyboardState.noteOn (1, 40, 1.0f);
    render (44100 / 2, 512);
    std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
    pump (150);
    auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
    CHECK (img.isValid() && img.getWidth() == 940, "editor creates and paints");
    if (argc > 1)
    {
        juce::File out (argv[1]);
        out.deleteFile();
        juce::FileOutputStream fo (out);
        if (fo.openedOk()) juce::PNGImageFormat().writeImageToStream (img, fo);
        std::printf ("      snapshot -> %s\n", argv[1]);
    }
    ed.reset();

    std::printf (failures ? "\n%d FAILED\n" : "\nall passed\n", failures);
    return failures ? 1 : 0;
}
