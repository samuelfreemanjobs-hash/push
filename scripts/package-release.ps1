#Requires -Version 5.1
param(
    [string] $Version = "1.0.0",
    [switch] $IncludeDrumSynth,
    [switch] $NoShipVst
)

$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
$Script = Join-Path $Root "disklordz-ralph\business\release\package_release.py"

$argsList = @($Script, "--version", $Version)
if ($IncludeDrumSynth) { $argsList += "--include-drumsynth-vst" }
if ($NoShipVst) { $argsList += "--no-ship-vst" }

& python @argsList
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }