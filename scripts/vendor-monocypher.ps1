# Refresh vendored Monocypher in starter-cpp (operator).
$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
$dest = Join-Path $repoRoot "disklordz-ralph\vst\starter-cpp\Modules\License\crypto"
$base = "https://raw.githubusercontent.com/LoupVaillant/Monocypher/master/src"

New-Item -ItemType Directory -Force -Path $dest | Out-Null
Invoke-WebRequest "$base/monocypher.h" -OutFile (Join-Path $dest "monocypher.h")
Invoke-WebRequest "$base/monocypher.c" -OutFile (Join-Path $dest "monocypher.c")
Write-Host "Updated Monocypher in $dest"