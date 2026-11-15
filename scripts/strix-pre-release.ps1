# Opt-in Strix pre-release scans (requires Docker + strix-agent on operator PATH).
param(
    [switch]$Quick,
    [string]$StagingUrl = ""
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $repoRoot

$strix = Get-Command strix -ErrorAction SilentlyContinue
if (-not $strix) {
    Write-Host "strix not on PATH. Install: pipx install strix-agent (see docs/dev/strix-opt-in.md)"
    exit 1
}

$playbook = Join-Path $repoRoot "docs\security\strix-vst-audit-playbook.md"
if (Test-Path $playbook) {
    Write-Host "Playbook: $playbook"
}

if ($Quick) {
    & strix -n -t (Join-Path $repoRoot "disklordz-ralph\vst\starter-cpp\Source") --scan-mode quick
    & strix -n -t (Join-Path $repoRoot "storefront") --scan-mode quick
    & strix -n -t (Join-Path $repoRoot "disklordz-ralph\business\openbb") --scan-mode quick
}

if ($StagingUrl) {
    & strix --target $StagingUrl --scan-mode standard
}

if (-not $Quick -and -not $StagingUrl) {
    Write-Host "Usage: .\scripts\strix-pre-release.ps1 -Quick"
    Write-Host "       .\scripts\strix-pre-release.ps1 -StagingUrl https://staging.example.com"
}