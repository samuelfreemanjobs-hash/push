# Wrapper: run from repo root after clone
param([switch]$InstallNode, [switch]$SkipGateway, [switch]$PromptSecrets)
$Root = $PSScriptRoot
$env:DLZ_REPO_ROOT = $Root
& (Join-Path $Root "openclaw\scripts\Complete-Windows-OpenClaw.ps1") @PSBoundParameters