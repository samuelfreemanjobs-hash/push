#Requires -Version 5.1
# Seed Mem0 with DiskLordz constraints via record script (dry-run friendly) or Claude /mem0:remember.

$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
$Mem0Dir = Join-Path $Root "disklordz-ralph\business\mem0"

if (-not $env:MEM0_API_KEY) {
    Write-Host "MEM0_API_KEY not set — paste these into Claude Code /mem0:remember instead:"
    Write-Host @"

Category: decisions_and_constraints
SampleRate 44100/48000, MaxBlock 512, zero heap in processBlock, MaxRenderMs=15, MIN_SPECTRAL_CONVERGENCE=0.08.

Category: project_knowledge
STACK-FROZEN: ship starter-cpp Detroit Phonk Trap Maker before TM88/Luger labs. Judges evaluate_vst.py and evaluate.py are immutable.

Category: sonic_targets
Punchy sub-808 kick (48Hz fundamental, ~55ms pitch drop), crisp trap snare, metallic hat stack.
"@
    exit 0
}

Push-Location $Mem0Dir
try {
    & python -c @"
from mem0 import MemoryClient
import os
client = MemoryClient(api_key=os.environ['MEM0_API_KEY'])
uid = os.environ.get('DLZ_MEM0_USER_ID', 'disklordz-operator')
aid = os.environ.get('DLZ_MEM0_AGENT_ID', 'memory-curator')
app = os.environ.get('DLZ_MEM0_APP_ID', 'disklordz-push')
seeds = [
    ('decisions_and_constraints', 'DSP constraints: 44100/48000, RT-safe, no heap in audio thread, MaxRenderMs 15, originality floor 0.08 spectral convergence.'),
    ('project_knowledge', 'DiskLordz ships starter-cpp VST3 first per STACK-FROZEN; autoresearch sandboxes VoiceDSP.h and drum_engine.py only.'),
    ('sonic_targets', 'Target: 48Hz 808 fundamental, 55ms pitch drop, trap snare transient, 6-oscillator hat metallic layer.'),
]
for cat, text in seeds:
    client.add(
        [{'role': 'user', 'content': text}, {'role': 'assistant', 'content': f'Seeded {cat} for DiskLordz.'}],
        user_id=uid,
        agent_id=aid,
        metadata={'app_id': app, 'category': cat},
    )
    print('seeded', cat)
"@
} finally {
    Pop-Location
}

Write-Host "Done. Verify: /mem0:search --category decisions_and_constraints dsp"