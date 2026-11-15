# AudioStorePortal one-shot MetaGPT run (see docs/plans/metagpt-license-portal-blueprint.md)
param(
    [string]$OutputRoot = "C:\Dev\metagpt-workspace",
    [string]$ProjectName = "AudioStorePortal"
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
$promptFile = Join-Path $repoRoot "docs\plans\metagpt-audiostoreportal-prompt.txt"

$mg = Get-Command metagpt -ErrorAction SilentlyContinue
if (-not $mg) {
    Write-Host "metagpt not on PATH. pip install --upgrade metagpt && metagpt --init-config"
    exit 1
}
if (-not (Test-Path $promptFile)) {
    throw "Missing prompt: $promptFile"
}

New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null
Set-Location $OutputRoot

$prompt = Get-Content $promptFile -Raw
Write-Host "MetaGPT -> workspace/$ProjectName (long run)..."
& metagpt $prompt --project-name $ProjectName