# DiskLordz business Ralph loop — run from repo root (e.g. C:\Dev\push)
$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $RepoRoot

$MaxIterations = 40
$LogFile = Join-Path $PSScriptRoot "ralph_output.log"
$PromptPath = Join-Path $PSScriptRoot "PROMPT.md"

if (-not (Get-Command claude -ErrorAction SilentlyContinue)) {
    Write-Host "claude CLI not found. Install Claude Code or run stories via Cursor Agent using PROMPT.md" -ForegroundColor Yellow
    exit 1
}

if (Test-Path $LogFile) { Remove-Item $LogFile }

$env:DLZ_REPO_ROOT = $RepoRoot

for ($i = 1; $i -le $MaxIterations; $i++) {
    Write-Host "`n=== DiskLordz Ralph $i / $MaxIterations ===" -ForegroundColor Cyan
    node (Join-Path $PSScriptRoot "scripts\sync-priority-hint.mjs")
    $Prompt = Get-Content -Path $PromptPath -Raw
    $Output = claude -p $Prompt --dangerously-skip-permissions 2>&1 | Tee-Object -FilePath $LogFile -Append

    if ($Output -match "<promise>COMPLETE</promise>") {
        Write-Host "`nSUCCESS: All DiskLordz Ralph stories complete." -ForegroundColor Green
        break
    }
    Start-Sleep -Seconds 2
}