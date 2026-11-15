#Requires -Version 5.1
<#
.SYNOPSIS
  Ruflo kit-producer helper: render kick at MIDI velocities 40, 85, 127 via HeadlessRender.
#>
param(
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path,
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"
$Build = Join-Path $RepoRoot "disklordz-ralph\vst\starter-cpp\build"
$Headless = Join-Path $Build "autoresearch-cpp\HeadlessRender_artefacts\$Configuration\HeadlessRender.exe"
if (-not (Test-Path $Headless)) {
    $Headless = Join-Path $Build "autoresearch-cpp\$Configuration\HeadlessRender.exe"
}
if (-not (Test-Path $Headless)) {
    Write-Error "Build HeadlessRender first: cmake -DDLZ_AUTORESEARCH=ON in starter-cpp"
}

$OutDir = Join-Path $RepoRoot "disklordz-ralph\vst\autoresearch-cpp\outputs\kits"
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$Autoresearch = Join-Path $RepoRoot "disklordz-ralph\vst\autoresearch-cpp"
Push-Location $Autoresearch

foreach ($vel in @(40, 85, 127)) {
    $norm = [math]::Round($vel / 127.0, 4)
    $out = "outputs/kits/kick_vel${vel}.wav"
    & $Headless $out $norm 36
    Write-Host "Wrote $out (velocity $vel)"
}

Pop-Location
Write-Host "Kit export done: $OutDir"