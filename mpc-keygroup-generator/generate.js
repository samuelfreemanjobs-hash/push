#!/usr/bin/env node
/**
 * Akai MPC Keygroup Program (.xpm) Generator
 *
 * Targets: MPC Live, MPC One, MPC X, MPC Key (firmware 2.x — XPM v2 format)
 *
 * JSON config (config.json):
 * {
 *   "programName": "Piano",
 *   "outputFile":  "Piano.xpm",   // optional, defaults to "<programName>.xpm"
 *   "firmware":    "2.10.0",       // optional
 *   "polyphony":   32,             // optional
 *   "samples": [
 *     { "file": "Piano_C2.wav", "rootNote": "C2" },
 *     { "file": "Piano_C3.wav", "rootNote": "C3" },
 *     { "file": "Piano_C4.wav", "rootNote": "C4" }
 *   ]
 * }
 *
 * Velocity layers per key (up to 4):
 * {
 *   "samples": [
 *     {
 *       "rootNote": "C3",
 *       "velocityLayers": [
 *         { "file": "Piano_C3_pp.wav", "velStart": 0,  "velEnd": 63  },
 *         { "file": "Piano_C3_ff.wav", "velStart": 64, "velEnd": 127 }
 *       ]
 *     }
 *   ]
 * }
 *
 * CLI (no JSON file):
 *   node generate.js --name Piano --out Piano.xpm C2:Piano_C2.wav C3:Piano_C3.wav
 *   node generate.js --name Piano Piano_C2.wav Piano_C3.wav   # auto-detects roots from names
 */

import { readFileSync, writeFileSync } from 'fs';
import { basename } from 'path';

// ─── MIDI utilities ──────────────────────────────────────────────────────────
// MPC convention: C-2 = MIDI 0 · C3 = MIDI 60 (middle C)

const CHROMATIC = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B'];

// Also accept flats written as Bb, Eb, Ab, Db, Gb
const FLAT_MAP = { 'BB': 'A#', 'EB': 'D#', 'AB': 'G#', 'DB': 'C#', 'GB': 'F#' };

