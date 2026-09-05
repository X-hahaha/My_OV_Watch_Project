# Checks executables only. Does not connect to, reset, or flash a target.
[CmdletBinding()]
param()

$requiredTools = @('arm-none-eabi-gcc', 'arm-none-eabi-gdb', 'arm-none-eabi-objdump', 'cmake', 'ninja', 'git')
$debuggers = @('openocd', 'ST-LINK_gdbserver')
$rows = @()
foreach ($toolName in ($requiredTools + $debuggers)) {
    $command = Get-Command $toolName -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
    if (!$command) {
        $rows += [pscustomobject]@{ Tool = $toolName; Status = 'MISSING'; Version = ''; Path = '' }
        continue
    }
    try {
        # ST-LINK_gdbserver uses -v; other tools use --version.
        $versionArg = if ($toolName -eq 'ST-LINK_gdbserver') { '-v' } else { '--version' }
        $versionOutput = @(& $command.Source $versionArg 2>&1)
        $resultCode = $LASTEXITCODE
        $status = if ($resultCode -eq 0) { 'PASS' } else { "ERROR($resultCode)" }
        $versionLine = ($versionOutput | ForEach-Object { "$_" } | Where-Object { $_.Trim() } | Select-Object -First 1)
        $rows += [pscustomobject]@{ Tool = $toolName; Status = $status; Version = $versionLine; Path = $command.Source }
    }
    catch {
        $rows += [pscustomobject]@{ Tool = $toolName; Status = 'ERROR'; Version = $_.Exception.Message; Path = $command.Source }
    }
}
$rows | Format-List
$missingRequired = @($rows | Where-Object { $_.Tool -in $requiredTools -and $_.Status -ne 'PASS' })
$debuggerAvailable = @($rows | Where-Object { $_.Tool -in $debuggers -and $_.Status -eq 'PASS' }).Count -gt 0
$gcc = $rows | Where-Object Tool -eq 'arm-none-eabi-gcc'
$gccVersionMatches = $gcc.Status -eq 'PASS' -and $gcc.Version -match '15\.2\.Rel1'
if ($missingRequired.Count -gt 0 -or !$debuggerAvailable -or !$gccVersionMatches) {
    Write-Host 'Toolchain check incomplete: all required tools, one debugger, and GCC 15.2.Rel1 are needed.'
    exit 1
}
Write-Host 'Toolchain executables PASS. MCU build and physical debugger connection remain separate checks.'
exit 0
