#Requires -Version 5.1
$ErrorActionPreference = "Stop"
$Root = if ($env:DLZ_REPO_ROOT) { $env:DLZ_REPO_ROOT } else { Split-Path $PSScriptRoot -Parent }
$Dir = Join-Path $Root "disklordz-ralph\business\deep-agents"
$Vst = Join-Path $Root "disklordz-ralph\vst\templates\drumkit-plugin"
$Py = Join-Path $Dir ".venv\Scripts\python.exe"
if (-not (Test-Path $Py)) {
    python -m venv (Join-Path $Dir ".venv")
    & (Join-Path $Dir ".venv\Scripts\pip.exe") install -r (Join-Path $Dir "requirements.txt")
}
$env:DLZ_REPO_ROOT = $Root
$env:DLZ_VST_PROJECT_ROOT = $Vst
& $Py (Join-Path $Dir "drum_deep_agent.py") @args