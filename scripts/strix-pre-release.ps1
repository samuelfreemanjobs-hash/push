# Opt-in Strix pre-release scans (Docker + strix-agent on operator PATH).
param(
    [switch]$Quick,
    [switch]$Standard,
    [switch]$Deep,
    [string]$StagingUrl = "",
    [string]$InstructionFile = ""
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $repoRoot

$strix = Get-Command strix -ErrorAction SilentlyContinue
if (-not $strix) {
    Write-Host "strix not on PATH. Install: pipx install strix-agent (see docs/dev/strix-opt-in.md)"
    exit 1
}

if (-not $InstructionFile) {
    $InstructionFile = Join-Path $repoRoot "skills\vst_audio_security.md"
}
if (-not (Test-Path $InstructionFile)) {
    Write-Host "Missing instruction file: $InstructionFile"
    exit 1
}

$sourceTarget = Join-Path $repoRoot "disklordz-ralph\vst\starter-cpp"
$playbook = Join-Path $repoRoot "docs\security\strix-vst-audit-playbook.md"
Write-Host "Instruction: $InstructionFile"
if (Test-Path $playbook) { Write-Host "Playbook:   $playbook" }

function Invoke-StrixScan {
    param([string]$Target, [string]$Mode)
    $args = @("-n", "--target", $Target, "--instruction-file", $InstructionFile, "--scan-mode", $Mode)
    & strix @args
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

if ($Quick) {
    Invoke-StrixScan -Target (Join-Path $repoRoot "disklordz-ralph\vst\starter-cpp\Source") -Mode "quick"
    Invoke-StrixScan -Target (Join-Path $repoRoot "storefront") -Mode "quick"
    Invoke-StrixScan -Target (Join-Path $repoRoot "disklordz-ralph\business\openbb") -Mode "quick"
}

if ($Standard) {
    Invoke-StrixScan -Target $sourceTarget -Mode "standard"
    if ($StagingUrl) {
        Invoke-StrixScan -Target $StagingUrl -Mode "standard"
    }
}

if ($Deep) {
    if (-not $StagingUrl) {
        Write-Host "-Deep requires -StagingUrl https://staging.example.com"
        exit 1
    }
    Invoke-StrixScan -Target $StagingUrl -Mode "deep"
}

if (-not $Quick -and -not $Standard -and -not $Deep) {
    @"
Usage:
  .\scripts\strix-pre-release.ps1 -Quick
  .\scripts\strix-pre-release.ps1 -Standard [-StagingUrl https://staging.example.com]
  .\scripts\strix-pre-release.ps1 -Deep -StagingUrl https://staging.example.com

Custom skill: skills\vst_audio_security.md
"@ | Write-Host
}