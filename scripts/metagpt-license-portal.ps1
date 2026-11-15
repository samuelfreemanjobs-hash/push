# Launch MetaGPT with the DiskLordz license-portal blueprint (operator; requires metagpt + API key).
param(
    [string]$OutputRoot = "C:\Dev\metagpt-workspace"
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
$blueprint = Join-Path $repoRoot "docs\plans\metagpt-license-portal-blueprint.md"

$mg = Get-Command metagpt -ErrorAction SilentlyContinue
if (-not $mg) {
    Write-Host "metagpt not on PATH. pip install --upgrade metagpt && metagpt --init-config"
    exit 1
}

if (-not (Test-Path $blueprint)) {
    throw "Missing blueprint: $blueprint"
}

New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null
Set-Location $OutputRoot

$prompt = @"
Read the MetaGPT one-shot prompt in:
$blueprint
(section 'MetaGPT one-shot prompt')
Execute that prompt fully. Output under ./workspace/disklordz-license-portal/
"@

Write-Host "Starting MetaGPT in $OutputRoot (this may take a long time)..."
& metagpt $prompt