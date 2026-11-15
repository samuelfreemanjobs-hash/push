# Launch Gemini CLI from repo root (reads GEMINI.md + AGENTS.md).
$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
Set-Location $Root

if (-not (Get-Command gemini -ErrorAction SilentlyContinue)) {
    Write-Host "Install: npm install -g @google/gemini-cli" -ForegroundColor Yellow
    Write-Host "Or: npx @google/gemini-cli" -ForegroundColor Yellow
    exit 1
}

Write-Host "GEMINI.md + AGENTS.md active in: $Root" -ForegroundColor Cyan
Write-Host "Prompt templates: docs/launch/prompts/gemini-*.md" -ForegroundColor DarkGray
& gemini @args