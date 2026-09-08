# Validates the CubeMX .ioc and generated HAL skeleton without touching hardware.
[CmdletBinding()]
param(
    [string]$IocPath = '',
    [switch]$RequireGeneratedCode
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$results = New-Object System.Collections.Generic.List[object]
$failed = $false

function Add-CheckResult {
    param(
        [string]$Name,
        [ValidateSet('PASS', 'FAIL', 'SKIP')]
        [string]$Status,
        [string]$Detail
    )
    $script:results.Add([pscustomobject]@{
        Check  = $Name
        Status = $Status
        Detail = $Detail
    })
    if ($Status -eq 'FAIL') {
        $script:failed = $true
    }
}

function Require-Pattern {
    param(
        [string]$Name,
        [string]$Text,
        [string]$Pattern,
        [string]$Detail
    )
    if ([regex]::IsMatch($Text, $Pattern)) {
        Add-CheckResult $Name 'PASS' $Detail
    }
    else {
        Add-CheckResult $Name 'FAIL' $Detail
    }
}

function Reject-Pattern {
    param(
        [string]$Name,
        [string]$Text,
        [string]$Pattern,
        [string]$Detail
    )
    if ([regex]::IsMatch($Text, $Pattern)) {
        Add-CheckResult $Name 'FAIL' $Detail
    }
    else {
        Add-CheckResult $Name 'PASS' $Detail
    }
}

if (!$IocPath) {
    $iocFiles = @(
        Get-ChildItem -LiteralPath $repoRoot -Recurse -File -Filter '*.ioc' |
            Where-Object { $_.FullName -notmatch '[\\/](?:\.git|build|tmp|\.tools)[\\/]' }
    )
    if ($iocFiles.Count -eq 0) {
        Write-Host 'NOT READY: no .ioc file exists under the repository. Generate the Day 2 CubeMX project first.'
        exit 2
    }
    if ($iocFiles.Count -gt 1) {
        Write-Host 'FAIL: more than one .ioc file exists. Pass the intended file with -IocPath.'
        $iocFiles | ForEach-Object { Write-Host "  $($_.FullName)" }
        exit 1
    }
    $resolvedIoc = $iocFiles[0].FullName
}
else {
    $candidate = $IocPath
    if (![System.IO.Path]::IsPathRooted($candidate)) {
        $candidate = Join-Path $repoRoot $candidate
    }
    if (!(Test-Path -LiteralPath $candidate -PathType Leaf)) {
        Write-Host "FAIL: .ioc file not found: $candidate"
        exit 1
    }
    $resolvedIoc = (Resolve-Path -LiteralPath $candidate).Path
}

$ioc = [System.IO.File]::ReadAllText($resolvedIoc)
$generatedRoot = Split-Path -Parent $resolvedIoc

function Get-GeneratedFilePath {
    param(
        [ValidateSet('Source', 'Header')]
        [string]$Kind,
        [string]$FileName
    )

    $relativeDirectories = if ($Kind -eq 'Source') {
        @('Core\Src', 'Src')
    }
    else {
        @('Core\Inc', 'Inc')
    }

    foreach ($relativeDirectory in $relativeDirectories) {
        $candidate = Join-Path (Join-Path $generatedRoot $relativeDirectory) $FileName
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return $candidate
        }
    }

    return Join-Path (Join-Path $generatedRoot $relativeDirectories[0]) $FileName
}

$timebasePath = Get-GeneratedFilePath 'Source' 'stm32f4xx_hal_timebase_tim.c'
$irqPath = Get-GeneratedFilePath 'Source' 'stm32f4xx_it.c'
$mainPath = Get-GeneratedFilePath 'Source' 'main.c'
$mainHeaderPath = Get-GeneratedFilePath 'Header' 'main.h'
$gpioPath = Get-GeneratedFilePath 'Source' 'gpio.c'
$timPath = Get-GeneratedFilePath 'Source' 'tim.c'
$usartPath = Get-GeneratedFilePath 'Source' 'usart.c'
$spiPath = Get-GeneratedFilePath 'Source' 'spi.c'

Write-Host "Checking CubeMX configuration: $resolvedIoc"

