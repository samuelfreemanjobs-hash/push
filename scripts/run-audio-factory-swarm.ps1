#Requires -Version 5.1
param(
    [string] $Task = "",
    [int] $MaxTurns = 18
)

$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
if (-not $env:DLZ_REPO_ROOT) { $env:DLZ_REPO_ROOT = $Root }

$Factory = Join-Path $Root "disklordz-ralph\factory\audio-factory"
$env:DLZ_AUDIO_FACTORY_ROOT = $Factory

$Dir = Join-Path $Root "disklordz-ralph\business\ms-agent-framework"
$Py = Join-Path $Dir ".venv\Scripts\python.exe"
if (-not (Test-Path $Py)) {
    & (Join-Path $Root "scripts\install-ms-agent-framework.ps1")
}

Push-Location $Factory
try {
    $argsList = @("factory_swarm.py", "--max-turns", $MaxTurns)
    if ($Task) { $argsList += $Task }
    & $Py @argsList
} finally {
    Pop-Location
}