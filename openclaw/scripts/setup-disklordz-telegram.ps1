# DiskLordz OpenClaw + Telegram (Windows)
$ErrorActionPreference = "Stop"
$OcScripts = Split-Path -Parent $MyInvocation.MyCommand.Path
$OcRoot = Split-Path -Parent $OcScripts
$Repo = if ($env:DLZ_REPO_ROOT) { $env:DLZ_REPO_ROOT } else { Split-Path -Parent $OcRoot }
$State = if ($env:OPENCLAW_STATE_DIR) { $env:OPENCLAW_STATE_DIR } else { Join-Path $env:USERPROFILE ".openclaw" }

function Test-NodeForOpenClaw {
    $v = (node -p "process.versions.node")
    $major = [int]($v -split "\.")[0]
    if ($major -lt 24) {
        throw "OpenClaw needs Node 24+ or 26+. Current: $v — install from https://nodejs.org/"
    }
}

if (-not (Get-Command openclaw -ErrorAction SilentlyContinue)) {
    Write-Host "Installing OpenClaw..." -ForegroundColor Cyan
    iwr -useb https://openclaw.ai/install.ps1 | iex
}

Test-NodeForOpenClaw

$env:DLZ_REPO_ROOT = $Repo
$env:OPENCLAW_STATE_DIR = $State

New-Item -ItemType Directory -Force -Path $State, "$State\hermes-out\research", "$State\workspace-shared\vault-docs" | Out-Null

# Workspaces + skills (Git Bash or WSL if bash missing)
if (Get-Command bash -ErrorAction SilentlyContinue) {
    bash "$OcRoot\disklordz-picks\scripts\install-pick-workspaces.sh"
    bash "$Repo\disklordz-agent-stack\deploy\bootstrap-free.sh"
} else {
    Write-Warning "Install Git Bash; then run install-pick-workspaces.sh manually"
}

node "$OcScripts\merge-disklordz-config.mjs"

if (-not $env:TELEGRAM_BOT_TOKEN) {
    Write-Host @"

Telegram (@BotFather):
  1) /newbot → save token
  2) `$env:TELEGRAM_BOT_TOKEN = '123:abc...'
  3) openclaw channels add --channel telegram --token `$env:TELEGRAM_BOT_TOKEN

"@ -ForegroundColor Yellow
} else {
    openclaw channels add --channel telegram --token $env:TELEGRAM_BOT_TOKEN
}

Write-Host @"
Gemini (free): openclaw onboard --classic → Google AI Studio key

Gateway:
  openclaw gateway install
  openclaw gateway start
  openclaw channels status --probe

Pairing: message bot → openclaw pairing list telegram → approve <CODE>

Test: SAGE: morning brief

Draft config: $State\disklordz.merged.json5
Docs: $OcRoot\SETUP-TELEGRAM.md
"@ -ForegroundColor Green