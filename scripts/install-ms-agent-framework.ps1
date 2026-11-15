#Requires -Version 5.1
$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
$Dir = Join-Path $Root "disklordz-ralph\business\ms-agent-framework"
$Venv = Join-Path $Dir ".venv"

Push-Location $Dir
try {
    if (-not (Test-Path $Venv)) {
        python -m venv .venv
    }
    & .\.venv\Scripts\pip.exe install -U pip
    & .\.venv\Scripts\pip.exe install -r requirements.txt
    Write-Host "Installed AutoGen stack in $Venv"
    Write-Host "Copy .env.example to .env and set OPENAI_API_KEY"
    Write-Host "Run: .\.venv\Scripts\python.exe run_vst_rd_team.py `"your task`""
} finally {
    Pop-Location
}