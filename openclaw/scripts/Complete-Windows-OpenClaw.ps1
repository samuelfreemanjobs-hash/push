# DiskLordz — one-shot Windows OpenClaw + Telegram automation
# Paste in PowerShell (Admin optional; needed for winget Node install):
#
#   Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
#   cd C:\Dev\push
#   git pull
#   $env:DLZ_REPO_ROOT = (Get-Location).Path
#   .\openclaw\scripts\Complete-Windows-OpenClaw.ps1
#
param(
    [switch]$InstallNode,
    [switch]$SkipGateway,
    [switch]$PromptSecrets
)

$ErrorActionPreference = "Stop"
$OcScripts = Split-Path -Parent $MyInvocation.MyCommand.Path
$OcRoot = Split-Path -Parent $OcScripts
$Repo = if ($env:DLZ_REPO_ROOT) { $env:DLZ_REPO_ROOT.TrimEnd('\') } else { Split-Path -Parent $OcRoot }
$State = if ($env:OPENCLAW_STATE_DIR) { $env:OPENCLAW_STATE_DIR } else { Join-Path $env:USERPROFILE ".openclaw" }
$LocalSecrets = Join-Path $State "disklordz.local.ps1"

function Test-NodeMajor24 {
    $v = (node -p "process.versions.node")
    return [int]($v -split "\.")[0] -ge 24
}

function Install-NodeIfRequested {
    if (Test-NodeMajor24) { return }
    if (-not $InstallNode) {
        Write-Host "Node 24+ required. Re-run with -InstallNode or install from https://nodejs.org/" -ForegroundColor Red
        throw "Node version too old: $(node -v)"
    }
    if (Get-Command winget -ErrorAction SilentlyContinue) {
        Write-Host "Installing Node.js LTS via winget..." -ForegroundColor Cyan
        winget install OpenJS.NodeJS.LTS --accept-package-agreements --accept-source-agreements
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
    } else {
        throw "winget not found — install Node 26 manually, reopen PowerShell, re-run."
    }
    if (-not (Test-NodeMajor24)) { throw "Node still < 24 after install — open a new terminal." }
}

function Refresh-OpenClawPath {
    $npmGlobal = & npm config get prefix 2>$null
    if ($npmGlobal -and (Test-Path $npmGlobal)) { $env:Path = "$npmGlobal;$env:Path" }
    $ocHome = Join-Path $env:USERPROFILE ".openclaw\bin"
    if (Test-Path $ocHome) { $env:Path = "$ocHome;$env:Path" }
}

function Load-LocalSecrets {
    if (Test-Path $LocalSecrets) {
        Write-Host "Loading $LocalSecrets" -ForegroundColor DarkGray
        . $LocalSecrets
    }
}

function Ensure-Secrets {
    if ($PromptSecrets -or (-not $env:TELEGRAM_BOT_TOKEN) -or (-not $env:GEMINI_API_KEY)) {
        if (-not $env:TELEGRAM_BOT_TOKEN) {
            $t = Read-Host "Telegram bot token (@BotFather) — leave blank to skip"
            if ($t) { $env:TELEGRAM_BOT_TOKEN = $t.Trim() }
        }
        if (-not $env:GEMINI_API_KEY) {
            $g = Read-Host "Gemini API key (Google AI Studio) — leave blank to skip"
            if ($g) { $env:GEMINI_API_KEY = $g.Trim() }
        }
    }
}

# --- main ---
if (-not (Test-Path (Join-Path $Repo "openclaw\SETUP-TELEGRAM.md"))) {
    throw "Clone push repo to C:\Dev\push (or set DLZ_REPO_ROOT)"
}

Install-NodeIfRequested
Load-LocalSecrets
Ensure-Secrets

$env:DLZ_REPO_ROOT = $Repo
$env:OPENCLAW_STATE_DIR = $State
if ($env:GEMINI_API_KEY) { $env:GOOGLE_GENERATIVE_AI_API_KEY = $env:GEMINI_API_KEY }

& (Join-Path $OcScripts "setup-disklordz-telegram.ps1") -SkipOpenClawInstall:$false

Push-Location (Join-Path $OcScripts)
npm install --silent 2>$null
if ($LASTEXITCODE -ne 0) { npm install }
Pop-Location

node (Join-Path $OcScripts "apply-disklordz-config.mjs")

if (-not (Get-Command openclaw -ErrorAction SilentlyContinue)) {
    Refresh-OpenClawPath
}
if (-not (Get-Command openclaw -ErrorAction SilentlyContinue)) {
    throw "openclaw CLI missing after install — reopen PowerShell and re-run."
}

if ($env:GEMINI_API_KEY) {
    openclaw config set models.providers.google.apiKey $env:GEMINI_API_KEY 2>$null
}
if ($env:TELEGRAM_BOT_TOKEN) {
    openclaw channels add --channel telegram --token $env:TELEGRAM_BOT_TOKEN
}

if (-not (Test-Path $LocalSecrets)) {
    @"
# Copy to disklordz.local.ps1 and re-run Complete-Windows-OpenClaw.ps1 (no -PromptSecrets)
# `$env:TELEGRAM_BOT_TOKEN = 'from BotFather'
# `$env:GEMINI_API_KEY = 'from aistudio.google.com'
"@ | Out-File -FilePath (Join-Path $State "disklordz.local.ps1.example") -Encoding utf8
}

if (-not $SkipGateway) {
    openclaw gateway install 2>$null
    openclaw gateway start
    Start-Sleep -Seconds 3
    openclaw gateway status
    openclaw channels status --probe 2>$null
}

$pairFile = Join-Path $State "PAIRING-NEXT.txt"
@"
DiskLordz OpenClaw — finish on phone
====================================
1) Telegram: open your bot, send any message
2) PowerShell: openclaw pairing list telegram
3) openclaw pairing approve telegram <CODE>
4) Test: SAGE: morning brief

Config: $State\openclaw.json
Logs:   openclaw logs --follow
Triage: openclaw triage
"@ | Out-File -FilePath $pairFile -Encoding utf8

Write-Host "`n=== DiskLordz OpenClaw automation complete ===" -ForegroundColor Green
Write-Host "Read: $pairFile" -ForegroundColor Cyan
if (-not $env:TELEGRAM_BOT_TOKEN) {
    Write-Host "Telegram token was skipped — set disklordz.local.ps1 and re-run." -ForegroundColor Yellow
}