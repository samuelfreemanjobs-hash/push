#Requires -Version 5.1
<#
.SYNOPSIS
  Release build + pluginval gate for Detroit Phonk Trap Maker (Windows).

.EXAMPLE
  .\scripts\vst-build-pluginval-gate.ps1
  $env:PLUGINVAL_STRICTNESS = "8"; .\scripts\vst-build-pluginval-gate.ps1 -Configuration Release
#>
param(
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path,
    [string]$Configuration = "Release",
    [int]$Strictness = $(if ($env:PLUGINVAL_STRICTNESS) { [int]$env:PLUGINVAL_STRICTNESS } else { 5 })
)

$ErrorActionPreference = "Stop"
$Starter = Join-Path $RepoRoot "disklordz-ralph\vst\starter-cpp"
$BuildDir = Join-Path $Starter "build"
$PluginVal = Join-Path $RepoRoot "disklordz-ralph\vst\tools\pluginval\pluginval.exe"

if (-not (Test-Path $Starter)) {
    Write-Error "starter-cpp not found at $Starter"
}

Write-Host "== CMake configure ($Configuration) =="
cmake -S $Starter -B $BuildDir -G "Visual Studio 17 2022" -A x64
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "== Build =="
cmake --build $BuildDir --config $Configuration --parallel
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$Vst3 = Join-Path $BuildDir "DetroitPhonkTrapMaker_artefacts\$Configuration\VST3\Detroit Phonk Trap Maker.vst3"
if (-not (Test-Path $Vst3)) {
    Write-Error "VST3 not found: $Vst3"
}

Write-Host "== Catch2 (if enabled) =="
ctest --test-dir $BuildDir -C $Configuration --output-on-failure
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if (-not (Test-Path $PluginVal)) {
    Write-Warning "pluginval not found at $PluginVal — download per vst/WINDOWS-BUILD.md"
    exit 2
}

Write-Host "== pluginval strictness $Strictness =="
& $PluginVal --strictness-level $Strictness --validate $Vst3
if ($LASTEXITCODE -ne 0) {
    Write-Error "pluginval FAILED — revert agent DSP changes"
}

Write-Host "GATE PASS: $Vst3"
exit 0