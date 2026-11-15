#Requires -Version 5.1
<#
  AutoGen VST R&D swarm (Windows + MSVC + pluginval).

  Example:
    $env:DLZ_REPO_ROOT = "C:\Dev\push"
    $env:OPENAI_API_KEY = "..."
    .\scripts\run-vst-rd-swarm.ps1 "Add drive SmoothedValue saturation"
#>
param(
    [string] $Task = "Add APVTS drive 0..1 with SmoothedValue saturation; audit, build, pluginval.",
    [string] $ProjectRoot = "",
    [int] $MaxTurns = 15
)

$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
if (-not $env:DLZ_REPO_ROOT) { $env:DLZ_REPO_ROOT = $Root }

if ($ProjectRoot) {
    $env:DLZ_VST_PROJECT_ROOT = $ProjectRoot
} elseif (-not $env:DLZ_VST_PROJECT_ROOT) {
    $env:DLZ_VST_PROJECT_ROOT = Join-Path $Root "disklordz-ralph\vst\templates\webview-drum-starter"
}

$Pv = Join-Path $Root "disklordz-ralph\vst\tools\pluginval\pluginval.exe"
if ((Test-Path $Pv) -and -not $env:DLZ_PLUGINVAL) { $env:DLZ_PLUGINVAL = $Pv }

$Dir = Join-Path $Root "disklordz-ralph\business\ms-agent-framework"
$Py = Join-Path $Dir ".venv\Scripts\python.exe"
if (-not (Test-Path $Py)) {
    & (Join-Path $Root "scripts\install-ms-agent-framework.ps1")
}

Push-Location $Dir
try {
    & $Py (Join-Path $Dir "vst_rd_swarm.py") --max-turns $MaxTurns $Task
} finally {
    Pop-Location
}