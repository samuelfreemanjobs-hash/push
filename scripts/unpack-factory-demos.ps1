# Unpack operator WAV demos from the three DiskLordz factory zips into Assets/Imports (not for git).
param(
    [Parameter(Mandatory = $true)]
    [string] $ZipDir
)

$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
$Imports = Join-Path $Root "Assets\Imports"
New-Item -ItemType Directory -Force -Path $Imports | Out-Null

$map = @{
    "DiskLordz_Cinematic_DSP_CPP_Source_and_35_WAVs*.zip" = @{
        Dest = "cinematic-dsp-demo"
        Prefix = "DiskLordz_Cinematic_DSP_CPP/DemoWAV"
    }
    "DiskLordz_Drum_Design_Architecture*.zip" = @{
        Dest = "drum-architecture-examples"
        Prefix = "DiskLordz_Drum_Architecture/Examples"
    }
    "DiskLordz_Drum_Forge_4*.zip" = @{
        Dest = "drum-forge-v4-demo"
        Prefix = "DiskLordz_Forge_V4/Demo_Kit"
    }
}

foreach ($pattern in $map.Keys) {
    $zip = Get-ChildItem -Path $ZipDir -Filter $pattern | Select-Object -First 1
    if (-not $zip) {
        Write-Warning "Missing zip matching $pattern in $ZipDir"
        continue
    }
    $destPath = Join-Path $Imports $map[$pattern].Dest
    New-Item -ItemType Directory -Force -Path $destPath | Out-Null
    Expand-Archive -Path $zip.FullName -DestinationPath $destPath -Force
    Write-Host "Extracted $($zip.Name) -> $destPath"
}

Write-Host "Done. Add Assets/Imports/*.wav to .gitignore if large files appear."