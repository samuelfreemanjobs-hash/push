#Requires -Version 5.1
<#
.SYNOPSIS
  Prints Ruflo + Claude launch checklist for DiskLordz autoresearch (does not start daemon unattended).
#>
param(
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
)

Set-Location $RepoRoot

Write-Host "== DiskLordz Ruflo autopilot checklist =="
Write-Host "1. npx ruflo@latest init (if needed)"
Write-Host "2. Copy .claude\settings.json.example -> .claude\settings.json"
Write-Host "3. scripts\ruflo-seed-memory.ps1"
Write-Host "4. CMake -DDLZ_AUTORESEARCH=ON in vst\starter-cpp"
Write-Host "5. python disklordz-ralph\vst\autoresearch-cpp\evaluate_vst.py"
Write-Host ""
Write-Host "Swarm:"
Write-Host "  npx ruflo@latest daemon start"
Write-Host "  npx ruflo@latest swarm init --topology hierarchical --max-agents 4 --strategy specialized"
Write-Host ""
Write-Host "Claude prompt: see CLAUDE.md 'Autopilot prompt' section"
Write-Host "Full doc: docs\dev\ruflo-launch.md"