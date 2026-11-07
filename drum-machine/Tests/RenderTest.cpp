// Offline smoke test: renders every factory kit (plus a forged kit) playing the
// built-in patterns, checks the output is finite, non-silent and not clipping
// wildly, verifies state save/restore, and writes WAV files to listen to.
//
//   KitForgeRenderTest [output-directory]

#include <juce_audio_formats/juce_audio_formats.h>
#include "PluginProcessor.h"
#include "Exporter.h"

namespace
{
    int failures = 0;

    void check (bool condition, const juce::String& what)
    {
        std::cout << (condition ? "  [ok]   " : "  [FAIL] ") << what << std::endl;
        if (! condition) ++failures;
    }

    juce::AudioBuffer<float> render (KitForgeProcessor& proc, double sr, int block, double seconds)
    {
        const int total = (int) (sr * seconds);
        juce::AudioBuffer<float> out (2, total);
        juce::AudioBuffer<float> buf (2, block);
        juce::MidiBuffer midi;

        for (int pos = 0; pos < total; pos += block)
        {
            const int n = std::min (block, total - pos);
            buf.setSize (2, n, false, false, true);
            midi.clear();
            proc.processBlock (buf, midi);
            for (int ch = 0; ch < 2; ++ch)
                out.copyFrom (ch, pos, buf, ch, 0, n);
        }
        return out;
    }

    void writeWav (const juce::AudioBuffer<float>& buffer, double sr, const juce::File& file)
    {
        file.deleteFile();
        juce::WavAudioFormat wav;
        std::unique_ptr<juce::OutputStream> stream (file.createOutputStream());
        if (stream == nullptr) return;
        if (auto* writer = wav.createWriterFor (stream.get(), sr, 2, 24, {}, 0))
        {
            stream.release();
            std::unique_ptr<juce::AudioFormatWriter> w (writer);
            w->writeFromAudioSampleBuffer (buffer, 0, buffer.getNumSamples());
        }
    }

    bool allFinite (const juce::AudioBuffer<float>& b)
    {
        for (int ch = 0; ch < b.getNumChannels(); ++ch)
            for (int i = 0; i < b.getNumSamples(); ++i)
                if (! std::isfinite (b.getSample (ch, i))) return false;
        return true;
    }
}

