# Stage latest autoresearch pack for OpenMontage (sibling marketing-studio/).
# Usage:
#   .\scripts\handoff-pack-to-openmontage.ps1
#   .\scripts\handoff-pack-to-openmontage.ps1 -PackDir "disklordz-ralph\factory\autoresearch\outputs\packs\..."
#   $env:OPENMONTAGE_ROOT = "C:\Dev\marketing-studio"

param(
    [string]$PackDir = "",
    [string]$MarketingRoot = ""
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $repoRoot

$py = Get-Command python -ErrorAction SilentlyContinue
if (-not $py) {
    $py = Get-Command python3 -ErrorAction SilentlyContinue
}
if (-not $py) {
    throw "python or python3 not found on PATH"
}

$handoff = Join-Path $repoRoot "disklordz-ralph\factory\autoresearch\publish_to_openmontage.py"
$args = @($handoff.Path)
if ($PackDir) {
    $args += @("--pack-dir", $PackDir)
}
if ($MarketingRoot) {
    $args += @("--marketing-root", $MarketingRoot)
}

& $py.Source @args