# Install Impeccable hooks for Cursor (project scope).
$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
Set-Location $Root

Write-Host "Impeccable install in: $Root" -ForegroundColor Cyan
Write-Host "Seeded: PRODUCT.md, DESIGN.md — run /impeccable audit on storefront/ after install" -ForegroundColor DarkGray

& npx impeccable install --providers=cursor --scope=project

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nReload Cursor. Try: /impeccable craft on storefront/disklordz-lite/" -ForegroundColor Green
}