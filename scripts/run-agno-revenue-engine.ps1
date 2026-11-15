#Requires -Version 5.1
$ErrorActionPreference = "Stop"
$Root = if ($env:DLZ_REPO_ROOT) { $env:DLZ_REPO_ROOT } else { Split-Path $PSScriptRoot -Parent }
$Dir = Join-Path $Root "disklordz-ralph\business\agno-revenue-engine"
$Py = Join-Path $Dir ".venv\Scripts\python.exe"
if (-not (Test-Path $Py)) {
    python -m venv (Join-Path $Dir ".venv")
    & (Join-Path $Dir ".venv\Scripts\pip.exe") install -r (Join-Path $Dir "requirements.txt")
}
$env:DLZ_REPO_ROOT = $Root
Push-Location $Dir
try {
    & $Py (Join-Path $Dir "agno_revenue_engine.py")
} finally {
    Pop-Location
}