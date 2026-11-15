#Requires -Version 5.1
# Install Mem0 Claude Code plugin (operator). API key via env — do not commit.
# https://docs.mem0.ai/integrations/claude-code

$ErrorActionPreference = "Stop"

if (-not $env:MEM0_API_KEY) {
    Write-Error "Set MEM0_API_KEY first (m0-... from app.mem0.ai)."
}

if (-not (Get-Command claude -ErrorAction SilentlyContinue)) {
    Write-Error "claude CLI not found"
}

claude plugin marketplace add mem0ai/mem0
claude plugin install mem0@mem0-plugins --scope user --config "api_key=$env:MEM0_API_KEY"

Write-Host "Installed mem0@mem0-plugins. Restart Claude Code, then /mem0:status"
Write-Host "Clear MEM0_API_KEY from env when done if desired."