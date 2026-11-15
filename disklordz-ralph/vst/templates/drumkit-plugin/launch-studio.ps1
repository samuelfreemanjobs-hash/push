#Requires -Version 5.1
<#
  One-click Herdr VST + Sample Pack Control Room.

  Copy layout:
    Copy-Item ..\..\..\..\..\config\herdr\layouts\vst-studio.json `
      "$env:USERPROFILE\.config\herdr-app\layouts\"

  Example:
    cd C:\Dev\push\disklordz-ralph\vst\templates\drumkit-plugin
    .\launch-studio.ps1
    .\launch-studio.ps1 -WatchRawAudio
#>
param(
    [string] $SessionName = "DrumKitStudio",
    [string] $ProjectDir = "",
    [switch] $WatchRawAudio,
    [switch] $NoAttach
)

$ErrorActionPreference = "Stop"
if (-not $ProjectDir) { $ProjectDir = $PSScriptRoot }
$ProjectDir = (Resolve-Path $ProjectDir).Path

$RepoRoot = $ProjectDir
for ($i = 0; $i -lt 8; $i++) {
    if (Test-Path (Join-Path $RepoRoot "disklordz-ralph\factory\audio-factory")) { break }
    $parent = Split-Path $RepoRoot -Parent
    if (-not $parent -or $parent -eq $RepoRoot) { break }
    $RepoRoot = $parent
}
$FactoryRoot = Join-Path $RepoRoot "disklordz-ralph\factory\audio-factory"
$RawAudio = Join-Path $FactoryRoot "raw_audio"
$LayoutSrc = Join-Path $RepoRoot "config\herdr\layouts\vst-studio.json"
$LayoutDst = Join-Path $env:USERPROFILE ".config\herdr-app\layouts\vst-studio.json"

if (-not (Get-Command herdr -ErrorAction SilentlyContinue)) {
    Write-Error "herdr not on PATH. Install: powershell -ExecutionPolicy Bypass -c ""irm https://herdr.dev/install.ps1 | iex"""
}

if (Test-Path $LayoutSrc) {
    New-Item -ItemType Directory -Force (Split-Path $LayoutDst) | Out-Null
    Copy-Item $LayoutSrc $LayoutDst -Force
    Write-Host "Installed layout -> $LayoutDst"
}

$env:DLZ_REPO_ROOT = $RepoRoot
$env:DLZ_VST_PROJECT_ROOT = $ProjectDir
$env:DLZ_AUDIO_FACTORY_ROOT = $FactoryRoot

# 1. Detached session
herdr new -s $SessionName -d $ProjectDir

# 2. Pane 0 — DSP
$msg0 = "Set-Location '$ProjectDir'; claude 'Read AGENTS.md. You are Pane 0 (DSP_Architect). Use Context7 before JUCE edits.'"
herdr send -t "${SessionName}:0.0" $msg0 Enter

# 3. Pane 1 — Factory
herdr split -h -t "${SessionName}:0.0"
$msg1 = "Set-Location '$FactoryRoot'; goose session"
herdr send -t "${SessionName}:0.1" $msg1 Enter
Start-Sleep -Seconds 2
herdr send -t "${SessionName}:0.1" "Read $ProjectDir\AGENTS.md. You are Pane 1 (Factory_Runner). Await QA sign-off before SKU export." Enter

# 4. Pane 2 — QA
herdr split -v -t "${SessionName}:0.1"
$msg2 = "Set-Location '$ProjectDir'; Write-Host 'Pane 2: cmake Release + pluginval (see AGENTS.md)'"
herdr send -t "${SessionName}:0.2" $msg2 Enter

# 5. Optional watcher — new WAV in raw_audio pings Factory pane
if ($WatchRawAudio) {
    if (-not (Test-Path $RawAudio)) {
        New-Item -ItemType Directory -Force $RawAudio | Out-Null
    }
    $scriptBlock = {
        param($Session, $WatchPath, $Target)
        $w = New-Object System.IO.FileSystemWatcher $WatchPath, "*.wav"
        $w.IncludeSubdirectories = $false
        $w.EnableRaisingEvents = $true
        Register-ObjectEvent $w Created -Action {
            $name = $Event.SourceEventArgs.Name
            $line = "HANDOFF_FACTORY:presets New raw WAV: $name — run process_and_master_sample_pack"
            herdr send -t $using:Target $line Enter
        } | Out-Null
        while ($true) { Start-Sleep -Seconds 5 }
    }
    Start-Job -ScriptBlock $scriptBlock -ArgumentList $SessionName, $RawAudio, "${SessionName}:0.1" | Out-Null
    Write-Host "Watching $RawAudio -> ${SessionName}:0.1"
}

Write-Host ""
Write-Host "Session: $SessionName | Project: $ProjectDir"
Write-Host "Factory: $FactoryRoot | AGENTS.md in plugin root"

if (-not $NoAttach) {
    herdr attach -t $SessionName
}