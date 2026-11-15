#Requires -Version 5.1
<#
  One-time Serena + clangd prep for DrumKit Pro template.
  Requires: cmake, Ninja (optional), uv/uvx for Serena CLI.

  Example:
    cd C:\Dev\push\disklordz-ralph\vst\templates\drumkit-plugin
    .\setup-serena.ps1
#>
param(
    [string] $ProjectRoot = "",
    [switch] $SkipIndex
)

$ErrorActionPreference = "Stop"
if (-not $ProjectRoot) { $ProjectRoot = $PSScriptRoot }
Set-Location $ProjectRoot

Write-Host "== Serena + clangd setup: $ProjectRoot ==" -ForegroundColor Cyan

if (-not (Test-Path "build\compile_commands.json")) {
    Write-Host "Configuring CMake (Ninja + compile_commands)..."
    $ninja = Get-Command ninja -ErrorAction SilentlyContinue
    if ($ninja) {
        cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release `
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DDLZ_USE_MONOREPO_JUCE=ON
    } else {
        Write-Host "Ninja not found — using Visual Studio 17 2022 (export may need manual copy)." -ForegroundColor Yellow
        cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DDLZ_USE_MONOREPO_JUCE=ON
    }
    cmake --build build --config Release --parallel 2>&1 | Out-Null
}

if (-not (Test-Path "build\compile_commands.json") -and (Test-Path "build")) {
    Write-Host "Tip: symlink compile_commands.json to build/ if using MSVC-only generators." -ForegroundColor Yellow
}

if (-not $SkipIndex) {
    if (Get-Command uvx -ErrorAction SilentlyContinue) {
        Write-Host "Indexing project (Serena)..."
        uvx --from git+https://github.com/oraios/serena serena project index
    } else {
        Write-Host "uvx not found — skip Serena index. Install: https://docs.astral.sh/uv/" -ForegroundColor Yellow
        Write-Host "Manual: uvx --from git+https://github.com/oraios/serena serena project index"
    }
}

Write-Host "Serena memories: .serena/memories/dsp_refactoring_sop.md" -ForegroundColor Green
Write-Host "clangd database: build/compile_commands.json" -ForegroundColor Green
Write-Host "Restart Claude Code / Goose / Cursor LSP after first configure." -ForegroundColor Green