Require-Pattern 'MCU' $ioc '(?m)^Mcu\.CPN=STM32F411CEU6\r?$' 'STM32F411CEU6'
Require-Pattern 'Cube firmware' $ioc '(?m)^ProjectManager\.FirmwarePackage=STM32Cube FW_F4 V1\.28\.2\r?$' 'STM32CubeF4 V1.28.2'
Require-Pattern 'Build generator' $ioc '(?mi)^ProjectManager\.TargetToolchain=CMake\r?$' 'CMake toolchain'
Reject-Pattern 'No RTOS/LVGL' $ioc '(?mi)^(FREERTOS\.|VP_FREERTOS_|rtos\.)|CMSIS[-_]?RTOS|LVGL' 'No FreeRTOS, CMSIS-RTOS, or LVGL entries'

Require-Pattern 'HAL timebase' $ioc '(?m)^NVIC\.TimeBaseIP=TIM10\r?$' 'TIM10 selected'
Reject-Pattern 'TIM1 not selected' $ioc '(?m)^NVIC\.TimeBaseIP=TIM1\r?$|^VP_SYS_VS_tim1\.' 'TIM1 is not the HAL timebase'
Require-Pattern 'System clock' $ioc '(?m)^RCC\.SYSCLKFreq_VALUE=100000000\r?$' 'SYSCLK 100 MHz'
Require-Pattern 'AHB clock' $ioc '(?m)^RCC\.HCLKFreq_Value=100000000\r?$' 'HCLK 100 MHz'
Require-Pattern 'APB1 divider' $ioc '(?m)^RCC\.APB1CLKDivider=RCC_HCLK_DIV2\r?$' 'PCLK1 50 MHz, APB1 timer clock 100 MHz'
Require-Pattern 'PLL M/N' $ioc '(?ms)^RCC\.PLLM=8\r?$.*^RCC\.PLLN=100\r?$' 'HSI / 8 * 100 / 2 = 100 MHz'
Require-Pattern 'RTC clock' $ioc '(?m)^RCC\.RCC_RTC_Clock_Source=RCC_RTCCLKSOURCE_LSE\r?$' 'RTC uses LSE'

Require-Pattern 'SWDIO' $ioc '(?m)^PA13\.Signal=SYS_JTMS-SWDIO\r?$' 'PA13 reserved for SWDIO'
Require-Pattern 'SWCLK' $ioc '(?m)^PA14\.Signal=SYS_JTCK-SWCLK\r?$' 'PA14 reserved for SWCLK'
Require-Pattern 'ADC input' $ioc '(?m)^PA1\.Signal=ADCx_IN1\r?$' 'PA1 is ADC1_IN1'
Require-Pattern 'SPI1 SCK' $ioc '(?m)^PB3\.Signal=SPI1_SCK\r?$' 'PB3 is SPI1_SCK'
Require-Pattern 'SPI1 MOSI' $ioc '(?m)^PB5\.Signal=SPI1_MOSI\r?$' 'PB5 is SPI1_MOSI'
Require-Pattern 'USART1 TX' $ioc '(?m)^PA9\.Signal=USART1_TX\r?$' 'PA9 is USART1_TX'
Require-Pattern 'USART1 RX' $ioc '(?m)^PA10\.Signal=USART1_RX\r?$' 'PA10 is USART1_RX'
Require-Pattern 'Backlight PWM' $ioc '(?m)^PB0\.Signal=S_TIM3_CH3\r?$' 'PB0 is TIM3_CH3'
Require-Pattern 'CHARG EXTI signal' $ioc '(?m)^PA2\.Signal=GPXTI2\r?$' 'PA2 uses EXTI2'
Require-Pattern 'CHARG EXTI edges' $ioc '(?m)^PA2\.GPIO_ModeDefaultEXTI=GPIO_MODE_IT_RISING_FALLING\r?$' 'Both edges detect VBUS/status changes'
Require-Pattern 'CHARG pull-down' $ioc '(?m)^PA2\.GPIO_PuPd=GPIO_PULLDOWN\r?$' 'No-VBUS state defaults low'

