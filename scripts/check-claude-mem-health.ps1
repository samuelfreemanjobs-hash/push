$ErrorActionPreference = "Stop"
$portFile = Join-Path $env:USERPROFILE ".claude-mem\.worker.port"
if (-not (Test-Path $portFile)) {
    Write-Error "No worker port file. Run: .\scripts\install-claude-mem.ps1"
}
$port = (Get-Content $portFile -Raw).Trim()
$url = "http://127.0.0.1:$port/api/health"
Write-Host "GET $url"
try {
    $resp = Invoke-RestMethod -Uri $url -TimeoutSec 5
    $resp | ConvertTo-Json
} catch {
    Write-Error "Worker not healthy. Try: npx claude-mem doctor"
}