// KitForgeFactory -- batch drum-kit factory.
//
// Generates kits with the KitForge synth engine, runs them through a vintage
// sampler emulation and writes ready-to-use sample packs (WAV + SFZ + .kitforge).
//
//   KitForgeFactory --factory                      export every factory kit
//   KitForgeFactory --kits 10 --machine 1          forge 10 kits through the SP-1200 style bus
//   KitForgeFactory --kits 4 --seed 42 --out packs --rate 48000 --bits 24 --low-keep 100
//
// Options:
//   --kits N         number of kits to forge (default 5)
//   --factory        export the factory kits instead of forging
//   --seed S         random seed, so a batch can be reproduced exactly
//   --machine M      0 clean, 1 SP-1200 style, 2 MPC60 style, 3 DR-660 phonk, 4 SP-808/Lofi-12 (default 1)
//   --random-machine pick a machine per kit
//   --low-keep HZ    crossover below which the lows bypass the crusher (default: machine's value)
//   --punch X        transient punch -1..1 (default 0.25 = about +1.5 dB attack)
//   --out DIR        output folder (default ./KitForge Packs)
//   --rate HZ        file sample rate (default 44100)
//   --bits B         file bit depth 16/24/32 (default 24)

#include "Exporter.h"
#include <iostream>

namespace
{
    juce::String argValue (const juce::StringArray& args, const juce::String& name, const juce::String& fallback)
    {
        const int i = args.indexOf (name);
        return (i >= 0 && i + 1 < args.size()) ? args[i + 1] : fallback;
    }
}

int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    juce::StringArray args;
    for (int i = 1; i < argc; ++i)
        args.add (argv[i]);

    if (args.contains ("--help") || args.contains ("-h"))
    {
        std::cout << "KitForgeFactory [--factory | --kits N] [--seed S] [--machine 0-4 | --random-machine]\n"
                     "                [--low-keep HZ] [--punch X] [--out DIR] [--rate HZ] [--bits B]\n\nMachines:\n";
        for (int i = 0; i < kf::machineNames().size(); ++i)
            std::cout << "  " << i << "  " << kf::machineNames()[i] << "\n";
        return 0;
    }

    const bool factory = args.contains ("--factory");
    const int numKits = juce::jlimit (1, 1000, argValue (args, "--kits", "5").getIntValue());
    const auto seed = argValue (args, "--seed", juce::String (juce::Time::currentTimeMillis())).getLargeIntValue();
    const int machine = juce::jlimit (0, kf::machineNames().size() - 1, argValue (args, "--machine", "1").getIntValue());
    const bool randomMachine = args.contains ("--random-machine");
    const float punch = juce::jlimit (-1.0f, 1.0f, argValue (args, "--punch", "0.25").getFloatValue());
    const juce::String lowKeepArg = argValue (args, "--low-keep", {});
    const auto outDir = juce::File::getCurrentWorkingDirectory().getChildFile (argValue (args, "--out", "KitForge Packs"));

    kf::Exporter::Options opt;
    opt.sampleRate = juce::jlimit (8000.0, 192000.0, argValue (args, "--rate", "44100").getDoubleValue());
    opt.bitDepth = argValue (args, "--bits", "24").getIntValue();
    if (opt.bitDepth != 16 && opt.bitDepth != 24 && opt.bitDepth != 32)
        opt.bitDepth = 24;

    juce::Random rng (seed);
    std::vector<std::pair<kf::Kit, int>> kits; // kit, first pad

    if (factory)
    {
        for (int k = 0; k < kf::KitFactory::factoryKitNames().size(); ++k)
        {
            auto kit = kf::KitFactory::factoryKit (k);
            kits.emplace_back (kit, kit.name.contains ("9-16") ? 9 : 1);
        }
    }
    else
    {
        std::cout << "Forging " << numKits << " kits (seed " << seed << ")" << std::endl;
        for (int k = 0; k < numKits; ++k)
        {
            auto kit = kf::KitFactory::forgeKit (rng);
            const int m = randomMachine ? 1 + rng.nextInt (kf::machineNames().size() - 1) : machine;
            kit.hasBus = true;
            kit.bus = kf::KitFactory::machineBus (m);
            kit.bus[kf::BPunch] = punch;
            if (lowKeepArg.isNotEmpty())
                kit.bus[kf::BLowKeep] = juce::jlimit (0.0f, 250.0f, lowKeepArg.getFloatValue());
            kits.emplace_back (kit, 1);
        }
    }

    int failures = 0;
    for (auto& [kit, firstPad] : kits)
    {
        opt.firstPad = firstPad;
        opt.prefix = kit.name;
        const auto folder = outDir.getChildFile (kf::Exporter::sanitise (kit.name.upToFirstOccurrenceOf (" (", false, false)));
        const auto result = kf::Exporter::exportKit (kit, folder, opt);

        if (result.ok())
        {
            std::cout << "  " << kit.name << "  ->  " << folder.getFullPathName() << "  (" << result.files.size() << " files)" << std::endl;
        }
        else
        {
            std::cout << "  FAILED " << kit.name << ": " << result.error << std::endl;
            ++failures;
        }
    }

    return failures == 0 ? 0 : 1;
}
