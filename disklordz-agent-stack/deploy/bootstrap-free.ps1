# Free hermes-out bootstrap (Windows)
$ErrorActionPreference = "Stop"
$Repo = if ($env:DLZ_REPO_ROOT) { $env:DLZ_REPO_ROOT } else { throw "Set DLZ_REPO_ROOT" }
$State = if ($env:OPENCLAW_STATE_DIR) { $env:OPENCLAW_STATE_DIR } else { Join-Path $env:USERPROFILE ".openclaw" }
$Out = Join-Path $State "hermes-out"

@("research", "pr-review", "relay", "events") | ForEach-Object {
    New-Item -ItemType Directory -Force -Path (Join-Path $Out $_) | Out-Null
}

Push-Location (Join-Path $Repo "disklordz-agent-stack")
npm install --silent 2>$null
if ($LASTEXITCODE -ne 0) { npm install }
$env:DLZ_REPO_ROOT = $Repo
$env:OPENCLAW_STATE_DIR = $State
npm run all
Pop-Location

Write-Host "OK: hermes-out at $Out"