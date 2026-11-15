# Sync Stripe/Gumroad into sales_ledger.csv for OpenBB (operator keys in env).
param(
    [switch]$Stripe,
    [switch]$Gumroad,
    [string]$ImportCsv = "",
    [string]$Source = "manual"
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent $PSScriptRoot
$sync = Join-Path $repoRoot "disklordz-ralph\business\openbb\sync_sales_ledger.py"

$py = Get-Command python -ErrorAction SilentlyContinue
if (-not $py) { $py = Get-Command python3 -ErrorAction SilentlyContinue }
if (-not $py) { throw "python not found" }

$args = @($sync)
if ($Stripe) { $args += "--sync-stripe" }
if ($Gumroad) { $args += "--sync-gumroad" }
if ($ImportCsv) { $args += @("--import-csv", $ImportCsv, "--source", $Source) }

& $py.Source @args