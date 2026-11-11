// kitgen - command line front end to the Phonk Factory core (no JUCE needed).
#include "PhonkDSP.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main (int argc, char** argv)
{
    phonk::ExportOptions o;
    std::string out = "Memphis_Phonk_MPC_Kit";
    for (int i = 1; i < argc; ++i)
    {
        auto is = [&] (const char* k) { return ! std::strcmp (argv[i], k) && i + 1 < argc; };
        if (is ("--out")) out = argv[++i];
        else if (is ("--seed")) o.kit.seed = o.pattern.seed = std::atoi (argv[++i]);
        else if (is ("--bpm")) o.currentBpm = std::atof (argv[++i]);
        else if (is ("--bars")) o.pattern.bars = std::atoi (argv[++i]);
        else if (is ("--variation")) o.pattern.variation = std::atof (argv[++i]);
        else if (is ("--bits")) o.bits = std::atoi (argv[++i]);
        else if (is ("--sr")) o.sampleRate = std::atof (argv[++i]);
        else { std::printf ("usage: kitgen [--out dir] [--seed n] [--bpm x] [--bars 1|2|4] [--variation 0..1] [--bits 16|24] [--sr hz]\n"); return 1; }
    }
    const int n = phonk::exportKit (out, o, [] (const std::string& f) { std::printf ("  %s\n", f.c_str()); });
    std::printf ("Done! %d dusty Memphis phonk WAVs saved to ./%s/\n", n, out.c_str());
    return n > 0 ? 0 : 2;
}
