#Requires -Version 5.1
<#
  Boot a 3-pane Herdr session for DiskLordz (DSP | factory | QA).
  Requires: herdr CLI on PATH (https://herdr.dev/)

  Example:
    $env:DLZ_REPO_ROOT = "C:\Dev\push"
    .\scripts\start-herdr-disklordz-studio.ps1
#>
param(
    [string] $Session = "",
    [switch] $DryRun
)

$ErrorActionPreference = "Stop"
$Root = if ($env:DLZ_REPO_ROOT) { $env:DLZ_REPO_ROOT } else { Split-Path $PSScriptRoot -Parent }
if (-not $Session) { $Session = $(if ($env:DLZ_HERDR_SESSION) { $env:DLZ_HERDR_SESSION } else { "DiskLordzStudio" }) }

$VstRoot = $(if ($env:DLZ_VST_PROJECT_ROOT) { $env:DLZ_VST_PROJECT_ROOT } else {
    Join-Path $Root "disklordz-ralph\vst\templates\drumkit-plugin"
})
$FactoryRoot = $(if ($env:DLZ_AUDIO_FACTORY_ROOT) { $env:DLZ_AUDIO_FACTORY_ROOT } else {
    Join-Path $Root "disklordz-ralph\factory\audio-factory"
})
$DspAgent = $(if ($env:DLZ_HERDR_DSP_AGENT) { $env:DLZ_HERDR_DSP_AGENT } else { "claude" })

if (-not (Get-Command herdr -ErrorAction SilentlyContinue)) {
    Write-Error "herdr not found. Install: irm https://herdr.dev/install.ps1 | iex"
}

$cmds = @(
    "herdr new -s `"$Session`" -d `"$Root`"",
    "herdr send -t `"${Session}:0.0`" `"Set-Location '$VstRoot'; Write-Host DSP pane — vst_rd_swarm.py or $DspAgent`" Enter",
    "herdr split -h -t `"${Session}:0.0`"",
    "herdr send -t `"${Session}:0.1`" `"Set-Location '$FactoryRoot'; Write-Host Factory pane — python factory_swarm.py`" Enter",
    "herdr split -v -t `"${Session}:0.1`"",
    "herdr send -t `"${Session}:0.2`" `"Set-Location '$Root'; Write-Host QA pane — .\scripts\vst-build-pluginval-gate.ps1`" Enter"
)

foreach ($c in $cmds) {
    Write-Host ">> $c" -ForegroundColor Cyan
    if (-not $DryRun) { Invoke-Expression $c }
}

Write-Host ""
Write-Host "Prefer one-click: disklordz-ralph\vst\templates\drumkit-plugin\launch-studio.ps1"
Write-Host "Coordination: drumkit-plugin\AGENTS.md + config\herdr\AGENTS-herdr-coordination.md"