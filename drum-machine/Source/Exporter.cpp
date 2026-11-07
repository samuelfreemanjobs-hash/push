#include "Exporter.h"

#include <juce_audio_formats/juce_audio_formats.h>

namespace kf::Exporter
{
float targetPeakDb (VoiceType type)
{
    switch (type)
    {
        case VoiceType::Kick:
        case VoiceType::Sub808:
        case VoiceType::Snare:
        case VoiceType::Clap:      return -1.0f;
        case VoiceType::Tom:
        case VoiceType::Rim:       return -2.0f;
        case VoiceType::Cowbell:
        case VoiceType::Perc:      return -3.0f;
        case VoiceType::ClosedHat:
        case VoiceType::OpenHat:
        case VoiceType::Shaker:
        case VoiceType::Cymbal:    return -4.0f;
    }
    return -1.0f;
}

juce::String sanitise (const juce::String& s)
{
    juce::String out;
    bool upperNext = true;
    for (auto c : s.replace ("#", "sharp"))
    {
        if (juce::CharacterFunctions::isLetterOrDigit (c))
        {
            out << juce::String::charToString (upperNext ? juce::CharacterFunctions::toUpperCase (c) : c);
            upperNext = false;
        }
        else
        {
            upperNext = true;
        }
    }
    return out.isEmpty() ? juce::String ("Untitled") : out;
}

namespace
{
    bool writeWav (const juce::AudioBuffer<float>& buffer, double sr, int bits, const juce::File& file)
    {
        file.deleteFile();
        std::unique_ptr<juce::OutputStream> stream (file.createOutputStream());
        if (stream == nullptr)
            return false;

        juce::WavAudioFormat wav;
        std::unique_ptr<juce::AudioFormatWriter> writer (
            wav.createWriterFor (stream.get(), sr, (unsigned int) buffer.getNumChannels(), bits, {}, 0));
        if (writer == nullptr)
            return false;
        stream.release(); // owned by the writer now
        return writer->writeFromAudioSampleBuffer (buffer, 0, buffer.getNumSamples());
    }

    void trimAndFade (juce::AudioBuffer<float>& b, double sr)
    {
        const float peak = b.getMagnitude (0, b.getNumSamples());
        if (peak <= 0.0f)
        {
            b.setSize (b.getNumChannels(), 1);
            b.clear();
            return;
        }

        // Cut trailing material quieter than -80 dB below the peak.
        const float threshold = peak * 1.0e-4f;
        int end = b.getNumSamples();
        while (end > 1)
        {
            bool loud = false;
            for (int ch = 0; ch < b.getNumChannels(); ++ch)
                loud |= std::abs (b.getSample (ch, end - 1)) > threshold;
            if (loud) break;
            --end;
        }

        const int fade = juce::jmin (end, (int) (0.005 * sr));
        end = juce::jmin (b.getNumSamples(), end + fade);
        b.setSize (b.getNumChannels(), end, true);
        b.applyGainRamp (end - fade, fade, 1.0f, 0.0f);
    }