Require-Pattern 'SPI polarity' $ioc '(?m)^SPI1\.CLKPolarity=SPI_POLARITY_HIGH\r?$' 'CPOL high'
Require-Pattern 'SPI phase' $ioc '(?m)^SPI1\.CLKPhase=SPI_PHASE_2EDGE\r?$' 'CPHA second edge'
Require-Pattern 'SPI calculated rate' $ioc '(?m)^SPI1\.CalculateBaudRate=50\.0 MBits/s\r?$' 'SPI1 SCK is 50 Mbit/s with the 100 MHz APB2 clock'
Require-Pattern 'TIM3 prescaler' $ioc '(?m)^TIM3\.Prescaler=99\r?$' 'TIM3 PSC 99'
Require-Pattern 'TIM3 period' $ioc '(?m)^TIM3\.Period=300\r?$' 'TIM3 ARR 300'

$uartBaudEntry = [regex]::Match($ioc, '(?m)^USART1\.BaudRate=(\d+)\r?$')
if ($uartBaudEntry.Success) {
    if ($uartBaudEntry.Groups[1].Value -eq '115200') {
        Add-CheckResult 'UART baud' 'PASS' 'USART1 115200 baud is explicit in .ioc'
    }
    else {
        Add-CheckResult 'UART baud' 'FAIL' "USART1 baud in .ioc is $($uartBaudEntry.Groups[1].Value), expected 115200"
    }
}
elseif (Test-Path -LiteralPath $usartPath) {
    $usart = [System.IO.File]::ReadAllText($usartPath)
    Require-Pattern 'UART baud' $usart '(?m)^\s*huart1\.Init\.BaudRate\s*=\s*115200\s*;\r?$' 'CubeMX omitted the default from .ioc; generated usart.c confirms 115200 baud'
}
elseif ($RequireGeneratedCode) {
    Add-CheckResult 'UART baud' 'FAIL' 'USART1.BaudRate is omitted from .ioc and generated usart.c is unavailable'
}
else {
    Add-CheckResult 'UART baud' 'SKIP' 'USART1.BaudRate is omitted as a CubeMX default; generate code to confirm 115200'
}

Require-Pattern 'SPI1 TX stream' $ioc '(?m)^Dma\.SPI1_TX\.\d+\.Instance=DMA2_Stream2\r?$' 'DMA2 Stream2 / Channel2 request mapping'
Require-Pattern 'SPI1 TX memory alignment' $ioc '(?m)^Dma\.SPI1_TX\.\d+\.MemDataAlignment=DMA_MDATAALIGN_BYTE\r?$' 'Memory alignment is BYTE'
Require-Pattern 'SPI1 TX peripheral alignment' $ioc '(?m)^Dma\.SPI1_TX\.\d+\.PeriphDataAlignment=DMA_PDATAALIGN_BYTE\r?$' 'Peripheral alignment is BYTE'
Require-Pattern 'USART1 RX stream' $ioc '(?m)^Dma\.USART1_RX\.\d+\.Instance=DMA2_Stream5\r?$' 'DMA2 Stream5 / Channel4 request mapping'
Require-Pattern 'USART1 TX stream' $ioc '(?m)^Dma\.USART1_TX\.\d+\.Instance=DMA2_Stream7\r?$' 'DMA2 Stream7 / Channel4 request mapping'

foreach ($label in @('POWER_EN', 'WAKE', 'KEY1', 'BLE_EN', 'WDOG_EN_N', 'WDI', 'LED_EN', 'CHARG')) {
    Require-Pattern "GPIO label $label" $ioc "(?m)^P[A-C][0-9]+\.GPIO_Label=$([regex]::Escape($label))\r?$" 'Required board-level label exists'
}

if (Test-Path -LiteralPath $timebasePath) {
    $timebase = [System.IO.File]::ReadAllText($timebasePath)
    Require-Pattern 'Generated TIM10 handle' $timebase '(?m)htim10\.Instance\s*=\s*TIM10\s*;' 'HAL timebase handle is TIM10'
    Require-Pattern 'Generated TIM10 clock' $timebase '__HAL_RCC_TIM10_CLK_ENABLE\s*\(\s*\)' 'TIM10 peripheral clock is enabled'
    Reject-Pattern 'Generated TIM1 handle absent' $timebase '(?m)htim1\.Instance\s*=\s*TIM1\s*;' 'No TIM1 timebase handle'
}
elseif ($RequireGeneratedCode) {
    Add-CheckResult 'Generated timebase file' 'FAIL' "Missing $timebasePath"
}
else {
    Add-CheckResult 'Generated timebase file' 'SKIP' 'Generate CubeMX code to check htim10'
}