function noteToMidi(name) {
  if (typeof name === 'number') {
    if (name < 0 || name > 127) throw new Error(`MIDI note out of range: ${name}`);
    return name;
  }
  let s = String(name).toUpperCase().trim();
  // Normalise flats (e.g. Bb3 → A#3)
  s = s.replace(/^([A-G]B)(-?\d+)$/, (_, note, oct) => (FLAT_MAP[note] ?? note) + oct);
  const m = s.match(/^([A-G]#?)(-?\d+)$/);
  if (!m) throw new Error(`Invalid note name: "${name}" — use C3, A#4, Bb2, etc.`);
  const chroma = CHROMATIC.indexOf(m[1]);
  if (chroma === -1) throw new Error(`Unknown note: "${m[1]}"`);
  const midi = (parseInt(m[2], 10) + 2) * 12 + chroma;
  if (midi < 0 || midi > 127) throw new Error(`Note out of MIDI range: "${name}" = ${midi}`);
  return midi;
}

function midiToNote(n) {
  return `${CHROMATIC[n % 12]}${Math.floor(n / 12) - 2}`;
}

/**
 * Try to extract a root note from a filename.
 * Handles suffixes like Piano_C3.wav, strings_A#4.wav, lead-Bb2.wav
 */
function detectRootFromName(filename) {
  let name = basename(filename, '.wav').toUpperCase();
  // Normalise flats before matching
  name = name.replace(/([A-G])B(-?\d)/g, (_, n, d) => (FLAT_MAP[n + 'B'] ?? n + 'B') + d);
  const m = name.match(/[_\-\s]([A-G]#?)(-?\d+)$/);
  if (!m) return null;
  try { return noteToMidi(m[1] + m[2]); } catch { return null; }
}

// ─── Keyzone algorithm ───────────────────────────────────────────────────────

/**
 * Assign LowNote/HighNote to each sample using the "split the difference"
 * midpoint algorithm.  Zones cover 0–127 with no gaps and no overlaps.
 *
 * Example — roots at 36 (C2), 60 (C3), 72 (C4):
 *   Zone 1: low=0,  high=floor((36+60)/2)=48
 *   Zone 2: low=49, high=floor((60+72)/2)=66
 *   Zone 3: low=67, high=127
 */
function assignKeyzones(samples) {
  const sorted = [...samples].sort((a, b) => a.rootNote - b.rootNote);

  return sorted.map((s, i) => {
    const prev = sorted[i - 1];
    const next = sorted[i + 1];
    return {
      ...s,
      lowNote:  prev ? Math.floor((prev.rootNote + s.rootNote) / 2) + 1 : 0,
      highNote: next ? Math.floor((s.rootNote + next.rootNote) / 2) : 127,
    };
  });
}

// ─── Validation ──────────────────────────────────────────────────────────────

function validateZones(zones) {
  const errors = [];

  if (zones.length === 0) errors.push('No samples provided.');

  // Check coverage and continuity
  const sorted = [...zones].sort((a, b) => a.lowNote - b.lowNote);
  if (sorted[0].lowNote !== 0)
    errors.push(`First zone starts at ${sorted[0].lowNote}, not 0.`);
  if (sorted.at(-1).highNote !== 127)
    errors.push(`Last zone ends at ${sorted.at(-1).highNote}, not 127.`);

  for (let i = 1; i < sorted.length; i++) {
    const prev = sorted[i - 1];
    const curr = sorted[i];
    if (curr.lowNote !== prev.highNote + 1)
      errors.push(`Gap/overlap between zone ${i} (ends ${prev.highNote}) and zone ${i+1} (starts ${curr.lowNote}).`);
    if (curr.lowNote > curr.highNote)
      errors.push(`Zone ${i+1} has invalid range: ${curr.lowNote}–${curr.highNote}.`);
  }

  // Check velocity coverage per zone
  zones.forEach((zone, zi) => {
    const layers = zone.velocityLayers ?? [{ velStart: 0, velEnd: 127 }];
    const velSorted = [...layers].sort((a, b) => a.velStart - b.velStart);
    if (velSorted[0].velStart !== 0)
      errors.push(`Zone ${zi+1}: velocity layers don't start at 0.`);
    if (velSorted.at(-1).velEnd !== 127)
      errors.push(`Zone ${zi+1}: velocity layers don't end at 127.`);
    for (let i = 1; i < velSorted.length; i++) {
      if (velSorted[i].velStart !== velSorted[i-1].velEnd + 1)
        errors.push(`Zone ${zi+1}: velocity gap between layers ${i} and ${i+1}.`);
    }
    if (layers.length > 4)
      errors.push(`Zone ${zi+1}: MPC supports up to 4 velocity layers, got ${layers.length}.`);
  });

  return errors;
}

// ─── XPM generation ──────────────────────────────────────────────────────────

function xml(str) {
  return String(str)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;');
}

/**
 * Build XPM v2 XML string.
 * This is the format used by MPC Live / One / X / Key running firmware 2.x.
 *
 * Key structure differences from v1:
 *   v1: <Keygroup>  <LowKey>/<HighKey>  <VelLow>/<VelHigh>  <NumKeygroupsPlay>
 *   v2: <Instrument> <LowNote>/<HighNote> <VelStart>/<VelEnd> <KeygroupNumKeygroups>
 */
function buildXPM(config, zones) {
  const firmware  = config.firmware  ?? '2.10.0';
  const polyphony = config.polyphony ?? 32;

  const instrumentsXML = zones.map((zone, zi) => {
    const velLayers = zone.velocityLayers ?? [
      { file: zone.file, velStart: 0, velEnd: 127 },
    ];

    const layersXML = velLayers.map((layer, li) => {
      // SampleFile = just the filename; MPC resolves relative to the .xpm location
      const sampleFile = xml(basename(layer.file));
      return `        <Layer number="${li + 1}">
          <SampleFile>${sampleFile}</SampleFile>
          <RootNote>${zone.rootNote}</RootNote>
          <VelStart>${layer.velStart}</VelStart>
          <VelEnd>${layer.velEnd}</VelEnd>
          <SampleStart>0</SampleStart>
          <SampleEnd>-1</SampleEnd>
          <LoopStart>-1</LoopStart>
          <LoopEnd>-1</LoopEnd>
          <Looping>0</Looping>
          <Bidirectional>0</Bidirectional>
          <Reverse>0</Reverse>
          <VolumeOffset>0</VolumeOffset>
          <PanOffset>0</PanOffset>
          <TuneOffset>0</TuneOffset>
        </Layer>`;
    }).join('\n');

    return `      <Instrument number="${zi + 1}">
        <Polyphony>${polyphony}</Polyphony>
        <LowNote>${zone.lowNote}</LowNote>
        <HighNote>${zone.highNote}</HighNote>
        <Layers>
${layersXML}
        </Layers>
      </Instrument>`;
  }).join('\n');

  return `<?xml version="1.0" encoding="UTF-8"?>
<MPCVObject>
  <Version>
    <File_Version>2.1</File_Version>
    <Application>MPC-V</Application>
    <Application_Version>${xml(firmware)}</Application_Version>
    <Platform>Linux</Platform>
  </Version>
  <Program type="Keygroup">
    <ProgramName>${xml(config.programName)}</ProgramName>
    <KeygroupNumKeygroups>${zones.length}</KeygroupNumKeygroups>
    <KeygroupMasterTranspose>0</KeygroupMasterTranspose>
    <Instruments>
${instrumentsXML}
    </Instruments>
  </Program>
</MPCVObject>
`;
}

// ─── Config parsing ───────────────────────────────────────────────────────────

function loadConfig() {
  const args = process.argv.slice(2);

  if (args.length === 0) {
    console.error([
      'Usage:',
      '  node generate.js config.json',
      '  node generate.js --name "Piano" [--out Piano.xpm] NOTE:FILE ...',
      '  node generate.js --name "Piano" Piano_C2.wav Piano_C3.wav   # auto-detect root',
      '',
      'Examples:',
      '  node generate.js --name Piano C2:Piano_C2.wav C3:Piano_C3.wav C4:Piano_C4.wav',
      '  node generate.js --name Piano Piano_C2.wav Piano_C3.wav Piano_C4.wav',
    ].join('\n'));
    process.exit(1);
  }

  // JSON file
  if (args[0].endsWith('.json') && !args[0].startsWith('--')) {
    const raw = JSON.parse(readFileSync(args[0], 'utf8'));
    if (!raw.programName) throw new Error('Config must have "programName".');
    if (!raw.samples?.length) throw new Error('Config must have at least one sample in "samples".');
    return raw;
  }

  // CLI flags
  const config = { programName: 'KeygroupProgram', samples: [] };
  for (let i = 0; i < args.length; i++) {
    switch (args[i]) {
      case '--name':      config.programName = args[++i]; break;
      case '--out':       config.outputFile  = args[++i]; break;
      case '--firmware':  config.firmware    = args[++i]; break;
      case '--polyphony': config.polyphony   = parseInt(args[++i], 10); break;
      default: {
        const arg = args[i];
        if (arg.includes(':')) {
          const colon = arg.indexOf(':');
          config.samples.push({ rootNote: arg.slice(0, colon), file: arg.slice(colon + 1) });
        } else {
          config.samples.push({ file: arg });
        }
      }
    }
  }
  if (!config.outputFile) config.outputFile = `${config.programName}.xpm`;
  return config;
}

// ─── Main ─────────────────────────────────────────────────────────────────────

function main() {
  const config = loadConfig();

  // Resolve root notes
  const samplesWithRoots = config.samples.map((s, i) => {
    let rootNote;

    if (s.velocityLayers) {
      // Multi-velocity entry — root note required
      if (s.rootNote == null)
        throw new Error(`Sample ${i + 1}: "rootNote" is required when using "velocityLayers".`);
      rootNote = noteToMidi(s.rootNote);
    } else {
      // Single-file entry
      if (s.rootNote != null) {
        rootNote = noteToMidi(s.rootNote);
      } else {
        rootNote = detectRootFromName(s.file);
        if (rootNote == null)
          throw new Error(
            `Cannot detect root note from "${s.file}". ` +
            'Name it like Piano_C3.wav, or specify "rootNote": "C3" in config.'
          );
        console.log(`  Auto-detected root: ${s.file} → ${midiToNote(rootNote)} (MIDI ${rootNote})`);
      }
    }

    return { ...s, rootNote };
  });

  // Calculate keyzones
  const zones = assignKeyzones(samplesWithRoots);

  // Validate
  const errors = validateZones(zones);
  if (errors.length > 0) {
    console.error('Keyzone validation failed:');
    errors.forEach(e => console.error(`  ✗ ${e}`));
    process.exit(1);
  }

  // Report mapping
  console.log(`\nKeygroup program: "${config.programName}" — ${zones.length} zone(s)\n`);
  console.log('  #   Root      Zone       Sample');
  console.log('  ──  ────────  ─────────  ──────────────────────');
  zones.forEach((z, i) => {
    const root     = `${midiToNote(z.rootNote).padEnd(4)} (${String(z.rootNote).padStart(3)})`;
    const range    = `${midiToNote(z.lowNote).padEnd(4)}–${midiToNote(z.highNote).padEnd(4)}`;
    const layerDesc = z.velocityLayers
      ? `${z.velocityLayers.length} vel layer(s)`
      : basename(z.file);
    console.log(`  ${String(i + 1).padStart(2)}  ${root}  ${range}  ${layerDesc}`);
  });

  // Generate and write
  const xpm = buildXPM(config, zones);
  const outFile = config.outputFile ?? `${config.programName}.xpm`;
  writeFileSync(outFile, xpm, 'utf8');
  console.log(`\nWrote: ${outFile}`);
}

main();