int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    const juce::File outDir = argc > 1 ? juce::File::getCurrentWorkingDirectory().getChildFile (argv[1])
                                       : juce::File::getCurrentWorkingDirectory().getChildFile ("renders");
    outDir.createDirectory();

    constexpr double sr = 48000.0;
    constexpr int block = 256;

    KitForgeProcessor proc;
    proc.setPlayConfigDetails (0, 2, sr, block);
    proc.prepareToPlay (sr, block);

    auto renderKit = [&] (const kf::Kit& kit, int pattern, const juce::String& fileName)
    {
        proc.loadKit (kit);
        kf::setParamValue (proc.apvts, kf::ids::pattern, (float) pattern);
        proc.setInternalPlaying (false);
        render (proc, sr, block, 1.0); // let tails die
        proc.setInternalPlaying (true);
        auto out = render (proc, sr, block, 8.0); // 4 bars at 120 BPM
        proc.setInternalPlaying (false);

        const float peak = out.getMagnitude (0, out.getNumSamples());
        const float rms = 0.5f * (out.getRMSLevel (0, 0, out.getNumSamples()) + out.getRMSLevel (1, 0, out.getNumSamples()));
        std::cout << kit.name << " (pattern " << (char) ('A' + pattern) << "): peak "
                  << juce::Decibels::gainToDecibels (peak) << " dBFS, rms "
                  << juce::Decibels::gainToDecibels (rms) << " dBFS" << std::endl;

        check (allFinite (out), "output is finite");
        check (rms > 0.01f, "output is not silent");
        check (peak <= 1.0f, "output never exceeds 0 dBFS");
        writeWav (out, sr, outDir.getChildFile (fileName));
    };

    for (int k = 0; k < kf::KitFactory::factoryKitNames().size(); ++k)
        renderKit (kf::KitFactory::factoryKit (k), k % 2, "kit_" + juce::String (k + 1) + ".wav");

    juce::Random rng (1234);
    for (int k = 0; k < 3; ++k)
    {
        auto kit = kf::KitFactory::forgeKit (rng);
        kf::KitFactory::generatePattern (proc.patterns, 2 + k, proc.apvts, rng);
        renderKit (kit, 2 + k, "forged_" + juce::String (k + 1) + ".wav");
    }

    // ---- Every drum type, forged many times, must produce a sound that ends ----
    std::cout << "Forging 200 voices of every type..." << std::endl;
    {
        bool ok = true;
        for (int type = 0; type < kf::voiceTypeNames().size(); ++type)
        {
            for (int i = 0; i < 200; ++i)
            {
                auto def = kf::KitFactory::forgeVoice ((kf::VoiceType) type, rng);
                for (int vp = 0; vp < kf::NumVP; ++vp)
                {
                    const auto& s = kf::voiceSpec (vp);
                    if (def[(size_t) vp] < s.min || def[(size_t) vp] > s.max)
                    {
                        std::cout << "    out of range: " << kf::voiceTypeNames()[type] << " " << s.id
                                  << " = " << def[(size_t) vp] << std::endl;
                        ok = false;
                    }
                }
            }
        }
        check (ok, "forged values are within parameter ranges");
    }

    // ---- MIDI input triggers tracks ----
    {
        proc.loadKit (kf::KitFactory::factoryKit (0));
        proc.setInternalPlaying (false);
        render (proc, sr, block, 5.0);
        juce::AudioBuffer<float> buf (2, block);
        juce::MidiBuffer midi;
        midi.addEvent (juce::MidiMessage::noteOn (1, kf::midiBaseNote, (juce::uint8) 120), 10);
        proc.processBlock (buf, midi);
        check (buf.getMagnitude (0, block) > 0.01f, "MIDI note C1 triggers the kick");
    }

    // ---- State round trip ----
    {
        proc.patterns.set (5, 3, 7, 99);
        kf::setParamValue (proc.apvts, kf::voiceParamID (2, kf::Pitch), 333.0f);
        proc.setKitName ("Round Trip");

        juce::MemoryBlock state;
        proc.getStateInformation (state);

        KitForgeProcessor other;
        other.setStateInformation (state.getData(), (int) state.getSize());
        check (other.patterns.get (5, 3, 7) == 99, "pattern data survives save/restore");
        check (std::abs (kf::getParamValue (other.apvts, kf::voiceParamID (2, kf::Pitch)) - 333.0f) < 1.0f,
               "voice parameters survive save/restore");
        check (other.getKitName() == "Round Trip", "kit name survives save/restore");
    }

    // ---- Kit XML round trip ----
    {
        auto kit = kf::KitFactory::forgeKit (rng);
        auto xml = kf::KitFactory::kitToXml (kit);
        kf::Kit back;
        check (kf::KitFactory::kitFromXml (*xml, back), "kit XML parses");
        bool same = back.name == kit.name;
        for (int t = 0; t < kf::numTracks; ++t)
            for (int vp = 0; vp < kf::NumVP; ++vp)
                if (kf::isKitParam (vp) && std::abs (back.voices[(size_t) t][(size_t) vp] - kit.voices[(size_t) t][(size_t) vp]) > 1.0e-3f)
                    same = false;
        check (same, "kit XML round trip is lossless");
    }


    // ---- Vintage bus: 12-bit quantisation ----
    {
        kf::BusParams bp;
        bp.bits = 12.0f;
        kf::VintageCrusher c;
        c.prepare (sr);
        c.setParams (bp);
        bool onGrid = true;
        for (int i = 0; i < 2000; ++i)
        {
            float l = 0.7f * std::sin ((float) i * 0.013f), r = l;
            c.process (l, r);
            const float steps = l * 2048.0f;
            onGrid &= std::abs (steps - std::round (steps)) < 1.0e-3f;
        }
        check (onGrid, "12-bit mode quantises to 4096 levels");
    }

    // ---- Vintage bus: decimation + pitch trick ----
    {
        kf::BusParams bp;
        bp.rate = 12000.0f;
        kf::VintageCrusher c;
        c.prepare (sr);
        c.setParams (bp);
        int changes = 0;
        float prev = 0.0f;
        for (int i = 0; i < 4800; ++i)
        {
            float l = std::sin ((float) i * 0.05f), r = l;
            c.process (l, r);
            if (std::abs (l - prev) > 0.0f) ++changes;
            prev = l;
        }
        check (std::abs (changes - 1200) <= 2, "12 kHz decimation holds each sample for 4 output samples");

        bp.rate = 26040.0f;
        bp.pitch = 5.0f;
        c.setParams (bp);
        check (std::abs (c.getEffectiveRate() - 26040.0 / std::pow (2.0, 5.0 / 12.0)) < 1.0,
               "pitch trick lowers the effective rate (26.04 kHz, +5 st -> " + juce::String (c.getEffectiveRate() / 1000.0, 2) + " kHz)");
    }

    // ---- Vintage bus: Low Keep protects the sub ----
    {
        auto crushError = [&] (float lowKeep)
        {
            kf::BusParams harsh;
            harsh.bits = 12.0f;
            harsh.rate = 8000.0f;
            harsh.lowKeep = lowKeep;
            kf::BusParams clean;
            clean.lowKeep = lowKeep;

            kf::VintageCrusher a, b;
            a.prepare (sr); a.setParams (harsh);
            b.prepare (sr); b.setParams (clean);
            double err = 0.0, sig = 0.0;
            for (int i = 0; i < (int) sr; ++i)
            {
                const float x = 0.8f * std::sin (2.0f * juce::MathConstants<float>::pi * 45.0f * (float) i / (float) sr);
                float l1 = x, r1 = x, l2 = x, r2 = x;
                a.process (l1, r1);
                b.process (l2, r2);
                if (i > (int) (0.1 * sr)) { err += (l1 - l2) * (l1 - l2); sig += l2 * l2; }
            }
            return 10.0 * std::log10 (err / sig);
        };
        const double without = crushError (0.0f), with = crushError (100.0f);
        std::cout << "  45 Hz sub through 12-bit/8 kHz: distortion " << without << " dB (Low Keep off), "
                  << with << " dB (Low Keep 100 Hz)" << std::endl;
        check (with < without - 20.0, "Low Keep keeps the sub clean");
    }

    // ---- Transient shaper boosts attacks ----
    {
        auto peakWith = [&] (float punch)
        {
            kf::TransientShaper ts;
            ts.prepare (sr);
            ts.setAmounts (punch, 0.0f);
            float peak = 0.0f;
            for (int i = 0; i < 4800; ++i)
            {
                float x = 0.5f * std::exp (-(float) i / 2000.0f) * std::sin ((float) i * 0.05f), y = x;
                ts.process (x, y);
                if (i < 480) peak = std::max (peak, std::abs (x));
            }
            return peak;
        };
        check (peakWith (1.0f) > peakWith (0.0f) * 1.1f, "Punch raises the attack");
    }

    // ---- Sample pack export ----
    {
        const auto dir = outDir.getChildFile ("pack_test");
        dir.deleteRecursively();
        auto kit = kf::KitFactory::factoryKit (kf::KitFactory::factoryKitNames().indexOf ("Jazz Phonk (16 pads)"));
        check (kit.voiceNames[0] == "Jazz Sub Kick" && kit.voiceNames[12] == "Sub Glide 808"
                   && (kf::VoiceType) (int) kit.voices[12][kf::Type] == kf::VoiceType::Sub808,
               "Jazz Phonk 16-pad kit is available (A01 Jazz Sub Kick .. A13 Sub Glide 808)");
        kf::Exporter::Options opt;
        const auto result = kf::Exporter::exportKit (kit, dir, opt);
        check (result.ok(), "export succeeds");
        int wavs = 0;
        bool levelsOk = true;
        juce::AudioFormatManager fm;
        fm.registerBasicFormats();
        for (auto& f : result.files)
        {
            std::cout << "    " << f << std::endl;
            if (! f.endsWith (".wav") || f.contains ("Texture")) continue;
            ++wavs;
            std::unique_ptr<juce::AudioFormatReader> reader (fm.createReaderFor (dir.getChildFile (f)));
            if (reader == nullptr) { levelsOk = false; continue; }
            juce::AudioBuffer<float> b ((int) reader->numChannels, (int) reader->lengthInSamples);
            reader->read (&b, 0, (int) reader->lengthInSamples, 0, true, true);
            const float peakDb = juce::Decibels::gainToDecibels (b.getMagnitude (0, b.getNumSamples()));
            levelsOk &= peakDb <= -0.9f && peakDb >= -4.1f && juce::approximatelyEqual (reader->sampleRate, 44100.0) && reader->bitsPerSample == 24;
        }
        check (wavs == kf::numTracks, "one WAV per pad");
        check (levelsOk, "WAVs are 24-bit/44.1 kHz, normalised to -1..-4 dBFS");
        check (result.files.contains ("JazzPhonk_Kick_JazzSubKick_A01_12bit.wav"), "files follow the naming convention");
        check (dir.getChildFile ("JazzPhonk.sfz").loadFileAsString().contains ("off_by=1"), "SFZ keymap with hat choke");
        check (dir.getChildFile ("JazzPhonk.sfz").loadFileAsString().contains ("A16_12bit.wav key=51"), "SFZ maps pad A16 to note 51");
        check (result.files.contains ("JazzPhonk_808Sub_SubGlide808_A13.wav"), "the 808 sub has no 12-bit tag (Lo-Fi Send 0)");
    }

    // ---- 16 pads: every factory kit has 16 real voices ----
    {
        bool ok = true;
        for (int k = 0; k < kf::KitFactory::factoryKitNames().size(); ++k)
        {
            const auto kit = kf::KitFactory::factoryKit (k);
            for (int t = 0; t < kf::numTracks; ++t)
                ok &= kit.voices[(size_t) t][kf::Level] > 0.0f && kit.voiceNames[(size_t) t].isNotEmpty();
        }
        check (ok, "all factory kits fill 16 pads");

        // An 8-pad kit file gets a derived second bank
        auto xml = kf::KitFactory::kitToXml (kf::KitFactory::factoryKit (0));
        for (auto* v = xml->getChildByName ("Voice"); v != nullptr;)
        {
            auto* next = v->getNextElementWithTagName ("Voice");
            if (v->getIntAttribute ("index") >= 8) xml->removeChildElement (v, true);
            v = next;
        }
        kf::Kit old;
        kf::KitFactory::kitFromXml (*xml, old);
        check (old.voiceNames[12] == "808 Sub" && old.voices[15][kf::Level] > 0.0f, "8-pad kit files load with a derived bank 2");

        proc.loadKit (kf::KitFactory::factoryKit (0));
        proc.setInternalPlaying (false);
        render (proc, sr, block, 5.0);
        juce::AudioBuffer<float> buf (2, block);
        juce::MidiBuffer midi;
        midi.addEvent (juce::MidiMessage::noteOn (1, kf::midiBaseNote + 15, (juce::uint8) 120), 0);
        const int before = proc.getTriggerCount (15);
        proc.processBlock (buf, midi);
        check (proc.getTriggerCount (15) == before + 1, "MIDI note 51 (D#2) plays pad 16");
    }

    // ---- Ratchets / hat rolls ----
    {
        constexpr int pat = 7, tr = 3;
        proc.patterns.clear (pat);
        proc.patterns.set (pat, tr, 0, 100);  proc.patterns.setRatchet (pat, tr, 0, 4);   // 1/64
        proc.patterns.set (pat, tr, 4, 100);  proc.patterns.setRatchet (pat, tr, 4, 3);   // triplet roll
        proc.patterns.set (pat, tr, 8, 100);  proc.patterns.setRatchet (pat, tr, 8, 2);   // 1/32
        proc.patterns.set (pat, tr, 12, 100);                                             // single
        kf::setParamValue (proc.apvts, kf::ids::pattern, (float) pat);
        kf::setParamValue (proc.apvts, kf::ids::tempo, 120.0f);

        for (float swingAmount : { 50.0f, 66.0f })
        {
            kf::setParamValue (proc.apvts, kf::ids::swing, swingAmount);
            proc.setInternalPlaying (false);
            render (proc, sr, block, 0.1);
            const int before = proc.getTriggerCount (tr);
            proc.setInternalPlaying (true);
            render (proc, sr, block, 1.99); // just under one bar at 120 BPM
            proc.setInternalPlaying (false);
            const int hits = proc.getTriggerCount (tr) - before;
            check (hits == 4 + 3 + 2 + 1, "ratchets x4 + x3 + x2 + x1 give 10 hits per bar (swing "
                                           + juce::String (swingAmount, 0) + "%, got " + juce::String (hits) + ")");
        }
        kf::setParamValue (proc.apvts, kf::ids::swing, 50.0f);

        // Exact positions of a x4 roll: 1/64 notes at 120 BPM = 31.25 ms apart
        proc.patterns.clear (pat);
        proc.patterns.set (pat, tr, 0, 127);
        proc.patterns.setRatchet (pat, tr, 0, 4);
        proc.loadKit (kf::KitFactory::factoryKit (5)); // short hats
        proc.setInternalPlaying (false);
        render (proc, sr, block, 2.0);
        for (int t = 0; t < kf::numTracks; ++t)
            kf::setParamValue (proc.apvts, kf::voiceParamID (t, kf::Solo), t == tr ? 1.0f : 0.0f);
        proc.setInternalPlaying (true);
        auto out = render (proc, sr, 64, 0.2);
        proc.setInternalPlaying (false);
        kf::setParamValue (proc.apvts, kf::voiceParamID (tr, kf::Solo), 0.0f);

        std::vector<int> onsets;
        bool armed = true;
        for (int i = 0; i < out.getNumSamples(); ++i)
        {
            const float a = std::abs (out.getSample (0, i));
            if (armed && a > 0.02f) { onsets.push_back (i); armed = false; }
            // re-arm after the hit has decayed for a bit
            if (! armed && ! onsets.empty() && i - onsets.back() > (int) (0.02 * sr)) armed = true;
        }
        bool spacingOk = onsets.size() >= 4;
        for (size_t i = 1; i < std::min<size_t> (onsets.size(), 4); ++i)
            spacingOk &= std::abs ((onsets[i] - onsets[i - 1]) - (int) (0.03125 * sr)) < (int) (0.002 * sr);
        check (spacingOk, "x4 roll hits are 1/64 notes (31.25 ms) apart");

        // ratchets survive state save/restore
        proc.patterns.setRatchet (6, 11, 9, 3);
        proc.patterns.set (6, 11, 9, 90);
        juce::MemoryBlock state;
        proc.getStateInformation (state);
        KitForgeProcessor other;
        other.setStateInformation (state.getData(), (int) state.getSize());
        check (other.patterns.getRatchet (6, 11, 9) == 3 && other.patterns.get (6, 11, 9) == 90,
               "ratchets and pad 12 data survive save/restore");
    }

    std::cout << (failures == 0 ? "ALL TESTS PASSED" : juce::String (failures) + " TEST(S) FAILED")
              << "  (renders in " << outDir.getFullPathName() << ")" << std::endl;
    return failures == 0 ? 0 : 1;
}
