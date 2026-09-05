# Download pinned Windows tools into this repository only.
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'
$repoRoot = Split-Path -Parent $PSScriptRoot
$toolsRoot = Join-Path $repoRoot '.tools'
$downloadRoot = Join-Path $toolsRoot 'downloads'
$toolchainLock = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'toolchain-lock.json') -Raw | ConvertFrom-Json
New-Item -ItemType Directory -Path $downloadRoot -Force | Out-Null

foreach ($package in $toolchainLock.packages) {
    $archivePath = Join-Path $downloadRoot $package.archive
    if (!(Test-Path -LiteralPath $archivePath)) {
        Write-Host "Downloading $($package.name) $($package.version)..."
        Invoke-WebRequest -Uri $package.url -OutFile $archivePath -UseBasicParsing -TimeoutSec 180
    }
    $actualHash = (Get-FileHash -LiteralPath $archivePath -Algorithm SHA256).Hash
    if ($actualHash -ne $package.sha256) {
        throw "SHA-256 mismatch for $archivePath. No extraction was performed for this package."
    }
    $executablePath = Join-Path $toolsRoot $package.executable
    if (!(Test-Path -LiteralPath $executablePath)) {
        $extractRoot = Join-Path $toolsRoot $package.extractTo
        Expand-Archive -LiteralPath $archivePath -DestinationPath $extractRoot -Force
    }
    if (!(Test-Path -LiteralPath $executablePath)) {
        throw "Expected executable is missing after extraction: $executablePath"
    }
    Write-Host "Installed: $($package.name) $($package.version); archive SHA-256 verified."
}

Write-Host 'Next: . ./tools/enter-env.ps1'
Write-Host 'Then: ./tools/check-toolchain.ps1'
