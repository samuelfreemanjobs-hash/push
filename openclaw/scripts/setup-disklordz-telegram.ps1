# DiskLordz OpenClaw + Telegram (Windows)
# Usage:
#   Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
#   cd C:\Dev\push
#   $env:DLZ_REPO_ROOT = (Get-Location).Path
#   .\openclaw\scripts\setup-disklordz-telegram.ps1
param(
    [switch]$SkipOpenClawInstall
)

$ErrorActionPreference = "Stop"
$OcScripts = Split-Path -Parent $MyInvocation.MyCommand.Path
$OcRoot = Split-Path -Parent $OcScripts
$Repo = if ($env:DLZ_REPO_ROOT) { $env:DLZ_REPO_ROOT.TrimEnd('\') } else { Split-Path -Parent $OcRoot }
$State = if ($env:OPENCLAW_STATE_DIR) { $env:OPENCLAW_STATE_DIR } else { Join-Path $env:USERPROFILE ".openclaw" }

if (-not (Test-Path (Join-Path $Repo "openclaw\SETUP-TELEGRAM.md"))) {
    throw "Not a push repo root: $Repo (expected openclaw\SETUP-TELEGRAM.md)"
}

function Test-NodeForOpenClaw {
    $v = (node -p "process.versions.node")
    $major = [int]($v -split "\.")[0]
    if ($major -lt 24) {
        Write-Host "WARN: OpenClaw requires Node 24+ or 26+. Current: $v" -ForegroundColor Red
        Write-Host "Install Node 26 LTS, reopen PowerShell, then re-run this script." -ForegroundColor Yellow
        Write-Host "Continuing workspace + config steps only..." -ForegroundColor Yellow
        return $false
    }
    return $true
}

function Refresh-OpenClawPath {
    $npmGlobal = & npm config get prefix 2>$null
    if ($npmGlobal -and (Test-Path $npmGlobal)) {
        $env:Path = "$npmGlobal;$env:Path"
    }
    $ocHome = Join-Path $env:USERPROFILE ".openclaw\bin"
    if (Test-Path $ocHome) {
        $env:Path = "$ocHome;$env:Path"
    }
}

$nodeOk = Test-NodeForOpenClaw

if (-not $SkipOpenClawInstall -and $nodeOk -and -not (Get-Command openclaw -ErrorAction SilentlyContinue)) {
    Write-Host "Installing OpenClaw..." -ForegroundColor Cyan
    iwr -useb https://openclaw.ai/install.ps1 | iex
    Refresh-OpenClawPath
}

$env:DLZ_REPO_ROOT = $Repo
$env:OPENCLAW_STATE_DIR = $State

Write-Host "DLZ_REPO_ROOT=$Repo" -ForegroundColor DarkGray
Write-Host "OPENCLAW_STATE_DIR=$State" -ForegroundColor DarkGray

& (Join-Path $OcRoot "disklordz-picks\scripts\install-pick-workspaces.ps1")
& (Join-Path $Repo "disklordz-agent-stack\deploy\bootstrap-free.ps1")
node (Join-Path $OcScripts "merge-disklordz-config.mjs")

$hasOpenClaw = $null -ne (Get-Command openclaw -ErrorAction SilentlyContinue)

if (-not $hasOpenClaw) {
    Write-Host @"

OpenClaw CLI not available yet. After Node 24+ / 26+:
  iwr -useb https://openclaw.ai/install.ps1 | iex
  openclaw onboard --classic

"@ -ForegroundColor Yellow
} elseif (-not $env:TELEGRAM_BOT_TOKEN) {
    Write-Host @"

Telegram (@BotFather):
  1) /newbot -> save token
  2) `$env:TELEGRAM_BOT_TOKEN = '123:abc...'
  3) openclaw channels add --channel telegram --token `$env:TELEGRAM_BOT_TOKEN

"@ -ForegroundColor Yellow
} else {
    openclaw channels add --channel telegram --token $env:TELEGRAM_BOT_TOKEN
}

Write-Host @"

--- Next (same PowerShell) ---
Gemini:  openclaw onboard --classic
Gateway: openclaw gateway install; openclaw gateway start
Probe:   openclaw channels status --probe
Pair:    DM bot -> openclaw pairing list telegram -> approve <CODE>
Test:    SAGE: morning brief

Merge agents/bindings/skills from:
  $State\disklordz.merged.json5
into ~/.openclaw/openclaw.json (Control UI or manual).

Docs: $OcRoot\SETUP-TELEGRAM.md
"@ -ForegroundColor Green