    juce::String bitsTag (const BusDef& bus, const VoiceDef& voice)
    {
        if (voice[LoFiSend] <= 0.0f || bus[BBits] >= 15.99f)
            return {};
        return "_" + juce::String (juce::roundToInt (bus[BBits])) + "bit";
    }
}

juce::AudioBuffer<float> renderVoice (const VoiceDef& def, const BusDef& busDef, double sr)
{
    const auto vp = VoiceParams::fromValues (def);
    const auto bp = BusParams::fromValues (busDef);

    DrumVoice voice;
    voice.prepare (sr);
    voice.setParams (vp);

    VintageCrusher crusher;
    crusher.prepare (sr);
    crusher.setParams (bp);

    TransientShaper shaper;
    shaper.prepare (sr);
    shaper.setAmounts (bp.punch, bp.tail);

    const int maxLen = (int) (16.0 * sr); // long 808s
    const int tail = (int) (0.05 * sr);
    juce::AudioBuffer<float> out (1, maxLen);
    out.clear();

    voice.trigger (1.0f);
    int n = 0, silentFor = 0;
    for (; n < maxLen && silentFor < tail; ++n)
    {
        const float x = voice.isActive() ? voice.process() : 0.0f;
        float sendL = x * vp.lofiSend, sendR = sendL;
        crusher.process (sendL, sendR);
        float y = x * (1.0f - vp.lofiSend) + sendL, y2 = y;
        shaper.process (y, y2);
        out.setSample (0, n, y);

        silentFor = voice.isActive() ? 0 : silentFor + 1;
    }

    out.setSize (1, juce::jmax (1, n), true);
    trimAndFade (out, sr);
    return out;
}

juce::String padFileName (const VoiceDef& voice, const juce::String& name, const BusDef& bus)
{
    const auto type = (VoiceType) (int) voice[Type];
    return sanitise (voiceTypeNames()[(int) type]) + "_" + sanitise (name) + bitsTag (bus, voice) + ".wav";
}

bool exportPad (const VoiceDef& voice, const BusDef& bus, const juce::File& file, const Options& opt)
{
    auto audio = renderVoice (voice, bus, opt.sampleRate);
    if (opt.normalise)
        if (const float peak = audio.getMagnitude (0, audio.getNumSamples()); peak > 0.0f)
            audio.applyGain (juce::Decibels::decibelsToGain (targetPeakDb ((VoiceType) (int) voice[Type])) / peak);
    file.getParentDirectory().createDirectory();
    return writeWav (audio, opt.sampleRate, opt.bitDepth, file);
}

Result exportKit (const Kit& kit, const juce::File& folder, const Options& opt)
{
    Result result;
    if (! folder.createDirectory())
    {
        result.error = "Can't create folder " + folder.getFullPathName();
        return result;
    }

    const auto bus = kit.hasBus ? kit.bus : KitFactory::defaultBus();
    const auto prefix = sanitise ((opt.prefix.isNotEmpty() ? opt.prefix : kit.name).upToFirstOccurrenceOf (" (", false, false));

    juce::String sfz;
    sfz << "// " << kit.name << " -- exported by KitForge\n"
        << "// Pads A" << juce::String (opt.firstPad).paddedLeft ('0', 2) << "..A"
        << juce::String (opt.firstPad + numTracks - 1).paddedLeft ('0', 2)
        << " map to MIDI notes " << (midiBaseNote + opt.firstPad - 1) << ".." << (midiBaseNote + opt.firstPad - 1 + numTracks - 1) << "\n\n"
        << "<control>\ndefault_path=./\n\n<global>\nloop_mode=one_shot\namp_veltrack=100\n\n";

    for (int t = 0; t < numTracks; ++t)
    {
        const auto& def = kit.voices[(size_t) t];
        const auto type = (VoiceType) (int) def[Type];
        const int pad = opt.firstPad + t;
        const auto padTag = "A" + juce::String (pad).paddedLeft ('0', 2);

        auto audio = renderVoice (def, bus, opt.sampleRate);
        if (opt.normalise)
        {
            const float peak = audio.getMagnitude (0, audio.getNumSamples());
            if (peak > 0.0f)
                audio.applyGain (juce::Decibels::decibelsToGain (targetPeakDb (type)) / peak);
        }

        const auto fileName = prefix + "_" + sanitise (voiceTypeNames()[(int) type]) + "_"
                            + sanitise (kit.voiceNames[(size_t) t]) + "_" + padTag + bitsTag (bus, def) + ".wav";
        if (! writeWav (audio, opt.sampleRate, opt.bitDepth, folder.getChildFile (fileName)))
        {
            result.error = "Couldn't write " + fileName;
            return result;
        }
        result.files.add (fileName);

        sfz << "<region> sample=" << fileName << " key=" << (midiBaseNote + pad - 1);
        if (const int choke = juce::roundToInt (def[Choke]); choke > 0)
            sfz << " group=" << choke << " off_by=" << choke << " off_mode=fast";
        sfz << "\n";
    }

    if (opt.writeVinylLoop && bus[BVinyl] > 0.0f)
    {
        // A seamless-enough 4 second surface-noise texture through the same bus.
        const double sr = opt.sampleRate;
        const int len = (int) (4.0 * sr);
        VinylNoise vinyl;
        vinyl.prepare (sr);
        vinyl.setAmount (juce::jmax (0.3f, bus[BVinyl]));
        VintageCrusher crusher;
        crusher.prepare (sr);
        crusher.setParams (BusParams::fromValues (bus));

        juce::AudioBuffer<float> loop (2, len);
        for (int i = 0; i < len; ++i)
        {
            float l = 0.0f, r = 0.0f;
            vinyl.process (l, r);
            crusher.process (l, r);
            loop.setSample (0, i, l);
            loop.setSample (1, i, r);
        }
        // crossfade the ends so it loops without a click
        const int xf = (int) (0.02 * sr);
        loop.applyGainRamp (0, xf, 0.0f, 1.0f);
        loop.applyGainRamp (len - xf, xf, 1.0f, 0.0f);
        if (const float peak = loop.getMagnitude (0, len); peak > 0.0f)
            loop.applyGain (juce::Decibels::decibelsToGain (-12.0f) / peak);

        const auto fileName = prefix + "_Texture_VinylLoop" + (bus[BBits] < 15.99f ? "_" + juce::String (juce::roundToInt (bus[BBits])) + "bit" : juce::String()) + ".wav";
        if (writeWav (loop, sr, opt.bitDepth, folder.getChildFile (fileName)))
            result.files.add (fileName);
    }

    if (opt.writeSfz)
    {
        const auto sfzName = prefix + ".sfz";
        if (folder.getChildFile (sfzName).replaceWithText (sfz))
            result.files.add (sfzName);
    }

    if (opt.writeKitFile)
    {
        const auto kitName = prefix + ".kitforge";
        if (auto xml = KitFactory::kitToXml (kit); xml->writeTo (folder.getChildFile (kitName)))
            result.files.add (kitName);
    }

    return result;
}
} // namespace kf::Exporter
