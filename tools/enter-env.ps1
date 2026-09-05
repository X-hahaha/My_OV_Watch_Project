# Run in the terminal used to build: . ./tools/enter-env.ps1
[CmdletBinding()]
param([string]$ArmGnuBin = '')

& {
    param($ScriptRoot, $ArmBin)
    $repoRoot = Split-Path -Parent $ScriptRoot
    $toolsRoot = Join-Path $repoRoot '.tools'
    $toolchainLock = Get-Content -LiteralPath (Join-Path $ScriptRoot 'toolchain-lock.json') -Raw | ConvertFrom-Json
    $toolBins = @()
    if ($ArmBin) {
        if (!(Test-Path -LiteralPath (Join-Path $ArmBin 'arm-none-eabi-gcc.exe'))) {
            throw "arm-none-eabi-gcc.exe was not found in $ArmBin"
        }
        $toolBins += (Resolve-Path -LiteralPath $ArmBin).Path
    }
    foreach ($package in $toolchainLock.packages) {
        $executablePath = Join-Path $toolsRoot $package.executable
        if (!(Test-Path -LiteralPath $executablePath)) {
            throw "Missing $($package.name). Run ./tools/install-toolchain.ps1 first."
        }
        $toolBins += Split-Path -Parent $executablePath
    }
    $env:PATH = (@($toolBins) + @($env:PATH -split ';') | Select-Object -Unique) -join ';'
    Write-Host 'Project toolchain is available in this terminal. System and user PATH were not changed.'
} $PSScriptRoot $ArmGnuBin