if (Test-Path -LiteralPath $irqPath) {
    $irq = [System.IO.File]::ReadAllText($irqPath)
    Require-Pattern 'Shared IRQ handler target' $irq 'HAL_TIM_IRQHandler\s*\(\s*&htim10\s*\)' 'TIM1_UP_TIM10_IRQHandler dispatches htim10'
    Require-Pattern 'Generated EXTI2 handler' $irq '(?ms)void\s+EXTI2_IRQHandler\s*\([^)]*\).*?HAL_GPIO_EXTI_IRQHandler\s*\(\s*CHARG_Pin\s*\)' 'EXTI2 dispatches the CHARG pin'
}
elseif ($RequireGeneratedCode) {
    Add-CheckResult 'Generated IRQ file' 'FAIL' "Missing $irqPath"
}
else {
    Add-CheckResult 'Generated IRQ file' 'SKIP' 'Generate CubeMX code to check IRQ dispatch'
}

if (Test-Path -LiteralPath $mainHeaderPath) {
    $mainHeader = [System.IO.File]::ReadAllText($mainHeaderPath)
    $pinDefinitions = @(
        @('CHARG', '2', 'GPIOA'),
        @('POWER_EN', '3', 'GPIOA'),
        @('WAKE', '4', 'GPIOA'),
        @('KEY1', '5', 'GPIOA'),
        @('BLE_EN', '8', 'GPIOA'),
        @('WDOG_EN_N', '1', 'GPIOB'),
        @('WDI', '2', 'GPIOB'),
        @('LED_EN', '15', 'GPIOB')
    )
    foreach ($pinDefinition in $pinDefinitions) {
        $label = $pinDefinition[0]
        $pin = $pinDefinition[1]
        $port = $pinDefinition[2]
        $pattern = "(?ms)^#define\s+$label`_Pin\s+GPIO_PIN_$pin\s*`r?`n#define\s+$label`_GPIO_Port\s+$port\s*$"
        Require-Pattern "Generated pin $label" $mainHeader $pattern "$label maps to $port pin $pin"
    }
}
elseif ($RequireGeneratedCode) {
    Add-CheckResult 'Generated main.h' 'FAIL' "Missing $mainHeaderPath"
}
else {
    Add-CheckResult 'Generated main.h' 'SKIP' 'Generate CubeMX code to check pin macros'
}

if (Test-Path -LiteralPath $gpioPath) {
    $gpio = [System.IO.File]::ReadAllText($gpioPath)
    Require-Pattern 'POWER_EN startup state' $gpio '(?m)^\s*HAL_GPIO_WritePin\([^;]*POWER_EN_Pin[^;]*GPIO_PIN_SET\s*\);' 'POWER_EN starts high'
    Require-Pattern 'WDOG disabled at startup' $gpio '(?m)^\s*HAL_GPIO_WritePin\([^;]*WDOG_EN_N_Pin[^;]*GPIO_PIN_SET\s*\);' 'WDOG_EN_N starts high'
    Require-Pattern 'BLE disabled at startup' $gpio '(?m)^\s*HAL_GPIO_WritePin\([^;]*BLE_EN_Pin[^;]*GPIO_PIN_RESET\s*\);' 'BLE_EN starts low'
    Require-Pattern 'Sensor LED disabled at startup' $gpio '(?m)^\s*HAL_GPIO_WritePin\([^;]*LED_EN_Pin[^;]*GPIO_PIN_RESET\s*\);' 'LED_EN starts low'
    Require-Pattern 'Generated CHARG EXTI mode' $gpio '(?ms)GPIO_InitStruct\.Pin\s*=\s*CHARG_Pin\s*;.*?GPIO_InitStruct\.Mode\s*=\s*GPIO_MODE_IT_RISING_FALLING\s*;.*?GPIO_InitStruct\.Pull\s*=\s*GPIO_PULLDOWN\s*;.*?HAL_GPIO_Init\s*\(\s*CHARG_GPIO_Port\s*,\s*&GPIO_InitStruct\s*\)' 'CHARG is both-edge EXTI with pull-down'
}
elseif ($RequireGeneratedCode) {
    Add-CheckResult 'Generated gpio.c' 'FAIL' "Missing $gpioPath"
}
else {
    Add-CheckResult 'Generated gpio.c' 'SKIP' 'Generate CubeMX code to check safe startup levels'
}

