#Requires -Version 5.1
param(
    [ValidateSet("cpp", "kit")]
    [string] $Track = "cpp",
    [switch] $DryRun
)

$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
$Script = Join-Path $Root "disklordz-ralph\business\mem0\record_autoresearch_win.py"

if (-not $env:MEM0_API_KEY) {
    Write-Error "MEM0_API_KEY not set. See docs/dev/mem0-opt-in.md"
}

if (-not $env:DLZ_MEM0_GIT_SHA) {
    $env:DLZ_MEM0_GIT_SHA = (git -C $Root rev-parse --short HEAD 2>$null)
}

$argsList = @($Script, "--track", $Track)
if ($DryRun) { $argsList += "--dry-run" }

& python @argsList