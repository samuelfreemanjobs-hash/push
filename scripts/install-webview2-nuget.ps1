#Requires -Version 5.1
# WebView2 SDK for JUCE NEEDS_WEBVIEW2 + static linking (operator, once per machine).

$ErrorActionPreference = "Stop"
$nuget = Get-Command nuget.exe -ErrorAction SilentlyContinue
if (-not $nuget) {
    Write-Error "Install NuGet CLI: winget install Microsoft.NuGet"
}

$out = Join-Path $env:USERPROFILE "AppData\Local\PackageManagement\NuGet\Packages"
New-Item -ItemType Directory -Force -Path $out | Out-Null
& nuget.exe install Microsoft.Web.WebView2 -OutputDirectory $out -NonInteractive
Write-Host "WebView2 SDK installed under $out"
Write-Host "Configure CMake from webview-drum-starter README."