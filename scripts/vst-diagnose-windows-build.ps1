#Requires -Version 5.1
<#
.SYNOPSIS
  Pre-flight checks before first JUCE + CMake build on Windows.
#>
param(
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
)

$ErrorActionPreference = "Continue"
$Starter = Join-Path $RepoRoot "disklordz-ralph\vst\starter-cpp"
$JuceCmake = Join-Path $Starter "externals\JUCE\CMakeLists.txt"
$MonoC = Join-Path $Starter "Modules\License\crypto\monocypher.c"
$fail = 0

function Test-Ok($label, [bool]$ok, [string]$hint) {
    if ($ok) {
        Write-Host "[OK]   $label" -ForegroundColor Green
    } else {
        Write-Host "[FAIL] $label" -ForegroundColor Red
        if ($hint) { Write-Host "       $hint" -ForegroundColor Yellow }
        $script:fail++
    }
}

Write-Host "== DiskLordz VST build diagnostics =="
Write-Host "Repo: $RepoRoot"
Write-Host ""

$cmakeVer = & cmake --version 2>$null
Test-Ok "cmake on PATH" ($LASTEXITCODE -eq 0) "Install CMake 3.22+"
if ($cmakeVer) { Write-Host "       $($cmakeVer[0])" }

$cl = Get-Command cl.exe -ErrorAction SilentlyContinue
Test-Ok "MSVC cl.exe" ($null -ne $cl) "Use Developer PowerShell for VS 2022"
if ($cl) { Write-Host "       $($cl.Source)" }

Test-Ok "starter-cpp exists" (Test-Path $Starter) ""
Test-Ok "JUCE CMakeLists.txt" (Test-Path $JuceCmake) @"
       git clone --depth 1 --branch 8.0.4 https://github.com/juce-framework/JUCE.git externals\JUCE
       (from starter-cpp folder — see docs/dev/juce-cmake-windows-troubleshooting.md)
"@

Test-Ok "Monocypher C source" (Test-Path $MonoC) "Run scripts\vendor-monocypher.ps1"

$headless = Join-Path $Starter "..\autoresearch-cpp\Source\VoiceDSP.h"
Test-Ok "VoiceDSP.h sandbox" (Test-Path $headless) ""

$pv = Join-Path $RepoRoot "disklordz-ralph\vst\tools\pluginval\pluginval.exe"
$pvPath = Get-Command pluginval -ErrorAction SilentlyContinue
Test-Ok "pluginval (optional)" ((Test-Path $pv) -or ($null -ne $pvPath)) "See vst/WINDOWS-BUILD.md"

Write-Host ""
if ($fail -eq 0) {
    Write-Host "All required checks passed. Suggested configure:"
    Write-Host "  cd disklordz-ralph\vst\starter-cpp"
    Write-Host "  cmake -S . -B build -G `"Visual Studio 17 2022`" -A x64 -DDLZ_AUTORESEARCH=ON -DDLZ_AUTORESEARCH_VST=ON"
    exit 0
}

Write-Host "$fail check(s) failed — fix above before cmake." -ForegroundColor Red
Write-Host "Full guide: docs\dev\juce-cmake-windows-troubleshooting.md"
exit 1