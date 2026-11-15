# DrumSynth-style autoresearch layout (DiskLordz)

Equivalent to a standalone `DrumSynth-Autoresearch/` tree, embedded in DiskLordz:

```text
disklordz-ralph/vst/
├── starter-cpp/              # JUCE in externals/JUCE — ship plugin
│   └── CMakeLists.txt        # -DDLZ_AUTORESEARCH=ON
└── autoresearch-cpp/
    ├── Source/
    │   ├── VoiceDSP.h        # ONLY agent-edited file
    │   └── HeadlessMain.cpp  # CLI renderer
    ├── targets/kick.wav      # you provide (gitignored)
    ├── outputs/              # rendered_kick.wav, .best_loss
    ├── evaluate_vst.py       # immutable judge
    ├── program_cpp.md        # agent playbook
    └── examples/             # PluginProcessor wiring reference (not built)
```

Commercial VST3 remains **`Detroit Phonk Trap Maker`** in `starter-cpp/` — not a separate DrumSynth SKU.  
**`DrumSynth Autoresearch`** (`plugin/`) is a marketing/QA preview wired directly to `VoiceDSP.h`.  
Autoresearch tunes `VoiceDSP.h`, then you port winners into `TrapCoreDSP`.