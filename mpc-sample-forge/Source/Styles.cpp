#include "Styles.h"
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

 { "id": "jetlife-ep-bell", "name": "Jet Life EP Bell", "tag": "JetLifeEPBell",
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

 { "id": "polyester-pluck", "name": "Polyester G-Funk Triangle Pluck", "tag": "PolyesterPluck",
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

 { "id": "memphis-organ", "name": "90s Memphis Detuned Organ Stab", "tag": "MemphisOrgan",
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

 { "id": "phonk-cowbell", "name": "Rare Phonk Pitch-Shifted Cowbell", "tag": "PhonkCowbell",
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

 { "id": "zaytoven-organ", "name": "Zaytoven Gospel Trap Organ", "tag": "ZaytovenOrgan",
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

 { "id": "luxury-harp", "name": "Luxury Ross Cinematic Harp", "tag": "LuxuryHarp",
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

 { "id": "screwed-choir", "name": "Screw-Head Down-Pitched Choir", "tag": "ScrewedChoir",
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

 { "id": "trap-brass", "name": "Trap Majesty Orchestral Brass", "tag": "TrapBrass",
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

 { "id": "rhodes-bass", "name": "Cruisin' Smooth Rhodes Bass/Key", "tag": "RhodesBass",
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

 { "id": "westcoast-kit", "name": "West Coast / Jet Life Kit", "tag": "WestCoast",
   "description": "Kit: Jet Life EP Bell + Polyester Pluck + Rhodes Bass (smooth analog, warm tines, Major 7th / Minor 9th / Dorian).",
   "kit": { "jetlife-ep-bell": 3, "polyester-pluck": 3, "rhodes-bass": 2 } },

 { "id": "memphis-kit", "name": "Memphis / OG Phonk Kit", "tag": "Memphis",
   "description": "Kit: Memphis Organ + Phonk Cowbell + Screwed Choir (cassette loss, wow & flutter, Harmonic Minor / Locrian).",
   "kit": { "memphis-organ": 3, "phonk-cowbell": 3, "screwed-choir": 2 } },

 { "id": "atlanta-kit", "name": "Atlanta / Luxury Trap Kit", "tag": "Atlanta",
   "description": "Kit: Zaytoven Organ + Luxury Harp + Trap Brass (workstation organs, harp plucks, brass hits, Natural Minor).",
   "kit": { "zaytoven-organ": 3, "luxury-harp": 3, "trap-brass": 2 } },

 { "id": "guide-palette", "name": "Recipe Guide Palette (all 9)", "tag": "GuidePalette",
   "description": "Kit: all nine guide recipes for puzzle-piece layering (e.g. Rhodes Bass on pad 1, EP Bell on pad 2, Polyester Pluck on pad 3).",
   "kit": { "rhodes-bass": 1, "jetlife-ep-bell": 1, "polyester-pluck": 1, "memphis-organ": 1, "phonk-cowbell": 1,
            "zaytoven-organ": 1, "luxury-harp": 1, "screwed-choir": 1, "trap-brass": 1 } }
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
    s.root = v.hasProperty ("root") ? juce::jlimit (0, 127, (int) v["root"]) : -1;

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
