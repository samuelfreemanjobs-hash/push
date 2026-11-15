#Requires -Version 5.1
# Seed Ruflo vector memory with DiskLordz DSP constraints (run after npx ruflo@latest init).

$ErrorActionPreference = "Stop"

if (-not (Get-Command npx -ErrorAction SilentlyContinue)) {
    Write-Error "npx not found"
}

npx ruflo@latest memory store --key "dsp_constraints" --value "SampleRate=44100/48000, MaxBlock=512, RealTimeSafe=True, MaxRenderMs=15.0, OriginalityFloor=0.08"
npx ruflo@latest memory store --key "sonic_targets" --value "Punchy sub-808 kick (48Hz fundamental, 55ms pitch drop), crisp trap snare, 6-oscillator metallic hat"

Write-Host "Seeded dsp_constraints and sonic_targets. Verify with: npx ruflo@latest memory search --query dsp_constraints"