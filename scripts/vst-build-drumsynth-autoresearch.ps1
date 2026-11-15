#Requires -Version 5.1
<#
  Configure, build, and pluginval DrumSynth Autoresearch (VoiceDSP preview VST).

  Usage:
    .\scripts\vst-build-drumsynth-autoresearch.ps1
    $env:PLUGINVAL_STRICTNESS = "8"; .\scripts\vst-build-drumsynth-autoresearch.ps1
#>
param(
    [string] $Configuration = "Release",
    [string] $Generator = "Visual Studio 17 2022"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
$Starter = Join-Path $Root "disklordz-ralph\vst\starter-cpp"
$Build = Join-Path $Starter "build"

if (-not (Test-Path (Join-Path $Starter "externals\JUCE"))) {
    Write-Error "JUCE not found under starter-cpp/externals/JUCE — clone per starter-cpp README."
}

Push-Location $Starter
try {
    cmake -S . -B build -G $Generator -A x64 `
        -DDLZ_AUTORESEARCH=ON `
        -DDLZ_AUTORESEARCH_VST=ON `
        -DDLZ_LICENSE_ED25519=ON
    cmake --build build --config $Configuration --parallel
} finally {
    Pop-Location
}

$vst3 = Join-Path $Build (Join-Path "autoresearch-cpp" (Join-Path "DrumSynthAutoresearch_artefacts" (Join-Path $Configuration (Join-Path "VST3" "DrumSynth Autoresearch.vst3"))))
if (-not (Test-Path $vst3)) {
    $alt = Join-Path $Build (Join-Path "DrumSynthAutoresearch_artefacts" (Join-Path $Configuration (Join-Path "VST3" "DrumSynth Autoresearch.vst3")))
    if (Test-Path $alt) { $vst3 = $alt }
}

if (-not (Test-Path $vst3)) {
    Write-Error "VST3 not found after build. Expected under build/autoresearch-cpp/.../DrumSynth Autoresearch.vst3"
}

$strict = if ($env:PLUGINVAL_STRICTNESS) { $env:PLUGINVAL_STRICTNESS } else { "8" }
$pluginval = $env:DLZ_PLUGINVAL
if (-not $pluginval) {
    $pluginval = (Get-Command pluginval -ErrorAction SilentlyContinue)?.Source
}
if (-not $pluginval) {
    Write-Warning "pluginval not on PATH — build OK, skipping validation."
    Write-Host "VST3: $vst3"
    exit 0
}

Write-Host "pluginval strictness=$strict on $vst3"
& $pluginval --strictness-level $strict --validate $vst3
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "DrumSynth Autoresearch: build + pluginval OK"
Write-Host "OpenMontage: run Standalone from build\...\Standalone\DrumSynth Autoresearch.exe — TRIGGER KICK + license bar"