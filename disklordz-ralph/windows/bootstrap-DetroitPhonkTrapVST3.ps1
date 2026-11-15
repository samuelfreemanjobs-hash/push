# VST sub-workspace for DiskLordz Ralph (DLZ-VST-*). Business loop stays in push repo.
$VstRoot = "C:\Dev\DetroitPhonkTrapVST3"
$PushRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)

New-Item -ItemType Directory -Force -Path "$VstRoot\Source" | Out-Null
Set-Location $VstRoot
if (-not (Test-Path ".git")) { git init }

@"
build/
externals/
tools/
*.log
*.wav
"@ | Out-File -FilePath ".gitignore" -Encoding utf8
New-Item -ItemType File -Force -Path "progress.txt" | Out-Null

if (-not (Test-Path "externals\JUCE")) {
  git clone --depth 1 --branch 8.0.4 https://github.com/juce-framework/JUCE externals/JUCE
}

if (-not (Test-Path "tools\pluginval\pluginval.exe")) {
  New-Item -ItemType Directory -Force -Path "tools" | Out-Null
  $pv = "https://github.com/Tracktion/pluginval/releases/latest/download/pluginval_windows_amd64.zip"
  Invoke-WebRequest -Uri $pv -OutFile "tools\pluginval.zip"
  Expand-Archive -Path "tools\pluginval.zip" -DestinationPath "tools\pluginval_unpack" -Force
  Get-ChildItem -Path "tools\pluginval_unpack" -Recurse -Filter "pluginval.exe" |
    Select-Object -First 1 |
    ForEach-Object {
      New-Item -ItemType Directory -Force -Path "tools\pluginval" | Out-Null
      Copy-Item $_.FullName "tools\pluginval\pluginval.exe"
    }
}

Copy-Item "$PushRoot\disklordz-ralph\windows\VST-PROMPT-SNIPPET.md" "VST-LINK.md" -Force
Write-Host "VST workspace ready at $VstRoot. Business Ralph: $PushRoot\disklordz-ralph\ralph.ps1" -ForegroundColor Green
Write-Host "Next: implement DLZ-VST-001 in this folder; verify via push repo: node disklordz-ralph/scripts/verify-story.mjs DLZ-VST-001" -ForegroundColor Cyan