#include "Styles.h"
#include "ProgressionLibrary.h"
#include "BuiltInSynth.h"

namespace forge
{
// Built-in styles. The same schema is accepted from user JSON files, so any of
// these can be dumped with --dump-style and edited.
static const char* builtInStyleJson = R"JSON(
[
 { "id": "neo-soul", "name": "Neo-Soul",
   "description": "Lush 9ths, 11ths and 13ths, rootless voicings, rolled like a Rhodes player.",
   "scale": "dorian",
   "chords": { "m9": 6, "maj9": 5, "m11": 5, "9": 3, "13": 3, "maj7s11": 2, "7s9": 2, "9sus4": 3, "m7": 2, "69": 2 },
   "voicings": { "rootless": 5, "drop2": 4, "spread": 3, "shell": 2, "open": 1 },
   "chordRatio": 0.75, "keyTypes": { "note": 3, "octave": 2, "fifth": 1, "stab": 2 },
   "chordRootRange": [45, 57], "keyRange": [36, 72], "velocity": [70, 100],
   "hold": [1.6, 2.8], "stabChance": 0.12, "stabHold": 0.3, "strumMs": 40 },

 { "id": "lofi", "name": "Lo-Fi Hip Hop",
   "description": "Mellow jazzy 7ths and 9ths, soft velocities, close and drop-2 voicings.",
   "scale": "minor",
   "chords": { "maj7": 5, "m7": 5, "m9": 4, "maj9": 4, "9": 2, "m11": 2, "69": 2, "7sus4": 1, "m7b5": 1 },
   "voicings": { "close": 3, "drop2": 4, "rootless": 3, "inv1": 2, "inv2": 1 },
   "chordRatio": 0.7, "keyTypes": { "note": 4, "octave": 2, "fifth": 1, "stab": 1 },
   "chordRootRange": [45, 57], "keyRange": [40, 76], "velocity": [55, 85],
   "hold": [1.8, 3.0], "stabChance": 0.1, "stabHold": 0.35, "strumMs": 30 },

 { "id": "trap", "name": "Dark Trap",
   "description": "Minor triads, diminished and sus colours, wide low voicings for 808-heavy beats.",
   "scale": "harmonicminor",
   "chords": { "min": 6, "madd9": 4, "m7": 3, "dim": 3, "sus2": 2, "5": 2, "mmaj7": 2, "maj": 2, "aug": 1, "madd11": 2 },
   "voicings": { "close": 3, "open": 3, "spread": 3, "inv1": 2, "inv2": 2 },
   "chordRatio": 0.6, "keyTypes": { "note": 5, "octave": 3, "fifth": 2, "stab": 1 },
   "chordRootRange": [43, 55], "keyRange": [36, 79], "velocity": [85, 120],
   "hold": [1.2, 2.2], "stabChance": 0.25, "stabHold": 0.2, "strumMs": 5 },

 { "id": "drill", "name": "UK / NY Drill",
   "description": "Sinister minor and phrygian colours, tense intervals, short stabs and long dark chords.",
   "scale": "phrygian",
   "chords": { "min": 5, "madd9": 3, "dim": 3, "m7b5": 2, "mmaj7": 3, "5": 2, "sus4": 2, "maj": 2, "aug": 1 },
   "voicings": { "close": 3, "open": 3, "spread": 2, "inv1": 2, "inv2": 2 },
   "chordRatio": 0.6, "keyTypes": { "note": 5, "octave": 3, "fifth": 1, "stab": 2 },
   "chordRootRange": [43, 55], "keyRange": [36, 79], "velocity": [90, 120],
   "hold": [1.0, 2.0], "stabChance": 0.3, "stabHold": 0.18, "strumMs": 4 },

 { "id": "rnb", "name": "Modern R&B",
   "description": "Silky maj9 / m9 / m11 chords with 9sus4 turnarounds and gentle rolls.",
   "scale": "minor",
   "chords": { "maj9": 5, "m9": 5, "m11": 4, "9sus4": 3, "maj13": 2, "m7": 2, "maj7": 2, "add9": 2, "69": 1 },
   "voicings": { "rootless": 4, "drop2": 4, "spread": 3, "open": 2 },
   "chordRatio": 0.75, "keyTypes": { "note": 3, "octave": 2, "fifth": 1, "stab": 1 },
   "chordRootRange": [45, 57], "keyRange": [40, 76], "velocity": [65, 100],
   "hold": [1.8, 3.0], "stabChance": 0.1, "stabHold": 0.3, "strumMs": 35 },

 { "id": "gospel", "name": "Gospel",
   "description": "Big church chords: 13ths, altered dominants, 6/9s and walk-up colours.",
   "scale": "major",
   "chords": { "maj9": 4, "13": 4, "7s9": 3, "7b9": 3, "69": 3, "m11": 3, "9sus4": 3, "13b9": 2, "dim7": 2, "m9": 3, "maj13": 2 },
   "voicings": { "spread": 4, "rootless": 3, "drop2": 3, "shell": 2, "open": 2 },
   "chordRatio": 0.8, "keyTypes": { "note": 2, "octave": 3, "fifth": 1, "stab": 2 },
   "chordRootRange": [43, 55], "keyRange": [36, 72], "velocity": [80, 115],
   "hold": [1.5, 2.6], "stabChance": 0.2, "stabHold": 0.3, "strumMs": 25 },

 { "id": "house", "name": "Deep House",
   "description": "Classic house stabs and pads: m7, m9, m11 and organ-style sus chords.",
   "scale": "minor",
   "chords": { "m7": 5, "m9": 5, "m11": 4, "7sus4": 2, "9sus4": 2, "maj7": 2, "maj9": 2, "quartal": 2 },
   "voicings": { "close": 4, "inv1": 3, "inv2": 3, "drop2": 2 },
   "chordRatio": 0.75, "keyTypes": { "note": 3, "octave": 3, "fifth": 2, "stab": 3 },
   "chordRootRange": [48, 59], "keyRange": [36, 72], "velocity": [90, 115],
   "hold": [0.8, 1.8], "stabChance": 0.45, "stabHold": 0.22, "strumMs": 0 },

 { "id": "jazz", "name": "Jazz",
   "description": "ii-V-I vocabulary: drop-2 and shell voicings, altered dominants, m7b5s.",
   "scale": "major",
   "chords": { "m7": 4, "7": 3, "maj7": 4, "m9": 3, "13": 3, "7b9": 2, "7s9": 2, "m7b5": 3, "dim7": 1, "maj9": 3, "69": 2, "7s5": 1 },
   "voicings": { "drop2": 5, "shell": 4, "rootless": 4, "close": 1 },
   "chordRatio": 0.8, "keyTypes": { "note": 3, "octave": 2, "fifth": 1, "stab": 2 },
   "chordRootRange": [43, 57], "keyRange": [40, 76], "velocity": [60, 100],
   "hold": [1.4, 2.4], "stabChance": 0.2, "stabHold": 0.35, "strumMs": 12 },

 { "id": "boom-bap", "name": "Boom Bap",
   "description": "Dusty sample-style chords: minor 7ths, 9ths, 6ths and soulful major 7ths.",
   "scale": "dorian",
   "chords": { "m7": 5, "m9": 4, "maj7": 4, "m6": 2, "6": 2, "9": 3, "min": 2, "7": 2, "m11": 2 },
   "voicings": { "close": 4, "drop2": 3, "inv1": 2, "rootless": 2 },
   "chordRatio": 0.7, "keyTypes": { "note": 4, "octave": 2, "fifth": 1, "stab": 2 },
   "chordRootRange": [45, 57], "keyRange": [38, 74], "velocity": [75, 105],
   "hold": [1.2, 2.2], "stabChance": 0.25, "stabHold": 0.3, "strumMs": 18 },

 { "id": "afrobeats", "name": "Afrobeats",
   "description": "Bright, warm major 7ths, add9s and 6ths with plucky stabs.",
   "scale": "major",
   "chords": { "maj7": 4, "add9": 4, "6": 3, "m7": 4, "maj9": 3, "sus2": 2, "maj": 2, "m9": 2, "69": 2 },
   "voicings": { "close": 4, "inv1": 3, "inv2": 3, "open": 2 },
   "chordRatio": 0.7, "keyTypes": { "note": 4, "octave": 2, "fifth": 1, "stab": 3 },
   "chordRootRange": [48, 60], "keyRange": [43, 79], "velocity": [80, 110],
   "hold": [1.0, 2.0], "stabChance": 0.35, "stabHold": 0.2, "strumMs": 8 },

 { "id": "pop", "name": "Pop",
   "description": "Clean triads, add9s and sus chords in friendly inversions.",
   "scale": "major",
   "chords": { "maj": 5, "min": 5, "add9": 3, "sus2": 3, "sus4": 2, "maj7": 2, "m7": 2, "madd9": 2, "6": 1 },
   "voicings": { "close": 4, "inv1": 3, "inv2": 3, "open": 2, "spread": 1 },
   "chordRatio": 0.7, "keyTypes": { "note": 4, "octave": 2, "fifth": 2, "stab": 2 },
   "chordRootRange": [48, 60], "keyRange": [40, 79], "velocity": [80, 110],
   "hold": [1.2, 2.2], "stabChance": 0.2, "stabHold": 0.25, "strumMs": 6 },

 { "id": "cinematic", "name": "Cinematic",
   "description": "Huge open fifths, sus2, minor add9 and lydian colours spread across the keyboard.",
   "scale": "minor",
   "chords": { "5": 3, "sus2": 4, "madd9": 4, "maj7s11": 3, "min": 3, "maj": 2, "quintal": 2, "mmaj9": 2, "maj9s11": 2 },
   "voicings": { "spread": 5, "open": 4, "close": 1 },
   "chordRatio": 0.75, "keyTypes": { "note": 3, "octave": 4, "fifth": 3, "stab": 1 },
   "chordRootRange": [38, 52], "keyRange": [28, 72], "velocity": [70, 115],
   "hold": [2.5, 4.0], "stabChance": 0.1, "stabHold": 0.4, "strumMs": 10 },

 { "id": "ambient", "name": "Ambient",
   "description": "Slow, floating quartal and suspended clusters with long holds.",
   "scale": "lydian",
   "chords": { "quartal": 4, "quintal": 3, "sus2": 4, "maj9": 3, "maj7s11": 3, "add9": 3, "9sus4": 2, "m9": 2 },
   "voicings": { "spread": 4, "open": 4, "close": 1, "inv1": 1, "inv2": 1, "drop2": 1 },
   "chordRatio": 0.8, "keyTypes": { "note": 3, "octave": 3, "fifth": 3, "stab": 0 },
   "chordRootRange": [40, 55], "keyRange": [36, 84], "velocity": [55, 90],
   "hold": [3.0, 5.0], "stabChance": 0.0, "stabHold": 0.4, "strumMs": 60 },

 { "id": "synthwave", "name": "Synthwave",
   "description": "80s minor triads, add9s and sus chords, punchy and close.",
   "scale": "minor",
   "chords": { "min": 5, "maj": 4, "madd9": 3, "add9": 3, "sus2": 3, "sus4": 2, "maj7": 2, "5": 2 },
   "voicings": { "close": 4, "inv1": 3, "inv2": 2, "open": 2 },
   "chordRatio": 0.7, "keyTypes": { "note": 4, "octave": 3, "fifth": 2, "stab": 2 },
   "chordRootRange": [45, 57], "keyRange": [36, 76], "velocity": [90, 120],
   "hold": [1.2, 2.2], "stabChance": 0.25, "stabHold": 0.25, "strumMs": 0 },

 { "id": "bell-tines", "name": "Bell Tines",
   "description": "16 Levels: glassy FM bells and tines, mostly single notes, short and clean for chromatic riffs.",
   "engine": "bell", "scale": "minor", "maxLength": 2.0,
   "chords": { "5": 3, "sus2": 2, "maj": 1, "min": 1, "add9": 1 },
   "voicings": { "close": 3, "open": 2 },
   "chordRatio": 0.15, "keyTypes": { "note": 7, "octave": 2, "fifth": 1, "stab": 0 },
   "chordRootRange": [60, 71], "keyRange": [55, 84], "velocity": [75, 115],
   "hold": [0.25, 0.7], "stabChance": 0.3, "stabHold": 0.12, "strumMs": 0 },

 { "id": "organ-stabs", "name": "Organ Stabs",
   "description": "16 Levels: drawbar organ stabs and single hits with key click, cut short for rhythmic chops.",
   "engine": "organ", "scale": "dorian", "maxLength": 1.0,
   "chords": { "min": 4, "m7": 4, "maj": 3, "7": 2, "m9": 2, "sus4": 2, "9": 1, "5": 2 },
   "voicings": { "close": 4, "inv1": 3, "inv2": 3 },
   "chordRatio": 0.55, "keyTypes": { "note": 4, "octave": 3, "fifth": 2, "stab": 0 },
   "chordRootRange": [48, 59], "keyRange": [43, 72], "velocity": [95, 120],
   "hold": [0.2, 0.5], "stabChance": 0.7, "stabHold": 0.16, "strumMs": 0 },

 { "id": "plucks", "name": "Plucks",
   "description": "16 Levels: plucked-string transients (Karplus-Strong), bright to muted, tight decays.",
   "engine": "pluck", "scale": "minor", "maxLength": 1.5,
   "chords": { "5": 3, "min": 1, "maj": 1, "sus2": 1 },
   "voicings": { "close": 2, "open": 1 },
   "chordRatio": 0.1, "keyTypes": { "note": 7, "octave": 2, "fifth": 2, "stab": 0 },
   "chordRootRange": [48, 59], "keyRange": [40, 79], "velocity": [80, 120],
   "hold": [0.1, 0.35], "stabChance": 0.3, "stabHold": 0.1, "strumMs": 6 },

 { "id": "choir", "name": "Choir Notes",
   "description": "16 Levels: formant choir 'aah / ooh / oh' notes and small chords; great pitched down on the pads.",
   "engine": "choir", "scale": "minor", "maxLength": 2.5,
   "chords": { "5": 3, "min": 3, "maj": 2, "sus2": 2, "madd9": 1 },
   "voicings": { "close": 2, "open": 3, "spread": 1 },
   "chordRatio": 0.3, "keyTypes": { "note": 6, "octave": 3, "fifth": 2, "stab": 0 },
   "chordRootRange": [48, 59], "keyRange": [45, 74], "velocity": [70, 105],
   "hold": [0.7, 1.5], "stabChance": 0.0, "stabHold": 0.3, "strumMs": 0 },

 { "id": "detuned-synth", "name": "Detuned Synth",
   "description": "16 Levels: detuned supersaw notes and power stabs with a snappy filter envelope.",
   "engine": "supersaw", "scale": "minor", "maxLength": 1.5,
   "chords": { "5": 4, "min": 3, "maj": 2, "sus2": 2, "m7": 1 },
   "voicings": { "close": 3, "open": 2, "inv1": 1 },
   "chordRatio": 0.3, "keyTypes": { "note": 6, "octave": 3, "fifth": 3, "stab": 0 },
   "chordRootRange": [48, 59], "keyRange": [36, 72], "velocity": [90, 120],
   "hold": [0.2, 0.6], "stabChance": 0.5, "stabHold": 0.15, "strumMs": 0 },

 { "id": "one-shot-palette", "name": "One-Shot Palette",
   "description": "16 Levels: a mixed kit of bells, organ, plucks, choir, synth and keys - one sound per pad.",
   "engine": "mixed", "scale": "minor", "maxLength": 1.8,
   "chords": { "5": 3, "min": 3, "maj": 2, "m7": 2, "sus2": 2 },
   "voicings": { "close": 3, "open": 2, "inv1": 1 },
   "chordRatio": 0.2, "keyTypes": { "note": 7, "octave": 2, "fifth": 2, "stab": 0 },
   "chordRootRange": [48, 59], "keyRange": [45, 79], "velocity": [80, 115],
   "hold": [0.2, 0.8], "stabChance": 0.5, "stabHold": 0.14, "strumMs": 0 },

 { "id": "jetlife-ep-bell", "name": "Jet Life EP Bell", "tag": "JetLifeEPBell", "short": "EPBell",
   "description": "Guide recipe 1 (Curren$y / Wiz Khalifa / Larry June): Rhodes tine layered with a soft 2-op sine bell. 350 ms, LP 4.2 kHz, tape saturation, stereo chorus.",
   "engine": "epbell", "root": 60, "scale": "dorian",
   "chords": { "maj7": 3, "m9": 3, "maj9": 1 }, "voicings": { "close": 2, "drop2": 2, "rootless": 1 },
   "chordRatio": 0.3, "keyTypes": { "note": 6, "octave": 2, "fifth": 1, "stab": 0 },
   "chordRootRange": [55, 66], "keyRange": [55, 79], "velocity": [105, 127],
   "hold": [0.25, 0.35], "stabChance": 0.0, "stabHold": 0.3, "strumMs": 8,
   "processing": { "maxLength": 0.35, "attackMs": 0, "ampDecayMs": 300,
                   "fx": [ { "type": "lowpass", "freq": 4200, "poles": 2, "resonance": 0.12 },
                           { "type": "tape", "drive": 0.35 },
                           { "type": "chorus", "rate": 0.8, "depth": 0.25, "mix": 0.5 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Ascending Minor 9th arpeggios across the upper pads over halftime drums." } },

 { "id": "polyester-pluck", "name": "Polyester G-Funk Triangle Pluck", "tag": "PolyesterPluck", "short": "TriPluck",
   "description": "Guide recipe 2 (Polyester the Saint / Wiz Khalifa / Le$): triangle + sub-octave sine, 4-pole LP 2.8 kHz with a 180 ms filter envelope. 280 ms, HP 200 Hz, 1/16 tape delay.",
   "engine": "triangle", "root": 48, "scale": "minorpentatonic",
   "chords": { "5": 1 }, "voicings": { "close": 1 },
   "chordRatio": 0.0, "keyTypes": { "note": 8, "octave": 2, "fifth": 1, "stab": 0 },
   "chordRootRange": [48, 59], "keyRange": [43, 79], "velocity": [110, 127],
   "hold": [0.2, 0.28], "stabChance": 0.0, "stabHold": 0.2, "strumMs": 0,
   "processing": { "maxLength": 0.28, "attackMs": 0, "tempo": 90,
                   "fx": [ { "type": "highpass", "freq": 200, "poles": 2 },
                           { "type": "delay", "sync": "1/16", "feedback": 0.15, "mix": 0.25, "tape": true } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Rapid pentatonic accent runs across pads 8-16; the fast decay keeps runs from smearing." } },

 { "id": "memphis-organ", "name": "90s Memphis Detuned Organ Stab", "tag": "MemphisOrgan", "short": "MemOrgan",
   "description": "Guide recipe 3 (90s Memphis Playa / OG Phonk): transistor combo organ minor stab. 12 ms fade-in, 500 ms, LP 2.4 kHz, cassette wow & flutter 45 %, 12-bit crush.",
   "engine": "comboorgan", "root": 60, "scale": "minor",
   "chords": { "min": 6, "dim": 2, "madd9": 1 }, "voicings": { "close": 3, "inv1": 2, "inv2": 1 },
   "chordRatio": 0.65, "keyTypes": { "note": 5, "octave": 2, "fifth": 1, "stab": 0 },
   "chordRootRange": [52, 63], "keyRange": [48, 72], "velocity": [127, 127],
   "hold": [0.45, 0.5], "stabChance": 0.0, "stabHold": 0.3, "strumMs": 0,
   "processing": { "maxLength": 0.5, "attackMs": 12, "ampDecayMs": 450,
                   "fx": [ { "type": "lowpass", "freq": 2400, "poles": 4 },
                           { "type": "wowflutter", "amount": 0.45 },
                           { "type": "hiss", "levelDb": -46 },
                           { "type": "bitcrush", "bits": 12 } ] },
   "pad": { "tune": -3, "muteGroup": 1, "fullLevel": true, "phrasing": "Pitch the pad down -3 semitones; play simple repetitive 3-to-5 note Natural Minor or Locrian phrases for horror-style motifs." } },

 { "id": "phonk-cowbell", "name": "Rare Phonk Pitch-Shifted Cowbell", "tag": "PhonkCowbell", "short": "Cowbell",
   "description": "Guide recipe 4 (OG Phonk / Rare Phonk / DJ Screw): TR-808 style cowbell hit. 180 ms, band-pass 800 Hz-3.5 kHz, warm overdrive, cassette tape noise.",
   "engine": "cowbell", "root": 79, "scale": "harmonicminor",
   "chords": { "5": 1 }, "voicings": { "close": 1 },
   "chordRatio": 0.0, "keyTypes": { "note": 1, "octave": 0, "fifth": 0, "stab": 0 },
   "chordRootRange": [67, 78], "keyRange": [67, 91], "velocity": [127, 127],
   "hold": [0.1, 0.18], "stabChance": 0.0, "stabHold": 0.1, "strumMs": 0,
   "processing": { "maxLength": 0.18, "attackMs": 0, "ampDecayMs": 150,
                   "fx": [ { "type": "bandpass", "low": 800, "high": 3500, "poles": 2 },
                           { "type": "overdrive", "drive": 0.5, "tone": 5000 },
                           { "type": "hiss", "levelDb": -40 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Syncopated off-beat call-and-response lead patterns; the pitched hit becomes the melody." } },

 { "id": "zaytoven-organ", "name": "Zaytoven Gospel Trap Organ", "tag": "ZaytovenOrgan", "short": "ZayOrgan",
   "description": "Guide recipe 5 (Gucci Mane / Young Jeezy): workstation drawbar organ key strike with key click. 320 ms, HP 150 Hz, +3 dB at 1.8 kHz, fast rotary, bright plate (0.6 s, 15 %).",
   "engine": "organ", "root": 60, "scale": "minor",
   "chords": { "min": 2, "m7": 1 }, "voicings": { "close": 1 },
   "chordRatio": 0.1, "keyTypes": { "note": 8, "octave": 2, "fifth": 1, "stab": 0 },
   "chordRootRange": [55, 66], "keyRange": [48, 76], "velocity": [127, 127],
   "hold": [0.3, 0.32], "stabChance": 0.0, "stabHold": 0.2, "strumMs": 0,
   "processing": { "maxLength": 0.32, "attackMs": 0, "ampDecayMs": 300, "fxTailMs": 120,
                   "fx": [ { "type": "highpass", "freq": 150, "poles": 2 },
                           { "type": "peak", "freq": 1800, "gainDb": 3, "q": 1.0 },
                           { "type": "rotary", "speed": "fast" },
                           { "type": "reverb", "kind": "plate", "decay": 0.6, "mix": 0.15 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Fast chromatic grace notes resolving upward into minor-scale target notes, gospel-organ style over trap drums." } },

 { "id": "luxury-harp", "name": "Luxury Ross Cinematic Harp", "tag": "LuxuryHarp", "short": "Harp",
   "description": "Guide recipe 6 (Luxury Rick Ross / Young Jeezy): concert harp pluck. 450 ms, HP 300 Hz, stereo expander, +12 st shimmer delay at a low mix.",
   "engine": "harp", "root": 60, "scale": "minor",
   "chords": { "5": 1 }, "voicings": { "close": 1 },
   "chordRatio": 0.0, "keyTypes": { "note": 8, "octave": 2, "fifth": 1, "stab": 0 },
   "chordRootRange": [55, 66], "keyRange": [48, 84], "velocity": [110, 127],
   "hold": [0.3, 0.45], "stabChance": 0.0, "stabHold": 0.2, "strumMs": 0,
   "processing": { "maxLength": 0.45, "attackMs": 0, "ampDecayMs": 400, "fxTailMs": 150,
                   "fx": [ { "type": "highpass", "freq": 300, "poles": 2 },
                           { "type": "widen", "width": 1.8 },
                           { "type": "shimmer", "timeMs": 160, "semitones": 12, "feedback": 0.35, "mix": 0.15 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Rapid ascending scale cascades sweeping pads 1 through 16." } },

 { "id": "screwed-choir", "name": "Screw-Head Down-Pitched Choir", "tag": "ScrewedChoir", "short": "Choir",
   "description": "Guide recipe 7 (DJ Screw / 90s Memphis Playa): choir chord stab varispeeded -5 st, 25 ms fade-in, 600 ms, 24 dB/oct LP 2.8 kHz, dark hall (2.2 s, 30 %), tape saturation.",
   "engine": "choir", "root": 60, "scale": "minor",
   "chords": { "min": 5, "5": 2, "maj": 2, "madd9": 1 }, "voicings": { "close": 2, "open": 2 },
   "chordRatio": 0.75, "keyTypes": { "note": 5, "octave": 3, "fifth": 2, "stab": 0 },
   "chordRootRange": [55, 66], "keyRange": [52, 72], "velocity": [100, 120],
   "hold": [0.75, 0.9], "stabChance": 0.0, "stabHold": 0.3, "strumMs": 0,
   "processing": { "maxLength": 0.6, "attackMs": 25, "ampDecayMs": 550, "fxTailMs": 250, "pitchShift": -5,
                   "fx": [ { "type": "lowpass", "freq": 2800, "poles": 4 },
                           { "type": "reverb", "kind": "darkhall", "decay": 2.2, "mix": 0.3 },
                           { "type": "tape", "drive": 0.3 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Lower pad registers: slow, atmospheric halftime chords." } },

 { "id": "trap-brass", "name": "Trap Majesty Orchestral Brass", "tag": "TrapBrass", "short": "Brass",
   "description": "Guide recipe 8 (Young Jeezy / Gucci Mane / Luxury Rick Ross): horn section + bright 80s analog brass. 300 ms, +4 dB at 1.2 kHz, fast 4:1 compression, short room.",
   "engine": "brass", "root": 60, "scale": "minor",
   "chords": { "5": 3, "min": 2, "maj": 1 }, "voicings": { "close": 2, "open": 1 },
   "chordRatio": 0.25, "keyTypes": { "note": 7, "octave": 3, "fifth": 2, "stab": 0 },
   "chordRootRange": [50, 61], "keyRange": [45, 72], "velocity": [127, 127],
   "hold": [0.28, 0.3], "stabChance": 0.0, "stabHold": 0.2, "strumMs": 0,
   "processing": { "maxLength": 0.3, "attackMs": 0, "ampDecayMs": 280, "fxTailMs": 100,
                   "fx": [ { "type": "peak", "freq": 1200, "gainDb": 4, "q": 1.0 },
                           { "type": "compressor", "thresholdDb": -18, "ratio": 4, "attackMs": 1, "releaseMs": 60 },
                           { "type": "reverb", "kind": "room", "decay": 0.5, "mix": 0.12 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Staccato single-note accents on beats 1 and 3 for a brass-driven foundation." } },

 { "id": "rhodes-bass", "name": "Cruisin' Smooth Rhodes Bass/Key", "tag": "RhodesBass", "short": "RhBass",
   "description": "Guide recipe 9 (Curren$y / Larry June / Le$): low-register Rhodes note (E1) through a tube preamp. 500 ms, LP 1.8 kHz, gentle tube overdrive.",
   "engine": "epiano", "root": 28, "scale": "dorian",
   "chords": { "5": 1 }, "voicings": { "close": 1 },
   "chordRatio": 0.0, "keyTypes": { "note": 8, "octave": 2, "fifth": 1, "stab": 0 },
   "chordRootRange": [28, 39], "keyRange": [28, 52], "velocity": [100, 127],
   "hold": [0.45, 0.5], "stabChance": 0.0, "stabHold": 0.3, "strumMs": 0,
   "processing": { "maxLength": 0.5, "attackMs": 0, "ampDecayMs": 480,
                   "fx": [ { "type": "lowpass", "freq": 1800, "poles": 2 },
                           { "type": "tube", "drive": 0.45 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Lower pads: walking basslines and smooth counter-melodies - bass tone and key element at once." } },

 { "id": "westcoast-kit", "name": "West Coast / Jet Life Kit", "tag": "WestCoast", "short": "WestCst",
   "description": "Kit: Jet Life EP Bell + Polyester Pluck + Rhodes Bass (smooth analog, warm tines, Major 7th / Minor 9th / Dorian).",
   "kit": { "jetlife-ep-bell": 3, "polyester-pluck": 3, "rhodes-bass": 2 } },

 { "id": "memphis-kit", "name": "Memphis / OG Phonk Kit", "tag": "Memphis", "short": "Memphis",
   "description": "Kit: Memphis Organ + Phonk Cowbell + Screwed Choir (cassette loss, wow & flutter, Harmonic Minor / Locrian).",
   "kit": { "memphis-organ": 3, "phonk-cowbell": 3, "screwed-choir": 2 } },

 { "id": "atlanta-kit", "name": "Atlanta / Luxury Trap Kit", "tag": "Atlanta", "short": "Atlanta",
   "description": "Kit: Zaytoven Organ + Luxury Harp + Trap Brass (workstation organs, harp plucks, brass hits, Natural Minor).",
   "kit": { "zaytoven-organ": 3, "luxury-harp": 3, "trap-brass": 2 } },

 { "id": "guide-palette", "name": "Recipe Guide Palette (all 9)", "tag": "GuidePalette", "short": "Guide",
   "description": "Kit: all nine guide recipes for puzzle-piece layering (e.g. Rhodes Bass on pad 1, EP Bell on pad 2, Polyester Pluck on pad 3).",
   "kit": { "rhodes-bass": 1, "jetlife-ep-bell": 1, "polyester-pluck": 1, "memphis-organ": 1, "phonk-cowbell": 1,
            "zaytoven-organ": 1, "luxury-harp": 1, "screwed-choir": 1, "trap-brass": 1 } },

 { "id": "lead-synth", "name": "Lead Instrument", "tag": "LeadSynth", "short": "Lead",
   "description": "Mono synth lead (saw + pulse, filter blip, delayed vibrato) as single notes; scale banks give a playable lead across 16 pads.",
   "engine": "lead", "root": 72, "scale": "minor", "scaleBank": "notes",
   "chords": { "5": 1 }, "voicings": { "close": 1 }, "chordRatio": 0.0,
   "keyTypes": { "note": 8, "octave": 1, "fifth": 0, "stab": 0 },
   "chordRootRange": [60, 71], "keyRange": [60, 84], "velocity": [100, 120], "hold": [0.4, 0.7], "stabChance": 0, "stabHold": 0.2, "strumMs": 0,
   "processing": { "maxLength": 0.9, "fxTailMs": 200,
                   "fx": [ { "type": "lowpass", "freq": 7000, "poles": 2 },
                           { "type": "chorus", "rate": 0.6, "depth": 0.2, "mix": 0.3 },
                           { "type": "delay", "sync": "1/8d", "feedback": 0.25, "mix": 0.18 },
                           { "type": "reverb", "kind": "plate", "decay": 0.8, "mix": 0.12 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Lead lines straight across the scale bank; the Mute Group keeps it mono like a real lead synth." } },

 { "id": "gfunk-lead", "name": "Funky Lead (G-Funk Whistle)", "tag": "FunkyLead", "short": "FunkLead",
   "description": "The West Coast whistle: a near-sine Moog lead that scoops up into each note and sings with vibrato. Minor pentatonic, high register.",
   "engine": "whistle", "root": 79, "scale": "minorpentatonic", "scaleBank": "notes",
   "chords": { "5": 1 }, "voicings": { "close": 1 }, "chordRatio": 0.0,
   "keyTypes": { "note": 8, "octave": 1, "fifth": 0, "stab": 0 },
   "chordRootRange": [72, 83], "keyRange": [72, 93], "velocity": [100, 115], "hold": [0.5, 0.8], "stabChance": 0, "stabHold": 0.2, "strumMs": 0,
   "processing": { "maxLength": 1.0, "fxTailMs": 250,
                   "fx": [ { "type": "chorus", "rate": 0.5, "depth": 0.25, "mix": 0.35 },
                           { "type": "tape", "drive": 0.25 },
                           { "type": "reverb", "kind": "plate", "decay": 1.2, "mix": 0.18 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "High, sliding minor-pentatonic phrases over the bass - the classic G-funk whistle." } },

 { "id": "bassline", "name": "Bassline", "tag": "Bassline", "short": "Bass",
   "description": "Round bass (sine body + filtered saw edge), tight and dry for basslines that sit under 808s and kicks.",
   "engine": "bass", "root": 36, "scale": "minor", "scaleBank": "notes",
   "chords": { "5": 1 }, "voicings": { "close": 1 }, "chordRatio": 0.0,
   "keyTypes": { "note": 8, "octave": 2, "fifth": 0, "stab": 0 },
   "chordRootRange": [28, 39], "keyRange": [28, 52], "velocity": [105, 120], "hold": [0.35, 0.5], "stabChance": 0, "stabHold": 0.2, "strumMs": 0,
   "processing": { "maxLength": 0.6,
                   "fx": [ { "type": "lowpass", "freq": 2500, "poles": 2 }, { "type": "tube", "drive": 0.3 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Walk the bass across pads 1-8; the Mute Group stops notes from overlapping." } },

 { "id": "moog-bass", "name": "Moog / G-Funk Bass", "tag": "MoogBass", "short": "MoogBass",
   "description": "Saw + square through a resonant 4-pole filter with a snappy envelope and drive: the bouncy G-funk synth bass.",
   "engine": "moogbass", "root": 33, "scale": "dorian", "scaleBank": "notes",
   "chords": { "5": 1 }, "voicings": { "close": 1 }, "chordRatio": 0.0,
   "keyTypes": { "note": 8, "octave": 3, "fifth": 0, "stab": 0 },
   "chordRootRange": [26, 37], "keyRange": [26, 48], "velocity": [110, 127], "hold": [0.25, 0.45], "stabChance": 0, "stabHold": 0.2, "strumMs": 0,
   "processing": { "maxLength": 0.5,
                   "fx": [ { "type": "overdrive", "drive": 0.25, "tone": 3500 }, { "type": "lowpass", "freq": 3500, "poles": 2 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Bouncy lines with octave jumps (pad 1 to pad 8 is an octave in a 7-note scale), G-funk style." } },

 { "id": "soul-chords", "name": "Soul Chords", "tag": "SoulChords", "short": "Soul",
   "description": "Warm Rhodes soul chords: maj7, m7, m9, dominant 9ths and 6/9s with gentle rolls and tape warmth.",
   "engine": "epiano", "root": 60, "scale": "major", "scaleBank": "chords",
   "chords": { "maj7": 4, "m7": 4, "m9": 3, "9": 2, "69": 2, "7": 2, "maj9": 2 },
   "voicings": { "close": 2, "drop2": 3, "rootless": 2, "inv1": 1 }, "chordRatio": 0.85,
   "keyTypes": { "note": 4, "octave": 1, "fifth": 0, "stab": 0 },
   "chordRootRange": [48, 59], "keyRange": [48, 72], "velocity": [80, 105], "hold": [1.0, 1.6], "stabChance": 0.1, "stabHold": 0.3, "strumMs": 20,
   "processing": { "maxLength": 1.8,
                   "fx": [ { "type": "tape", "drive": 0.3 }, { "type": "lowpass", "freq": 6500, "poles": 2 },
                           { "type": "chorus", "rate": 0.4, "depth": 0.15, "mix": 0.2 } ] },
   "pad": { "muteGroup": 0, "fullLevel": true, "phrasing": "Chord banks: pads 1-7 are the scale's chords (I ii iii IV V vi vii), pads 8-14 the same chords re-voiced." } },

 { "id": "ninth-chords", "name": "9th Chords", "tag": "NinthChords", "short": "Ninth",
   "description": "Nothing but 9ths: maj9, m9, dominant 9, 9sus4, 6/9 and minor-major 9, mostly rootless and drop-2.",
   "engine": "epiano", "root": 60, "scale": "major", "scaleBank": "chords",
   "chords": { "maj9": 4, "m9": 4, "9": 3, "9sus4": 2, "69": 2, "mmaj9": 1 },
   "voicings": { "rootless": 4, "drop2": 3, "spread": 1 }, "chordRatio": 1.0,
   "keyTypes": { "note": 1, "octave": 0, "fifth": 0, "stab": 0 },
   "chordRootRange": [45, 56], "keyRange": [45, 72], "velocity": [80, 105], "hold": [1.2, 1.8], "stabChance": 0.05, "stabHold": 0.3, "strumMs": 25,
   "processing": { "maxLength": 2.0, "fxTailMs": 150,
                   "fx": [ { "type": "lowpass", "freq": 7000, "poles": 2 }, { "type": "reverb", "kind": "plate", "decay": 0.8, "mix": 0.1 } ] },
   "pad": { "muteGroup": 0, "fullLevel": true, "phrasing": "Every pad is a 9th chord of the key: try ii-V-I (pads 2, 5, 1) or vi-ii-V-I." } },

 { "id": "scale-notes", "name": "Each Note in Scale", "tag": "ScaleNotes", "short": "Notes",
   "description": "Always a scale bank: one clean key sound per bank, pads 1-16 climbing the scale - melodies without 16 Levels.",
   "engine": "epiano", "root": 60, "scale": "minor", "scaleBank": "notes", "scaleBankOnly": true,
   "chords": { "5": 1 }, "voicings": { "close": 1 }, "chordRatio": 0.0,
   "keyTypes": { "note": 1, "octave": 0, "fifth": 0, "stab": 0 },
   "chordRootRange": [48, 59], "keyRange": [48, 72], "velocity": [100, 110], "hold": [0.5, 0.7], "stabChance": 0, "stabHold": 0.2, "strumMs": 0,
   "processing": { "maxLength": 1.0 },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Pads 1-16 climb the scale: play melodies directly, every pad in key." } },

 { "id": "screw-rnb", "name": "DJ Screw R&B Chords", "tag": "ScrewRnB", "short": "ScrewRnB",
   "description": "Slowed, syrupy R&B chords: minor 7/9/11 and maj7s on Rhodes, varispeeded -4 st, dark filter, tape wobble and a dark hall.",
   "engine": "epiano", "root": 60, "scale": "minor", "scaleBank": "chords",
   "chords": { "m7": 4, "m9": 4, "maj7": 3, "m11": 2, "9sus4": 2, "maj9": 2 },
   "voicings": { "rootless": 3, "drop2": 3, "spread": 2 }, "chordRatio": 0.85,
   "keyTypes": { "note": 4, "octave": 2, "fifth": 0, "stab": 0 },
   "chordRootRange": [48, 59], "keyRange": [48, 72], "velocity": [70, 95], "hold": [1.4, 2.0], "stabChance": 0, "stabHold": 0.3, "strumMs": 30,
   "processing": { "maxLength": 2.4, "attackMs": 15, "pitchShift": -4, "fxTailMs": 300,
                   "fx": [ { "type": "lowpass", "freq": 3000, "poles": 4 }, { "type": "wowflutter", "amount": 0.3 },
                           { "type": "tape", "drive": 0.35 }, { "type": "chorus", "rate": 0.4, "depth": 0.3, "mix": 0.3 },
                           { "type": "reverb", "kind": "darkhall", "decay": 2.0, "mix": 0.25 } ] },
   "pad": { "muteGroup": 0, "fullLevel": true, "phrasing": "Slow R&B progressions; resample and use Half Speed (Shift + Pad 6) for the full chopped & screwed drag." } },

 { "id": "trap-scale", "name": "Trap Scale", "tag": "TrapScale", "short": "TrapScl",
   "description": "Always a scale bank: dark harmonic-minor bell melodies for trap, high-passed with a short reverb.",
   "engine": "bell", "root": 72, "scale": "harmonicminor", "scaleBank": "notes", "scaleBankOnly": true,
   "chords": { "5": 1 }, "voicings": { "close": 1 }, "chordRatio": 0.0,
   "keyTypes": { "note": 1, "octave": 0, "fifth": 0, "stab": 0 },
   "chordRootRange": [60, 71], "keyRange": [60, 84], "velocity": [100, 120], "hold": [0.25, 0.4], "stabChance": 0, "stabHold": 0.2, "strumMs": 0,
   "processing": { "maxLength": 0.8, "fxTailMs": 200,
                   "fx": [ { "type": "highpass", "freq": 200, "poles": 2 }, { "type": "reverb", "kind": "plate", "decay": 0.9, "mix": 0.2 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Dark melodies over 808s; the raised 7th (pad 7) gives harmonic minor its trap tension." } },

 { "id": "gospel-rnb-90s", "name": "90s Gospel R&B Chords", "tag": "GospelRnB", "short": "GospelRB",
   "description": "90s gospel-flavoured R&B: maj9, 13, m11, 9sus4, 7#9 and 6/9 chords, spread and rootless on Rhodes.",
   "engine": "epiano", "root": 60, "scale": "major", "scaleBank": "chords",
   "chords": { "maj9": 4, "13": 3, "m11": 3, "9sus4": 3, "7s9": 2, "69": 2, "maj13": 2, "m9": 3 },
   "voicings": { "spread": 3, "rootless": 3, "drop2": 2 }, "chordRatio": 0.85,
   "keyTypes": { "note": 3, "octave": 2, "fifth": 0, "stab": 0 },
   "chordRootRange": [45, 57], "keyRange": [45, 72], "velocity": [85, 110], "hold": [1.2, 1.8], "stabChance": 0.1, "stabHold": 0.3, "strumMs": 25,
   "processing": { "maxLength": 2.0, "fxTailMs": 200,
                   "fx": [ { "type": "tape", "drive": 0.2 }, { "type": "reverb", "kind": "plate", "decay": 1.0, "mix": 0.15 } ] },
   "pad": { "muteGroup": 0, "fullLevel": true, "phrasing": "Classic moves: IV-iii-ii-I and ii-V-I with 9sus4 turnarounds." } },

 { "id": "vaporwave", "name": "Vaporwave Chords", "tag": "Vaporwave", "short": "Vapor",
   "description": "Slowed 80s DX-style electric piano: maj7/maj9/m7/6-9, varispeeded -3 st, thick chorus, tape wobble and a big hall.",
   "engine": "epbell", "root": 60, "scale": "major", "scaleBank": "chords",
   "chords": { "maj7": 4, "maj9": 4, "m7": 3, "m9": 2, "add9": 2, "69": 2 },
   "voicings": { "close": 2, "drop2": 2, "open": 2 }, "chordRatio": 0.85,
   "keyTypes": { "note": 3, "octave": 2, "fifth": 0, "stab": 0 },
   "chordRootRange": [50, 61], "keyRange": [50, 74], "velocity": [75, 100], "hold": [1.6, 2.2], "stabChance": 0, "stabHold": 0.3, "strumMs": 15,
   "processing": { "maxLength": 2.6, "attackMs": 10, "pitchShift": -3, "fxTailMs": 400,
                   "fx": [ { "type": "lowpass", "freq": 6000, "poles": 2 }, { "type": "chorus", "rate": 0.35, "depth": 0.5, "mix": 0.5 },
                           { "type": "wowflutter", "amount": 0.3 }, { "type": "reverb", "kind": "hall", "decay": 2.5, "mix": 0.35 } ] },
   "pad": { "muteGroup": 0, "fullLevel": true, "phrasing": "Slow maj7 progressions (IV-iii-ii-I); let the hall ring between hits." } },

 { "id": "chillwave", "name": "Chillwave Chords", "tag": "Chillwave", "short": "Chillw",
   "description": "Hazy synth chords: maj7, add9, sus2, m7 and maj9 on a soft detuned synth with tape saturation, wobble and hall.",
   "engine": "supersaw", "root": 60, "scale": "major", "scaleBank": "chords",
   "chords": { "maj7": 4, "add9": 3, "sus2": 3, "m7": 3, "maj9": 3 },
   "voicings": { "open": 3, "spread": 2, "close": 1 }, "chordRatio": 0.85,
   "keyTypes": { "note": 3, "octave": 2, "fifth": 0, "stab": 0 },
   "chordRootRange": [50, 61], "keyRange": [50, 74], "velocity": [70, 95], "hold": [1.8, 2.4], "stabChance": 0, "stabHold": 0.3, "strumMs": 0,
   "processing": { "maxLength": 2.8, "attackMs": 40, "fxTailMs": 400,
                   "fx": [ { "type": "lowpass", "freq": 4000, "poles": 2 }, { "type": "tape", "drive": 0.3 },
                           { "type": "wowflutter", "amount": 0.25 }, { "type": "chorus", "rate": 0.3, "depth": 0.4, "mix": 0.4 },
                           { "type": "reverb", "kind": "hall", "decay": 2.2, "mix": 0.3 } ] },
   "pad": { "muteGroup": 0, "fullLevel": true, "phrasing": "Long, washy chords: let them overlap (no Mute Group) for a dreamy bed." } },

 { "id": "phonkwave", "name": "Phonkwave Chords", "tag": "Phonkwave", "short": "PhonkW",
   "description": "Phonk meets vaporwave: eerie phrygian minor chords on keys, varispeeded -3 st, 12-bit, wow & flutter, cassette hiss.",
   "engine": "epiano", "root": 60, "scale": "phrygian", "scaleBank": "chords",
   "chords": { "m7": 4, "min": 3, "m9": 3, "mmaj7": 2, "dim": 2, "m7b5": 2 },
   "voicings": { "close": 3, "inv1": 2, "open": 2 }, "chordRatio": 0.85,
   "keyTypes": { "note": 3, "octave": 2, "fifth": 0, "stab": 0 },
   "chordRootRange": [48, 59], "keyRange": [48, 72], "velocity": [90, 110], "hold": [1.0, 1.5], "stabChance": 0.1, "stabHold": 0.25, "strumMs": 0,
   "processing": { "maxLength": 1.6, "pitchShift": -3, "fxTailMs": 250,
                   "fx": [ { "type": "lowpass", "freq": 3500, "poles": 4 }, { "type": "bitcrush", "bits": 12 },
                           { "type": "wowflutter", "amount": 0.4 }, { "type": "hiss", "levelDb": -44 },
                           { "type": "reverb", "kind": "room", "decay": 0.8, "mix": 0.2 } ] },
   "pad": { "muteGroup": 0, "fullLevel": true, "phrasing": "Eerie phrygian progressions (i-II is the classic phonk move); pitch the pad down for more drift." } },

 { "id": "808-bass", "name": "808 Bass", "tag": "Bass808", "short": "808",
   "description": "Tuned 808 bass: sine boom with a short pitch punch and drive. As a scale bank: 16 tuned 808s in your key.",
   "engine": "808", "root": 36, "scale": "minor", "scaleBank": "notes",
   "chords": { "5": 1 }, "voicings": { "close": 1 }, "chordRatio": 0.0,
   "keyTypes": { "note": 1, "octave": 0, "fifth": 0, "stab": 0 },
   "chordRootRange": [24, 35], "keyRange": [24, 48], "velocity": [120, 127], "hold": [0.8, 1.2], "stabChance": 0, "stabHold": 0.2, "strumMs": 0,
   "processing": { "maxLength": 1.6, "fx": [ { "type": "tube", "drive": 0.35 }, { "type": "lowpass", "freq": 5000, "poles": 2 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Mute Group 1 so each 808 cuts the last; play the bassline across the bank, octave = 7 pads up." } },

 { "id": "clav-funk", "name": "Clavinet Funk", "tag": "ClavFunk", "short": "Clav",
   "description": "Funky clavinet: narrow-pulse pluck with a bright filter snap. Short stabs and single notes for G-funk / West Coast.",
   "engine": "clav", "root": 60, "scale": "dorian", "scaleBank": "chords",
   "chords": { "m7": 4, "7": 3, "9": 3, "min": 2, "7s9": 2, "m9": 2 },
   "voicings": { "close": 3, "inv1": 2, "inv2": 2 }, "chordRatio": 0.6,
   "keyTypes": { "note": 6, "octave": 3, "fifth": 0, "stab": 0 },
   "chordRootRange": [52, 63], "keyRange": [48, 76], "velocity": [100, 127], "hold": [0.12, 0.3], "stabChance": 0.7, "stabHold": 0.12, "strumMs": 0,
   "processing": { "maxLength": 0.5, "fx": [ { "type": "highpass", "freq": 120 }, { "type": "peak", "freq": 1500, "gainDb": 3 } ] },
   "pad": { "muteGroup": 0, "fullLevel": true, "phrasing": "Syncopated 16th-note stabs; ghost the off-beats with lower velocity." } },

 { "id": "talkbox-lead", "name": "Talkbox Lead", "tag": "Talkbox", "short": "Talkbox",
   "description": "Talkbox / vocoder-style lead: a synth that 'says' oh-ah through moving vowel formants, with vibrato. Zapp & Roger vibes.",
   "engine": "talkbox", "root": 67, "scale": "minorpentatonic", "scaleBank": "notes",
   "chords": { "5": 1 }, "voicings": { "close": 1 }, "chordRatio": 0.0,
   "keyTypes": { "note": 8, "octave": 1, "fifth": 0, "stab": 0 },
   "chordRootRange": [55, 66], "keyRange": [55, 79], "velocity": [100, 120], "hold": [0.5, 0.9], "stabChance": 0, "stabHold": 0.2, "strumMs": 0,
   "processing": { "maxLength": 1.1, "fxTailMs": 200,
                   "fx": [ { "type": "chorus", "rate": 0.5, "depth": 0.2, "mix": 0.3 }, { "type": "reverb", "kind": "plate", "decay": 1.0, "mix": 0.15 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Hook lines answering the vocal; slide between neighbouring pads." } },

 { "id": "wurli-keys", "name": "Wurlitzer Keys", "tag": "WurliKeys", "short": "Wurli",
   "description": "Wurlitzer electric piano: reedy, barks when played hard, optional tremolo. Lo-fi, soul and boom bap chords.",
   "engine": "wurli", "root": 60, "scale": "dorian", "scaleBank": "chords",
   "chords": { "m7": 4, "maj7": 3, "9": 2, "69": 2, "m9": 3, "7": 2 },
   "voicings": { "close": 3, "drop2": 2, "inv1": 2 }, "chordRatio": 0.75,
   "keyTypes": { "note": 4, "octave": 2, "fifth": 0, "stab": 0 },
   "chordRootRange": [50, 61], "keyRange": [48, 74], "velocity": [85, 115], "hold": [1.0, 1.6], "stabChance": 0.2, "stabHold": 0.25, "strumMs": 15,
   "processing": { "maxLength": 1.8, "fx": [ { "type": "tape", "drive": 0.3 }, { "type": "lowpass", "freq": 6000 } ] },
   "pad": { "muteGroup": 0, "fullLevel": false, "phrasing": "Lay back on the beat; leave Full Level off so harder hits bark more." } },

 { "id": "horror-musicbox", "name": "Horror Music Box", "tag": "MusicBox", "short": "MusicBox",
   "description": "Eerie music box / celesta in harmonic minor, high register, wobbling tape and a big dark space. Memphis / phonk horror.",
   "engine": "musicbox", "root": 79, "scale": "harmonicminor", "scaleBank": "notes",
   "chords": { "5": 1 }, "voicings": { "close": 1 }, "chordRatio": 0.0,
   "keyTypes": { "note": 1, "octave": 0, "fifth": 0, "stab": 0 },
   "chordRootRange": [72, 83], "keyRange": [72, 96], "velocity": [90, 115], "hold": [0.4, 0.6], "stabChance": 0, "stabHold": 0.2, "strumMs": 0,
   "processing": { "maxLength": 1.2, "fxTailMs": 400,
                   "fx": [ { "type": "wowflutter", "amount": 0.25 }, { "type": "reverb", "kind": "darkhall", "decay": 2.2, "mix": 0.3 } ] },
   "pad": { "muteGroup": 0, "fullLevel": true, "phrasing": "Slow, simple 3-5 note motifs; let notes ring into the reverb." } },

 { "id": "pizz-strings", "name": "Pizzicato Strings", "tag": "PizzStrings", "short": "Pizz",
   "description": "Plucked orchestral strings for cinematic luxury-trap runs: short, dark pluck with a hall.",
   "engine": "pizz", "root": 67, "scale": "minor", "scaleBank": "notes",
   "chords": { "5": 1 }, "voicings": { "close": 1 }, "chordRatio": 0.0,
   "keyTypes": { "note": 6, "octave": 2, "fifth": 0, "stab": 0 },
   "chordRootRange": [55, 66], "keyRange": [55, 84], "velocity": [100, 120], "hold": [0.2, 0.3], "stabChance": 0, "stabHold": 0.2, "strumMs": 0,
   "processing": { "maxLength": 0.6, "fxTailMs": 250,
                   "fx": [ { "type": "highpass", "freq": 150 }, { "type": "reverb", "kind": "hall", "decay": 1.6, "mix": 0.2 } ] },
   "pad": { "muteGroup": 0, "fullLevel": true, "phrasing": "Fast ascending/descending runs across the bank, like the luxury trap string sweeps." } },

 { "id": "mellotron-flute", "name": "Mellotron Flute", "tag": "MelloFlute", "short": "Flute",
   "description": "Tape-replay flute: soft tone, breathy chiff, wow and flutter. The dusty flute of boom bap and Memphis flips.",
   "engine": "flute", "root": 72, "scale": "dorian", "scaleBank": "notes",
   "chords": { "min": 2, "maj": 1, "sus2": 1 }, "voicings": { "close": 1 }, "chordRatio": 0.15,
   "keyTypes": { "note": 6, "octave": 1, "fifth": 1, "stab": 0 },
   "chordRootRange": [60, 71], "keyRange": [60, 84], "velocity": [90, 110], "hold": [0.6, 1.0], "stabChance": 0, "stabHold": 0.2, "strumMs": 0,
   "processing": { "maxLength": 1.2, "attackMs": 10,
                   "fx": [ { "type": "wowflutter", "amount": 0.35 }, { "type": "tape", "drive": 0.3 }, { "type": "lowpass", "freq": 6000 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Lazy melodies over boom bap drums; resample and pitch down for darker flips." } },

 { "id": "log-drum", "name": "Amapiano Log Drum", "tag": "LogDrum", "short": "LogDrum",
   "description": "Pitched amapiano log drum: punchy, woody, saturated. Tuned per pad as a scale bank.",
   "engine": "logdrum", "root": 43, "scale": "minor", "scaleBank": "notes",
   "chords": { "5": 1 }, "voicings": { "close": 1 }, "chordRatio": 0.0,
   "keyTypes": { "note": 1, "octave": 0, "fifth": 0, "stab": 0 },
   "chordRootRange": [36, 47], "keyRange": [36, 58], "velocity": [110, 127], "hold": [0.3, 0.5], "stabChance": 0, "stabHold": 0.2, "strumMs": 0,
   "processing": { "maxLength": 0.8, "fx": [ { "type": "overdrive", "drive": 0.2, "tone": 4000 } ] },
   "pad": { "muteGroup": 1, "fullLevel": false, "phrasing": "Syncopated log-drum basslines; leave Full Level off for accents." } },

 { "id": "smooth-jazz-chords", "name": "Smooth Jazz Chords", "tag": "SmoothJazz", "short": "SmJazz",
   "description": "Smooth jazz keys: maj9, m11, 13, 9sus4, maj7#11 and 6/9 on chorused Rhodes, rootless and drop-2.",
   "engine": "epiano", "root": 60, "scale": "major", "scaleBank": "chords",
   "chords": { "maj9": 4, "m11": 3, "13": 3, "9sus4": 3, "maj7s11": 2, "69": 3, "m9": 3 },
   "voicings": { "rootless": 4, "drop2": 3, "spread": 1 }, "chordRatio": 0.85,
   "keyTypes": { "note": 3, "octave": 1, "fifth": 0, "stab": 0 },
   "chordRootRange": [45, 57], "keyRange": [48, 76], "velocity": [75, 100], "hold": [1.4, 2.0], "stabChance": 0.05, "stabHold": 0.3, "strumMs": 20,
   "processing": { "maxLength": 2.2, "fxTailMs": 250,
                   "fx": [ { "type": "chorus", "rate": 0.6, "depth": 0.3, "mix": 0.35 }, { "type": "reverb", "kind": "plate", "decay": 1.4, "mix": 0.18 } ] },
   "pad": { "muteGroup": 0, "fullLevel": true, "phrasing": "ii-V-I and IV-iii-ii-I moves; let chords ring into each other." } },

 { "id": "rnb-80s-chords", "name": "1980s R&B Chords", "tag": "RnB80s", "short": "RnB80",
   "description": "80s R&B / quiet storm: maj9, m9, 9sus4, 7sus4 and add9 on a DX-style electric piano with chorus and a bright plate.",
   "engine": "epbell", "root": 60, "scale": "major", "scaleBank": "chords",
   "chords": { "maj9": 4, "m9": 4, "9sus4": 3, "7sus4": 2, "maj7": 3, "add9": 2, "m7": 2 },
   "voicings": { "close": 3, "drop2": 3, "inv1": 1 }, "chordRatio": 0.85,
   "keyTypes": { "note": 3, "octave": 1, "fifth": 0, "stab": 0 },
   "chordRootRange": [50, 61], "keyRange": [50, 76], "velocity": [80, 105], "hold": [1.2, 1.8], "stabChance": 0.15, "stabHold": 0.25, "strumMs": 10,
   "processing": { "maxLength": 2.0, "fxTailMs": 250,
                   "fx": [ { "type": "chorus", "rate": 0.8, "depth": 0.35, "mix": 0.4 }, { "type": "reverb", "kind": "plate", "decay": 1.2, "mix": 0.2 } ] },
   "pad": { "muteGroup": 0, "fullLevel": true, "phrasing": "9sus4 to maj9 resolutions; stack with a bass bank for the full quiet-storm feel." } },

 { "id": "smooth-jazz-sax", "name": "Smooth Jazz Sax", "tag": "JazzSax", "short": "Sax",
   "description": "Smooth jazz alto sax: breathy horn tone that scoops into the note, then sings with vibrato. Dorian, mid register.",
   "engine": "sax", "root": 67, "scale": "dorian", "scaleBank": "notes",
   "chords": { "5": 1 }, "voicings": { "close": 1 }, "chordRatio": 0.0,
   "keyTypes": { "note": 8, "octave": 1, "fifth": 0, "stab": 0 },
   "chordRootRange": [55, 66], "keyRange": [55, 79], "velocity": [95, 115], "hold": [0.7, 1.1], "stabChance": 0, "stabHold": 0.2, "strumMs": 0,
   "processing": { "maxLength": 1.4, "fxTailMs": 300,
                   "fx": [ { "type": "peak", "freq": 1800, "gainDb": 2 }, { "type": "reverb", "kind": "plate", "decay": 1.6, "mix": 0.2 } ] },
   "pad": { "muteGroup": 1, "fullLevel": false, "phrasing": "Short phrases with space between them; Mute Group keeps it one horn." } },

 { "id": "trap-progressions", "name": "Trap Progressions", "tag": "TrapProg", "short": "TrapProg",
   "description": "20 classic trap chord progressions, each with 2 alternates (60 in all). One bank per set: main progression, alternate 1 and alternate 2, voice-led, plus 808 roots on spare pads. Original key, or any key you pick.",
   "engine": "epiano", "root": 60, "scale": "minor", "progressionLibrary": "trap",
   "chords": { "min": 1, "maj": 1, "5": 1 }, "voicings": { "close": 1 }, "chordRatio": 1.0,
   "keyTypes": { "note": 1, "octave": 0, "fifth": 0, "stab": 0 },
   "chordRootRange": [53, 64], "keyRange": [36, 60], "velocity": [90, 108], "hold": [1.6, 2.0], "stabChance": 0, "stabHold": 0.3, "strumMs": 0,
   "processing": { "maxLength": 2.2, "fxTailMs": 350,
                   "fx": [ { "type": "highpass", "freq": 120, "poles": 2 }, { "type": "lowpass", "freq": 6500, "poles": 2 },
                           { "type": "reverb", "kind": "plate", "decay": 1.8, "mix": 0.22 } ] },
   "pad": { "muteGroup": 0, "fullLevel": true, "phrasing": "Play a row left to right, one chord per bar (or two), over 808s and hats; switch rows for the next section." } },

 { "id": "wiz-ep", "name": "Wiz Khalifa Electric Piano", "tag": "WizEP", "short": "WizEP",
   "description": "Taylor Gang / Kush & OJ era: dreamy Rhodes maj7, maj9, m9 and maj7#11 chords, spread and drop-2, through tape, slow chorus and a soft plate. Hazy, floating, laid back.",
   "engine": "epiano", "root": 60, "scale": "major", "scaleBank": "chords",
   "chords": { "maj7": 3, "maj9": 4, "m9": 3, "maj7s11": 2, "add9": 2, "m7": 2, "69": 1 },
   "voicings": { "spread": 3, "drop2": 3, "rootless": 2, "close": 1 }, "chordRatio": 0.8,
   "keyTypes": { "note": 4, "octave": 2, "fifth": 0, "stab": 0 },
   "chordRootRange": [50, 61], "keyRange": [55, 79], "velocity": [80, 105], "hold": [1.3, 1.9], "stabChance": 0.05, "stabHold": 0.35, "strumMs": 25,
   "processing": { "maxLength": 2.4, "fxTailMs": 350,
                   "fx": [ { "type": "lowpass", "freq": 6500, "poles": 2 }, { "type": "tape", "drive": 0.3 },
                           { "type": "chorus", "rate": 0.5, "depth": 0.3, "mix": 0.4 }, { "type": "reverb", "kind": "plate", "decay": 1.6, "mix": 0.22 } ] },
   "pad": { "muteGroup": 0, "fullLevel": false, "phrasing": "Let the chords ring over slow 808s; maj7 to m9 a whole step down is the classic float." } },

 { "id": "gfunk-ep", "name": "G-Funk Electric Piano", "tag": "GFunkEP", "short": "GFunkEP",
   "description": "West Coast G-funk keys: Rhodes m7, m9, 9 and 7#9 in dorian with a swirling chorus (phaser-like), a little tube warmth and a short room. Pairs with the Funky Lead and Moog Bass recipes.",
   "engine": "epiano", "root": 60, "scale": "dorian", "scaleBank": "chords",
   "chords": { "m7": 3, "m9": 3, "9": 2, "7": 1, "7s9": 1, "13": 1, "m11": 1 },
   "voicings": { "close": 3, "drop2": 2, "shell": 1 }, "chordRatio": 0.75,
   "keyTypes": { "note": 4, "octave": 2, "fifth": 1, "stab": 1 },
   "chordRootRange": [52, 63], "keyRange": [52, 76], "velocity": [90, 115], "hold": [0.9, 1.4], "stabChance": 0.25, "stabHold": 0.3, "strumMs": 12,
   "processing": { "maxLength": 1.8, "fxTailMs": 200,
                   "fx": [ { "type": "highpass", "freq": 120, "poles": 2 }, { "type": "tube", "drive": 0.25 },
                           { "type": "chorus", "rate": 1.4, "depth": 0.45, "mix": 0.45 }, { "type": "reverb", "kind": "room", "decay": 0.6, "mix": 0.12 } ] },
   "pad": { "muteGroup": 0, "fullLevel": false, "phrasing": "Stabs on the 'and' of 2 and 4 with a laid-back swing; i-m7 to IV9 is the G-funk move." } },

 { "id": "ross-lux-strings", "name": "Rick Ross Luxury Jazz Strings", "tag": "LuxStrings", "short": "LuxStr",
   "description": "Maybach Music luxury: a lush string section on jazz chords (maj9, m9, m11, maj7#11, 6/9), spread wide in a dark hall. Cinematic, expensive, soulful.",
   "engine": "strings", "root": 60, "scale": "dorian", "scaleBank": "chords",
   "chords": { "maj9": 3, "m9": 4, "m11": 3, "maj7s11": 2, "69": 2, "maj7": 2, "m7": 1 },
   "voicings": { "spread": 4, "open": 3, "drop2": 2 }, "chordRatio": 0.85,
   "keyTypes": { "note": 4, "octave": 3, "fifth": 1, "stab": 0 },
   "chordRootRange": [48, 59], "keyRange": [55, 84], "velocity": [85, 110], "hold": [1.9, 2.5], "stabChance": 0.05, "stabHold": 0.5, "strumMs": 20,
   "processing": { "maxLength": 3.0, "fxTailMs": 500,
                   "fx": [ { "type": "highpass", "freq": 140, "poles": 2 }, { "type": "peak", "freq": 3000, "gainDb": 2, "q": 0.8 },
                           { "type": "reverb", "kind": "darkhall", "decay": 2.4, "mix": 0.3 } ] },
   "pad": { "muteGroup": 0, "fullLevel": false, "phrasing": "One chord per bar, let them overlap; add the Luxury Jazz Brass on the downbeats." } },

 { "id": "ross-lux-brass", "name": "Rick Ross Luxury Jazz Brass", "tag": "LuxBrass", "short": "LuxBrs",
   "description": "Maybach Music horns: jazz brass on maj9, m9, 13 and 7#9 chords, half stabs, half swells, warm tube and a plate. Big-band money over trap drums.",
   "engine": "brass", "root": 60, "scale": "dorian", "scaleBank": "chords",
   "chords": { "maj9": 3, "m9": 3, "13": 2, "7s9": 2, "m11": 2, "69": 2, "9sus4": 1 },
   "voicings": { "drop2": 3, "close": 2, "shell": 2, "spread": 1 }, "chordRatio": 0.8,
   "keyTypes": { "note": 4, "octave": 3, "fifth": 1, "stab": 0 },
   "chordRootRange": [50, 61], "keyRange": [55, 79], "velocity": [100, 127], "hold": [0.6, 1.0], "stabChance": 0.35, "stabHold": 0.25, "strumMs": 6,
   "processing": { "maxLength": 1.4, "fxTailMs": 300,
                   "fx": [ { "type": "tube", "drive": 0.3 }, { "type": "peak", "freq": 1500, "gainDb": 2.5, "q": 0.9 },
                           { "type": "compressor", "thresholdDb": -16, "ratio": 3, "attackMs": 5, "releaseMs": 120 },
                           { "type": "reverb", "kind": "plate", "decay": 1.4, "mix": 0.2 } ] },
   "pad": { "muteGroup": 0, "fullLevel": true, "phrasing": "Stabs on 1 and the 'and' of 2, a swell into the next bar; layer with the Luxury Jazz Strings." } },

 { "id": "jeezy-brass", "name": "Jeezy Brass", "tag": "JeezyBrass", "short": "JzBrass",
   "description": "Snowman-era Atlanta (Shawty Redd / Drumma Boy): dark minor brass hits - power chords and minor triads in harmonic minor, short and punchy, saturated and compressed hard.",
   "engine": "brass", "root": 55, "scale": "harmonicminor", "scaleBank": "chords",
   "chords": { "5": 4, "min": 3, "maj": 1, "dim": 1 },
   "voicings": { "close": 3, "open": 2 }, "chordRatio": 0.65,
   "keyTypes": { "note": 5, "octave": 4, "fifth": 2, "stab": 1 },
   "chordRootRange": [46, 57], "keyRange": [43, 70], "velocity": [120, 127], "hold": [0.35, 0.45], "stabChance": 0.3, "stabHold": 0.18, "strumMs": 0,
   "processing": { "maxLength": 0.6, "ampDecayMs": 450, "fxTailMs": 150,
                   "fx": [ { "type": "overdrive", "drive": 0.25, "tone": 3500 },
                           { "type": "compressor", "thresholdDb": -20, "ratio": 5, "attackMs": 1, "releaseMs": 70 },
                           { "type": "reverb", "kind": "room", "decay": 0.6, "mix": 0.14 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Hit the i and bVI chords on the downbeat with the 808; the raised 7th (harmonic minor) is the menace." } },

 { "id": "jeezy-strings", "name": "Jeezy Strings", "tag": "JeezyStrings", "short": "JzStr",
   "description": "Thug Motivation strings: dark, dramatic minor strings - staccato hits and short legato swells in harmonic minor, a little grit and a dark hall.",
   "engine": "strings", "root": 60, "scale": "harmonicminor", "scaleBank": "chords",
   "chords": { "min": 4, "5": 2, "maj": 1, "dim": 1, "m7": 1, "mmaj7": 1 },
   "voicings": { "close": 2, "open": 3, "spread": 1 }, "chordRatio": 0.6,
   "keyTypes": { "note": 5, "octave": 3, "fifth": 1, "stab": 2 },
   "chordRootRange": [50, 61], "keyRange": [55, 84], "velocity": [105, 127], "hold": [0.5, 1.1], "stabChance": 0.35, "stabHold": 0.2, "strumMs": 0,
   "processing": { "maxLength": 1.4, "fxTailMs": 350,
                   "fx": [ { "type": "highpass", "freq": 160, "poles": 2 }, { "type": "tape", "drive": 0.35 },
                           { "type": "reverb", "kind": "darkhall", "decay": 1.8, "mix": 0.25 } ] },
   "pad": { "muteGroup": 0, "fullLevel": true, "phrasing": "Staccato hits on the 16ths before the snare; swell into the hook." } },

 { "id": "trap-brass-stabs", "name": "Trap Brass Stabs", "tag": "TrapBrassStab", "short": "TrBrStab",
   "description": "Modern trap brass: short, distorted fifths and octave stabs low in the register, hard-clipped and compressed so they cut through 808s. (The guide's Trap Majesty Brass is the orchestral one.)",
   "engine": "brass", "root": 52, "scale": "minor", "scaleBank": "chords",
   "chords": { "5": 5, "min": 2, "sus4": 1 },
   "voicings": { "close": 3, "open": 1 }, "chordRatio": 0.6,
   "keyTypes": { "note": 4, "octave": 5, "fifth": 2, "stab": 2 },
   "chordRootRange": [43, 54], "keyRange": [40, 64], "velocity": [127, 127], "hold": [0.2, 0.26], "stabChance": 0.5, "stabHold": 0.14, "strumMs": 0,
   "processing": { "maxLength": 0.35, "ampDecayMs": 300, "fxTailMs": 60,
                   "fx": [ { "type": "highpass", "freq": 90, "poles": 2 }, { "type": "overdrive", "drive": 0.4, "tone": 4000 },
                           { "type": "compressor", "thresholdDb": -22, "ratio": 6, "attackMs": 0.5, "releaseMs": 50 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Triplet stabs between kicks; keep them short so the 808 owns the low end." } },

 { "id": "trap-pizz", "name": "Trap Pizz", "tag": "TrapPizz", "short": "TrPizz",
   "description": "Dark trap pizzicato: plucked strings in harmonic minor for fast triplet runs, with a few fifth double-stops, high-passed with a short plate. Pairs with Trap Scale and Trap Brass.",
   "engine": "pizz", "root": 67, "scale": "harmonicminor", "scaleBank": "notes",
   "chords": { "5": 2, "min": 1 }, "voicings": { "close": 1 }, "chordRatio": 0.15,
   "keyTypes": { "note": 8, "octave": 2, "fifth": 2, "stab": 0 },
   "chordRootRange": [55, 66], "keyRange": [55, 84], "velocity": [105, 127], "hold": [0.18, 0.25], "stabChance": 0, "stabHold": 0.15, "strumMs": 0,
   "processing": { "maxLength": 0.45, "fxTailMs": 150,
                   "fx": [ { "type": "highpass", "freq": 220, "poles": 2 }, { "type": "reverb", "kind": "plate", "decay": 0.9, "mix": 0.18 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Triplet runs up the pads; the raised 7th (pad 7) before the octave gives the tension." } },

 { "id": "gucci-synth", "name": "Gucci Mane Synth", "tag": "GucciSynth", "short": "GucciSyn",
   "description": "So Icy / Zaytoven-Drumma Boy era Atlanta: a bright, bouncy saw + square synth lead in minor pentatonic, short with a filter blip, a 1/8 delay and a plate.",
   "engine": "lead", "root": 72, "scale": "minorpentatonic", "scaleBank": "notes",
   "chords": { "5": 1, "min": 1 }, "voicings": { "close": 1 }, "chordRatio": 0.1,
   "keyTypes": { "note": 8, "octave": 3, "fifth": 1, "stab": 1 },
   "chordRootRange": [60, 71], "keyRange": [60, 86], "velocity": [110, 127], "hold": [0.22, 0.35], "stabChance": 0, "stabHold": 0.15, "strumMs": 0,
   "processing": { "maxLength": 0.6, "fxTailMs": 300, "tempo": 140,
                   "fx": [ { "type": "highpass", "freq": 180, "poles": 2 },
                           { "type": "delay", "sync": "1/8", "feedback": 0.2, "mix": 0.18 }, { "type": "reverb", "kind": "plate", "decay": 1.0, "mix": 0.15 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Bouncy 2-bar riffs on the pentatonic pads, answer with an octave jump." } },

 { "id": "shawty-redd-synth", "name": "Shawty Redd Synth", "tag": "ShawtyRedd", "short": "RedSyn",
   "description": "Shawty Redd's eerie minor synths (Jeezy, Gucci): wide detuned saws with a slow filter, dark harmonic-minor melodies and minor chord stabs, in a big hall.",
   "engine": "supersaw", "root": 64, "scale": "harmonicminor", "scaleBank": "notes",
   "chords": { "min": 3, "5": 2, "m7": 1, "mmaj7": 1 }, "voicings": { "close": 2, "open": 1 }, "chordRatio": 0.35,
   "keyTypes": { "note": 7, "octave": 3, "fifth": 1, "stab": 1 },
   "chordRootRange": [52, 63], "keyRange": [55, 84], "velocity": [100, 120], "hold": [0.5, 0.9], "stabChance": 0.2, "stabHold": 0.25, "strumMs": 0,
   "processing": { "maxLength": 1.3, "fxTailMs": 450,
                   "fx": [ { "type": "lowpass", "freq": 5000, "poles": 2 }, { "type": "highpass", "freq": 150, "poles": 2 },
                           { "type": "reverb", "kind": "hall", "decay": 2.0, "mix": 0.26 } ] },
   "pad": { "muteGroup": 1, "fullLevel": false, "phrasing": "Slow, menacing 4-note melodies; stab the minor chord under the hook." } },

 { "id": "juicyj-strip-synth", "name": "Juicy J Strip Club Synth Line", "tag": "StripSynth", "short": "StripSyn",
   "description": "Memphis strip-club synth lines (Juicy J / Three 6 Mafia): a high, nasal square-ish lead in phrygian minor, short and bouncy, a dotted delay and a touch of bitcrush.",
   "engine": "lead", "root": 79, "scale": "phrygian", "scaleBank": "notes",
   "chords": { "5": 1 }, "voicings": { "close": 1 }, "chordRatio": 0.0,
   "keyTypes": { "note": 9, "octave": 3, "fifth": 1, "stab": 1 },
   "chordRootRange": [67, 78], "keyRange": [67, 91], "velocity": [115, 127], "hold": [0.15, 0.25], "stabChance": 0, "stabHold": 0.12, "strumMs": 0,
   "processing": { "maxLength": 0.45, "fxTailMs": 300, "tempo": 140,
                   "fx": [ { "type": "highpass", "freq": 300, "poles": 2 }, { "type": "bitcrush", "bits": 10, "downsample": 1 },
                           { "type": "delay", "sync": "3/16", "feedback": 0.25, "mix": 0.2 } ] },
   "pad": { "muteGroup": 1, "fullLevel": true, "phrasing": "Repeating 1-bar hooks with the flat 2nd (pad 2) - the phrygian sound of Memphis." } },

 { "id": "smooth-jazz-pad", "name": "Smooth Jazz Pad", "tag": "JazzPad", "short": "JzPad",
   "description": "Smooth jazz pads: a warm analog pad on maj9, m9, m11, 13 and 6/9 chords, rootless and spread, slow chorus and a long hall. Sits under Rhodes and sax.",
   "engine": "pad", "root": 60, "scale": "major", "scaleBank": "chords",
   "chords": { "maj9": 4, "m9": 4, "m11": 3, "13": 2, "69": 3, "maj7s11": 2, "9sus4": 2 },
   "voicings": { "rootless": 3, "spread": 3, "drop2": 2 }, "chordRatio": 0.9,
   "keyTypes": { "note": 3, "octave": 2, "fifth": 0, "stab": 0 },
   "chordRootRange": [48, 59], "keyRange": [48, 76], "velocity": [80, 100], "hold": [2.2, 2.8], "stabChance": 0, "stabHold": 0.5, "strumMs": 0,
   "processing": { "maxLength": 3.2, "fxTailMs": 600,
                   "fx": [ { "type": "chorus", "rate": 0.4, "depth": 0.35, "mix": 0.4 }, { "type": "reverb", "kind": "hall", "decay": 2.6, "mix": 0.28 } ] },
   "pad": { "muteGroup": 0, "fullLevel": false, "phrasing": "Hold each chord a full bar; ii9-V13-Imaj9 is home base." } },

 { "id": "sade-pad", "name": "Sade Pad", "tag": "SadePad", "short": "SadePad",
   "description": "Sade-style quiet storm: a dark, warm pad on m9, m11, maj7 and add9 chords, low-passed and taped, with a long soft plate. Moody, sensual, sparse.",
   "engine": "pad", "root": 57, "scale": "dorian", "scaleBank": "chords",
   "chords": { "m9": 4, "m11": 3, "maj7": 3, "add9": 2, "madd9": 2, "m7": 2, "maj9": 2 },
   "voicings": { "spread": 3, "open": 3, "rootless": 1 }, "chordRatio": 0.9,
   "keyTypes": { "note": 3, "octave": 2, "fifth": 0, "stab": 0 },
   "chordRootRange": [45, 56], "keyRange": [45, 72], "velocity": [75, 95], "hold": [2.4, 3.0], "stabChance": 0, "stabHold": 0.5, "strumMs": 0,
   "processing": { "maxLength": 3.4, "fxTailMs": 650,
                   "fx": [ { "type": "lowpass", "freq": 3500, "poles": 2 }, { "type": "tape", "drive": 0.3 },
                           { "type": "reverb", "kind": "plate", "decay": 2.2, "mix": 0.26 } ] },
   "pad": { "muteGroup": 0, "fullLevel": false, "phrasing": "Two chords, slow: i m9 to bVII maj7 (or iv m11) over a soft kick and rim." } }
]
)JSON";

static juce::String safeTag (const juce::String& t)
{
    return t.retainCharacters ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-");
}

static juce::var weightsToVar (const std::vector<Weighted<juce::String>>& w)
{
    auto* obj = new juce::DynamicObject();
    for (auto& e : w) obj->setProperty (e.value, e.weight);
    return juce::var (obj);
}

static juce::var rangeVar (double a, double b)
{
    juce::Array<juce::var> arr { a, b };
    return juce::var (arr);
}

juce::var Style::toVar() const
{
    auto* o = new juce::DynamicObject();
    o->setProperty ("id", id);
    o->setProperty ("name", name);
    o->setProperty ("description", description);
    o->setProperty ("scale", scale);
    o->setProperty ("chords", weightsToVar (chords));

    std::vector<Weighted<juce::String>> v;
    for (auto& e : voicings) v.push_back ({ voicingId (e.value), e.weight });
    o->setProperty ("voicings", weightsToVar (v));

    o->setProperty ("chordRatio", chordRatio);
    auto* kt = new juce::DynamicObject();
    kt->setProperty ("note", keyNote);
    kt->setProperty ("octave", keyOctave);
    kt->setProperty ("fifth", keyFifth);
    kt->setProperty ("stab", keyStab);
    o->setProperty ("keyTypes", juce::var (kt));
    o->setProperty ("chordRootRange", rangeVar (chordRootLow, chordRootHigh));
    o->setProperty ("keyRange", rangeVar (keyLow, keyHigh));
    o->setProperty ("velocity", rangeVar (velocityLow, velocityHigh));
    o->setProperty ("hold", rangeVar (holdLow, holdHigh));
    o->setProperty ("stabChance", stabChance);
    o->setProperty ("stabHold", stabHold);
    o->setProperty ("strumMs", strumMs);
    o->setProperty ("engine", engine);
    if (tag.isNotEmpty()) o->setProperty ("tag", tag);
    if (shortTag.isNotEmpty()) o->setProperty ("short", shortTag);
    if (scaleBank.isNotEmpty()) o->setProperty ("scaleBank", scaleBank);
    if (scaleBankOnly) o->setProperty ("scaleBankOnly", true);
    if (progressionLibrary.isNotEmpty()) o->setProperty ("progressionLibrary", progressionLibrary);
    if (root >= 0) o->setProperty ("root", root);
    if (isKit()) o->setProperty ("kit", weightsToVar (kit));

    auto* pr = new juce::DynamicObject();
    pr->setProperty ("maxLength", processing.maxLength);
    pr->setProperty ("attackMs", processing.attackMs);
    pr->setProperty ("ampDecayMs", processing.ampDecayMs);
    pr->setProperty ("fxTailMs", processing.fxTailMs);
    pr->setProperty ("pitchShift", processing.pitchShift);
    pr->setProperty ("tempo", processing.tempo);
    if (processing.fx.isArray()) pr->setProperty ("fx", processing.fx);
    o->setProperty ("processing", juce::var (pr));

    auto* pad_ = new juce::DynamicObject();
    pad_->setProperty ("tune", pad.tune);
    pad_->setProperty ("muteGroup", pad.muteGroup);
    pad_->setProperty ("fullLevel", pad.fullLevel);
    if (pad.phrasing.isNotEmpty()) pad_->setProperty ("phrasing", pad.phrasing);
    o->setProperty ("pad", juce::var (pad_));
    return juce::var (o);
}

template <typename T>
static void readRange (const juce::var& v, const char* key, T& lo, T& hi)
{
    auto r = v[key];
    if (r.isArray() && r.size() == 2)
    {
        lo = (T) (double) r[0];
        hi = (T) (double) r[1];
        if (hi < lo) std::swap (lo, hi);
    }
}

bool Style::fromVar (const juce::var& v, Style& s, juce::String& error)
{
    if (! v.isObject()) { error = "style must be a JSON object"; return false; }

    s = Style();
    s.id = v["id"].toString();
    s.name = v.getProperty ("name", s.id).toString();
    s.description = v["description"].toString();
    if (s.id.isEmpty()) s.id = s.name.toLowerCase().replaceCharacter (' ', '-');
    if (s.id.isEmpty()) { error = "style needs an \"id\" or \"name\""; return false; }

    if (v.hasProperty ("scale")) s.scale = v["scale"].toString();
    std::vector<int> tmp;
    if (! getScale (s.scale, tmp))
    {
        error = "unknown scale '" + s.scale + "' (use one of: " + allScaleNames().joinIntoString (", ") + ")";
        return false;
    }

    if (auto* kitObj = v["kit"].getDynamicObject())
        for (auto& p : kitObj->getProperties())
            if ((float) p.value > 0)
                s.kit.push_back ({ p.name.toString(), (float) p.value });

    if (auto* chords = v["chords"].getDynamicObject())
        for (auto& p : chords->getProperties())
        {
            if (findQuality (p.name.toString()) == nullptr)
            {
                juce::StringArray ids;
                for (auto& q : allQualities()) ids.add (q.id);
                error = "unknown chord quality '" + p.name.toString() + "' (known: " + ids.joinIntoString (", ") + ")";
                return false;
            }
            if ((float) p.value > 0)
                s.chords.push_back ({ p.name.toString(), (float) p.value });
        }

    if (s.chords.empty() && s.isKit())
        s.chords.push_back ({ "min", 1.0f }); // unused: a kit plans with its member styles
    if (s.chords.empty()) { error = "style '" + s.id + "' needs at least one entry in \"chords\""; return false; }

    if (auto* vo = v["voicings"].getDynamicObject())
        for (auto& p : vo->getProperties())
        {
            Voicing voicing;
            if (! parseVoicing (p.name.toString(), voicing))
            {
                error = "unknown voicing '" + p.name.toString() + "' (known: " + allVoicingIds().joinIntoString (", ") + ")";
                return false;
            }
            if ((float) p.value > 0)
                s.voicings.push_back ({ voicing, (float) p.value });
        }

    if (s.voicings.empty())
        s.voicings.push_back ({ Voicing::Close, 1.0f });

    s.chordRatio = juce::jlimit (0.0f, 1.0f, (float) v.getProperty ("chordRatio", s.chordRatio));

    auto kt = v["keyTypes"];
    if (kt.isObject())
    {
        s.keyNote   = juce::jmax (0.0f, (float) kt.getProperty ("note", 0));
        s.keyOctave = juce::jmax (0.0f, (float) kt.getProperty ("octave", 0));
        s.keyFifth  = juce::jmax (0.0f, (float) kt.getProperty ("fifth", 0));
        s.keyStab   = juce::jmax (0.0f, (float) kt.getProperty ("stab", 0));
        if (s.keyNote + s.keyOctave + s.keyFifth + s.keyStab <= 0) s.keyNote = 1;
    }

    readRange (v, "chordRootRange", s.chordRootLow, s.chordRootHigh);
    readRange (v, "keyRange", s.keyLow, s.keyHigh);
    readRange (v, "velocity", s.velocityLow, s.velocityHigh);
    readRange (v, "hold", s.holdLow, s.holdHigh);

    s.chordRootLow  = juce::jlimit (24, 96, s.chordRootLow);
    s.chordRootHigh = juce::jlimit (s.chordRootLow + 11, 108, s.chordRootHigh);
    s.keyLow  = juce::jlimit (21, 108, s.keyLow);
    s.keyHigh = juce::jlimit (s.keyLow, 108, s.keyHigh);
    s.velocityLow  = juce::jlimit (1, 127, s.velocityLow);
    s.velocityHigh = juce::jlimit (s.velocityLow, 127, s.velocityHigh);
    s.holdLow  = juce::jlimit (0.05, 20.0, s.holdLow);
    s.holdHigh = juce::jlimit (s.holdLow, 20.0, s.holdHigh);

    s.stabChance = juce::jlimit (0.0f, 1.0f, (float) v.getProperty ("stabChance", s.stabChance));
    s.stabHold   = juce::jlimit (0.03, 5.0, (double) v.getProperty ("stabHold", s.stabHold));
    s.strumMs    = juce::jlimit (0.0, 250.0, (double) v.getProperty ("strumMs", s.strumMs));
    // processing (the top-level "maxLength" of older style files still works)
    auto pr = v["processing"];
    auto& P = s.processing;
    P.maxLength  = juce::jlimit (0.0, 30.0, (double) pr.getProperty ("maxLength", v.getProperty ("maxLength", 0.0)));
    P.attackMs   = juce::jlimit (0.0, 2000.0, (double) pr.getProperty ("attackMs", 0.0));
    P.ampDecayMs = juce::jlimit (0.0, 30000.0, (double) pr.getProperty ("ampDecayMs", 0.0));
    P.fxTailMs   = juce::jlimit (0.0, 10000.0, (double) pr.getProperty ("fxTailMs", 0.0));
    P.pitchShift = juce::jlimit (-24, 24, (int) pr.getProperty ("pitchShift", 0));
    P.tempo      = juce::jlimit (30.0, 300.0, (double) pr.getProperty ("tempo", 90.0));
    P.fx         = pr["fx"];
    if (! validateFx (P.fx, error)) { error = "style '" + s.id + "': " + error; return false; }

    s.tag = safeTag (v.getProperty ("tag", s.name).toString());
    s.shortTag = safeTag (v.getProperty ("short", v.hasProperty ("tag") ? s.tag.substring (0, 9) : juce::String()).toString());
    s.root = v.hasProperty ("root") ? juce::jlimit (0, 127, (int) v["root"]) : -1;

    s.scaleBank = v["scaleBank"].toString().toLowerCase();
    if (s.scaleBank.isNotEmpty() && s.scaleBank != "notes" && s.scaleBank != "chords" && s.scaleBank != "progressions")
    {
        error = "style '" + s.id + "': \"scaleBank\" must be \"notes\", \"chords\" or \"progressions\"";
        return false;
    }
    s.scaleBankOnly = (bool) v.getProperty ("scaleBankOnly", false);
    s.progressionLibrary = v["progressionLibrary"].toString().toLowerCase();
    if (s.progressionLibrary.isNotEmpty())
    {
        if (forge::progressionLibrary (s.progressionLibrary).empty())
        {
            error = "style '" + s.id + "': unknown \"progressionLibrary\" '" + s.progressionLibrary + "' (known: trap)";
            return false;
        }
        s.scaleBank = "progressions";
        s.scaleBankOnly = true;
    }

    auto pad = v["pad"];
    s.pad.tune      = juce::jlimit (-36, 36, (int) pad.getProperty ("tune", 0));
    s.pad.muteGroup = juce::jlimit (0, 32, (int) pad.getProperty ("muteGroup", 1));
    s.pad.fullLevel = (bool) pad.getProperty ("fullLevel", false);
    s.pad.phrasing  = pad["phrasing"].toString();

    // "engine" (older files: "recipe") picks the built-in synth sound
    s.engine = v.getProperty ("engine", v.getProperty ("recipe", "epiano")).toString().toLowerCase();
    Engine r;
    if (s.engine != "mixed" && ! parseEngine (s.engine, r))
    {
        error = "unknown engine '" + s.engine + "' (use one of: " + engineIds().joinIntoString (", ") + ", mixed)";
        return false;
    }
    return true;
}

const juce::Array<Style>& builtInStyles()
{
    static const juce::Array<Style> styles = []
    {
        juce::Array<Style> result;
        auto parsed = juce::JSON::parse (builtInStyleJson);
        jassert (parsed.isArray());

        for (auto& v : *parsed.getArray())
        {
            Style s;
            juce::String err;
            if (Style::fromVar (v, s, err))
                result.add (s);
            else
                jassertfalse;
        }
        return result;
    }();
    return styles;
}

const Style* findBuiltInStyle (const juce::String& idOrName)
{
    for (auto& s : builtInStyles())
        if (s.id.equalsIgnoreCase (idOrName) || s.name.equalsIgnoreCase (idOrName))
            return &s;
    return nullptr;
}

bool loadStyleFile (const juce::File& f, Style& out, juce::String& error)
{
    if (! f.existsAsFile()) { error = "style file not found: " + f.getFullPathName(); return false; }

    juce::var v;
    auto result = juce::JSON::parse (f.loadFileAsString(), v);
    if (result.failed()) { error = "JSON error in " + f.getFileName() + ": " + result.getErrorMessage(); return false; }

    if (v.isArray() && v.size() > 0)
        v = v[0];

    return Style::fromVar (v, out, error);
}
}
