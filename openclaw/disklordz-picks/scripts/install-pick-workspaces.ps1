# Copy DiskLordz pod workspaces + shared skills into OPENCLAW_STATE_DIR
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$SkillsRoot = Join-Path (Split-Path -Parent $Root) "skills"
$State = if ($env:OPENCLAW_STATE_DIR) { $env:OPENCLAW_STATE_DIR } else { Join-Path $env:USERPROFILE ".openclaw" }

$dirs = @(
    $State,
    (Join-Path $State "hermes-out\research"),
    (Join-Path $State "hermes-out\pr-review"),
    (Join-Path $State "hermes-out"),
    (Join-Path $State "workspace-shared\vault-docs"),
    (Join-Path $State "skills")
)
foreach ($d in $dirs) { New-Item -ItemType Directory -Force -Path $d | Out-Null }

foreach ($id in @("sage", "dev", "mkt", "biz")) {
    $dest = Join-Path $State "workspace-$id"
    New-Item -ItemType Directory -Force -Path $dest | Out-Null
    foreach ($file in @("AGENTS.md", "OVERNIGHT.md", "SOUL.md")) {
        $src = Join-Path $Root "workspaces\$id\$file"
        if (Test-Path $src) {
            Copy-Item -Force $src (Join-Path $dest $file)
        }
    }
}

$shared = Join-Path $SkillsRoot "shared"
if (Test-Path $shared) {
    Get-ChildItem -Path $shared -Directory | ForEach-Object {
        $name = $_.Name
        $target = Join-Path $State "skills\$name"
        if (Test-Path $target) { Remove-Item -Recurse -Force $target }
        Copy-Item -Recurse -Force $_.FullName $target
        foreach ($pod in @("sage", "dev", "mkt", "biz", "review")) {
            $podSkills = Join-Path $State "workspace-$pod\skills"
            New-Item -ItemType Directory -Force -Path $podSkills | Out-Null
            $podDest = Join-Path $podSkills $name
            if (-not (Test-Path $podDest)) {
                Copy-Item -Recurse -Force $_.FullName $podDest
            }
        }
    }
}

Write-Host "Workspaces: $State\workspace-{sage,dev,mkt,biz}"
Write-Host "Skills: $State\skills"
Write-Host "Hermes-out: $State\hermes-out"