if (Test-Path -LiteralPath $timPath) {
    $tim = [System.IO.File]::ReadAllText($timPath)
    Require-Pattern 'Generated TIM3 PWM channel' $tim 'HAL_TIM_PWM_ConfigChannel\s*\(\s*&htim3\s*,\s*&sConfigOC\s*,\s*TIM_CHANNEL_3\s*\)' 'TIM3 CH3 is configured for PWM'
    Require-Pattern 'Generated TIM3 initial pulse' $tim '(?m)^\s*sConfigOC\.Pulse\s*=\s*0\s*;\r?$' 'Initial compare value is zero'
}
elseif ($RequireGeneratedCode) {
    Add-CheckResult 'Generated tim.c' 'FAIL' "Missing $timPath"
}
else {
    Add-CheckResult 'Generated tim.c' 'SKIP' 'Generate CubeMX code to check TIM3 PWM'
}

if (Test-Path -LiteralPath $spiPath) {
    $spi = [System.IO.File]::ReadAllText($spiPath)
    Require-Pattern 'Generated SPI1 prescaler' $spi '(?m)^\s*hspi1\.Init\.BaudRatePrescaler\s*=\s*SPI_BAUDRATEPRESCALER_2\s*;\r?$' 'Generated SPI1 uses APB2 / 2 = 50 Mbit/s'
    Require-Pattern 'Generated SPI1 data size' $spi '(?m)^\s*hspi1\.Init\.DataSize\s*=\s*SPI_DATASIZE_8BIT\s*;\r?$' 'Generated SPI1 uses 8-bit frames'
}
elseif ($RequireGeneratedCode) {
    Add-CheckResult 'Generated spi.c' 'FAIL' "Missing $spiPath"
}
else {
    Add-CheckResult 'Generated spi.c' 'SKIP' 'Generate CubeMX code to check the LCD SPI rate'
}

if (Test-Path -LiteralPath $mainPath) {
    $main = [System.IO.File]::ReadAllText($mainPath)
    $gpioIndex = $main.IndexOf('MX_GPIO_Init();', [System.StringComparison]::Ordinal)
    $otherIndexes = @()
    foreach ($call in @('MX_DMA_Init();', 'MX_ADC1_Init();', 'MX_RTC_Init();', 'MX_SPI1_Init();', 'MX_TIM3_Init();', 'MX_USART1_UART_Init();')) {
        $index = $main.IndexOf($call, [System.StringComparison]::Ordinal)
        if ($index -ge 0) {
            $otherIndexes += $index
        }
    }
    if ($gpioIndex -ge 0 -and $otherIndexes.Count -gt 0 -and $gpioIndex -lt ($otherIndexes | Measure-Object -Minimum).Minimum) {
        Add-CheckResult 'GPIO initialization order' 'PASS' 'MX_GPIO_Init runs before other peripheral init functions'
    }
    else {
        Add-CheckResult 'GPIO initialization order' 'FAIL' 'MX_GPIO_Init must run first to latch POWER_EN promptly'
    }
}
elseif ($RequireGeneratedCode) {
    Add-CheckResult 'Generated main.c' 'FAIL' "Missing $mainPath"
}
else {
    Add-CheckResult 'Generated main.c' 'SKIP' 'Generate CubeMX code to check init order'
}

$results | Format-Table -AutoSize -Wrap
if ($failed) {
    Write-Host 'Day 2 CubeMX static checks FAIL.'
    exit 1
}

Write-Host 'Day 2 CubeMX static checks PASS. Runtime and hardware checks remain separate.'
exit 0
