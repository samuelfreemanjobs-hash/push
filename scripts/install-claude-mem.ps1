# Install Claude-Mem hooks for Cursor (project-scoped when run from repo root).
# Requires: Node 22+, Bun (https://bun.sh) — claude-mem uses Bun for the worker runtime.
param(
    [ValidateSet("interactive", "defer")]
    [string] $Setup = "defer",
    [string] $Provider = ""
)

$ErrorActionPreference = "Stop"
$Root = if ($PSScriptRoot) { Split-Path $PSScriptRoot -Parent } else { (Get-Location).Path }
Set-Location $Root

Write-Host "DiskLordz — Claude-Mem install from: $Root" -ForegroundColor Cyan
Write-Host "Docs: docs/dev/claude-mem.md" -ForegroundColor DarkGray

if (-not (Get-Command bun -ErrorAction SilentlyContinue)) {
    Write-Warning "Bun not found. Install: irm bun.sh/install.ps1 | iex  (or https://bun.sh)"
    Write-Warning "Then restart PowerShell and re-run this script."
}

$args = @("claude-mem", "install", "--ide", "cursor", "--setup", $Setup)
if ($Provider) {
    $args += @("--provider", $Provider)
}

Write-Host "Running: npx $($args -join ' ')" -ForegroundColor Yellow
& npx @args

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nRestart Cursor. Then: .\scripts\check-claude-mem-health.ps1" -ForegroundColor Green
    Write-Host "Seed memory: docs/dev/claude-mem-seed.md" -ForegroundColor Green
}