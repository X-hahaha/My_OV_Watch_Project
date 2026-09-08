# Day 2：纯 HAL CubeMX 配置清单

准备日期：2026-09-05，2026-09-06 更新。状态：CubeMX 工程已生成到 `project/my_project`，生成代码可用 CMake/Ninja/GCC 完整编译；当前剩余一项静态配置修正：PA2 `CHARG` 需要从普通输入改为双边沿 EXTI。当前没有实板，因此本页只能证明静态配置与编译结果，不能证明启动、串口、PWM 或外设电气行为。

## 与参考工程的边界

- 新建 STM32F411CEU6 工程，不导入参考 `OV_Watch.ioc`。旧文件用于核对资源，不作为配置模板。
- 不启用 FreeRTOS、CMSIS-RTOS、LVGL、USB 或其他中间件。
- HAL 时基固定为 TIM10。参考工程使用 TIM1，但新工程释放 TIM1；`TIM1_UP_TIM10_IRQn` 只是两者共用的向量名称。
- 选择 CMake 工具链和 GCC。CubeMX 先生成基础构建文件，Day 3 再统一应用区链接脚本、构建预设和调试配置。
- Day 2 生成的默认链接地址通常从 `0x08000000` 开始。在 Day 3 将 Flash 改为 `0x0800C000 / 0x74000` 并验证 VTOR 以前，只编译，不烧录。

## 1. 新建工程与代码生成

在 MCU/MPU Selector 中搜索 `STM32F411CEU6`，确认封装为 UFQFPN48。Project Manager 使用以下设置：

| 项目 | 设置 |
|---|---|
| Project Name | `my_project` |
| Project Location | 当前实际为仓库内的 `project`，最终工程位于 `project/my_project` |
| Application Structure | Basic |
| Toolchain / IDE | CMake |
| Default Compiler / Linker | GCC |
| Firmware Package | 固定选择本机已安装的 `STM32Cube_FW_F4_V1.28.2` |
| Heap / Stack | `0x200` / `0x400`，仅作裸机初值 |
| Keep User Code when re-generating | 开启 |
| Peripheral initialization files | 每个外设生成独立 `.c/.h` |
| Firmware library | Copy only the necessary library files |
| Set all free pins as analog | 完成下方所有引脚分配后开启 |

生成前再次确认最终工程目录位于当前 Git 仓库内，不能在其中再初始化一次 Git。当前采用 `project/my_project` 子目录布局，检查脚本同时支持 CubeMX 的 `Src/Inc` 和 `Core/Src/Core/Inc` 两种生成布局。

## 2. 系统与时钟

| 页面 | 设置 |
|---|---|
| System Core / SYS / Debug | Serial Wire；保留 PA13 SWDIO、PA14 SWCLK |
| System Core / SYS / Timebase Source | TIM10 |
| RCC / Low Speed Clock | Crystal/Ceramic Resonator，PC14/PC15 接 32.768 kHz LSE |
| HSE | 不启用；PH0/PH1 在原理图中未连接 |
| PLL Source | HSI 16 MHz |
| PLL | M=8、N=100、P=2、Q=4 |
| SYSCLK / HCLK | 100 MHz / 100 MHz |
| APB1 | /2，PCLK1=50 MHz，APB1 Timer Clock=100 MHz |
| APB2 | /1，PCLK2=100 MHz，APB2 Timer Clock=100 MHz |

PLLQ 输出为 50 MHz，本项目不启用要求精确 48 MHz 的 USB、SDIO 等外设。TIM10 的 `HAL_InitTick()` 应把计数时钟分到 1 MHz，并使用 ARR=999 得到 1 ms Tick；若 CubeMX 自动计算，以生成代码为准。

## 3. 外设配置

| 外设 | 配置 |
|---|---|
| ADC1 | PA1 / IN1；12 bit、右对齐、单次转换、软件触发、PCLK2/4；Rank 1；初始采样 3 cycles；分压 1 MΩ/1 MΩ、比例 1:2，Vref=3.3 V，换算 `raw * 2 * 3.3 / 4096` |
| SPI1 | Master、Transmit Only、8 bit、MSB first、CPOL High、CPHA 2 Edge、软件 NSS、Prescaler 2；PB3 SCK、PB5 MOSI；P169H002-CTP 沿用旧工程 50 Mbit/s |
| TIM3 | PWM Generation CH3，PB0；PSC=99、Counter Period=300、Pulse=0；旧配置约 3.322 kHz |
| USART1 | Asynchronous，PA9 TX、PA10 RX；115200、8 data bits、no parity、1 stop bit、no flow control、oversampling 16 |
| RTC | Clock Source LSE；Calendar 与 Wakeup 开启；初始可沿用 RTCCLK/16、counter 2000，实际周期由生成代码和实板验证 |

USART1 采用 115200 是参考源码值；旧 `.ioc` 的 9600 只保留为待实测差异。

`Pulse=0` 是 TIM3_CH3 的初始比较值 CCR3，不表示 PWM 永远固定为 0。PWM 初始化后还要调用 `HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3)` 才开始输出；运行时使用 `__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, compare)` 改变占空比。当前计数周期为 `ARR+1=301` 个计数，旧驱动按 `compare = brightness_percent * 300 / 100` 调光，因此初始化为 0 可以确保背光默认关闭，再由业务层设置亮度。

## 4. DMA 与 NVIC

STM32F411 的 SPI1 TX 确实允许 DMA2 Stream2 / Channel2，因此保留旧 Stream 选择，但把旧 `.ioc` 的 HALFWORD 对齐修正为 BYTE。

| 请求 | 控制器 / Stream / Channel | 方向与模式 | 对齐 / 自增 / FIFO | IRQ 优先级 |
|---|---|---|---|---|
| SPI1_TX | DMA2 Stream2 / Channel2 | Memory to Peripheral；Normal；Low | BYTE/BYTE；Memory increment；Peripheral fixed；FIFO disabled | 5 / 0 |
| USART1_RX | DMA2 Stream5 / Channel4 | Peripheral to Memory；Normal；Low | BYTE/BYTE；Memory increment；Peripheral fixed；FIFO disabled | 5 / 0 |
| USART1_TX | DMA2 Stream7 / Channel4 | Memory to Peripheral；Normal；Low | BYTE/BYTE；Memory increment；Peripheral fixed；FIFO disabled | 5 / 0 |

其他中断初值：USART1、RTC Wakeup、EXTI2、EXTI4、EXTI9_5、EXTI15_10 使用 5/0；TIM10 的共享 `TIM1_UP_TIM10_IRQn` 使用 15/0；Priority Group 设为 4。Day 5 引入 FreeRTOS 时，再按 `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` 复核所有会调用 RTOS API 的中断。

## 5. GPIO 名称、模式和启动电平

| 引脚 | User Label | 模式 / 上下拉 | 初始电平或触发 |
|---|---|---|---|
| PA2 | `CHARG` | EXTI Rising/Falling，Pull-down | 无 VBUS 时默认低；充电时 `CHRG#` 下拉，经 LED/2 kΩ 支路使 PA2 所在节点为高。该网络暂按“VBUS/充电电源存在”解释，三状态实测后再确定是否能区分充电中与充满 |
| PA3 | `POWER_EN` | Output Push-Pull，No pull，Low speed | **High**；并把 `MX_GPIO_Init()` 放在外设初始化首位 |
| PA4 | `WAKE` | EXTI Rising，No pull | 外部 R15 下拉，按 SW2 后为高 |
| PA5 | `KEY1` | EXTI Falling，Pull-up | 按下接地 |
| PA8 | `BLE_EN` | Output Push-Pull，No pull，Low speed | Low |
| PA11 / PA12 | `PROM_SDA` / `PROM_SCL` | Output Open-Drain，No pull | High；外部各有 4.7 kΩ 上拉 |
| PA15 | `TP_RST` | Output Push-Pull，Pull-up | High |
| PB1 | `WDOG_EN_N` | Output Push-Pull，Pull-up，Low speed | High，启动时禁用外部看门狗 |
| PB2 | `WDI` | Output Push-Pull，No pull，High speed | Low |
| PB4 / PB6 | `TP_SDA` / `TP_SCL` | Output Open-Drain，No pull | High；外部 R4 阵列 4.7 kΩ 上拉 |
| PB7 / PB8 / PB9 | `LCD_RST` / `LCD_CS` / `LCD_DC` | Output Push-Pull，No pull，Very high speed | High |
| PB12 | `SENSOR_INT` | EXTI Falling，Pull-up | 外部也有 4.7 kΩ 上拉 |
| PB13 / PB14 | `SENSOR_SDA` / `SENSOR_SCL` | Output Open-Drain，No pull | High；外部 R1/R2 各 4.7 kΩ 上拉 |
| PB15 | `LED_EN` | Output Push-Pull，No pull，Low speed | Low，防止传感器 LED 在启动时点亮 |

PA0、PA7、PB10、PC13 在原理图中为 NC，保持 Analog/No pull。PA6 标为 `UID_SEL`，但当前原理图中未找到对应负载、参考源码也未使用，先保持 Analog/No pull 并记录为待确认。触摸接口存在 `TP_INT` 网络，但没有连接到 MCU；不要把 PB1 配成触摸中断。

## 6. 生成前检查

- Pinout 中不存在红色冲突；PA13/PA14 仍显示 SYS Serial Wire。
- Middleware 列表中没有 FreeRTOS、CMSIS-RTOS 或 LVGL。
- SYS Timebase Source 显示 TIM10。
- SPI1 TX DMA 是 Stream2 / Channel2 且两端对齐均为 BYTE。
- USART1 是 115200；TIM3 CH3 是 PB0；ADC1 IN1 是 PA1。
- PA2 是 EXTI2、Rising/Falling、Pull-down；不能只选择普通 GPIO Input。
- PA3 初始 High，PB1、PB15、PA8 的安全初始电平分别为 High、Low、Low。
- Project Manager / Advanced Settings 中 `MX_GPIO_Init()` 排在其他 `MX_*_Init()` 之前。

生成后先不要烧录。保存 `.ioc`，在仓库根目录运行：

```powershell
./tools/check-day02-config.ps1 -RequireGeneratedCode
git status --short
```

脚本会递归发现仓库内唯一的 `.ioc`；存在多个文件时再用 `-IocPath project/my_project/my_project.ioc` 明确指定。CubeMX 可能省略仍为默认值的 `USART1.BaudRate=115200`，检查器会改查生成的 `usart.c`，不会把省略默认字段误判为失败。当前尚未生成 `.ioc` 时，检查脚本会以退出码 2 报告 `NOT READY`，不会修改任何文件或连接硬件。

## 7. 当前验收状态

| 项目 | 状态 |
|---|---|
| CubeMX、CMake、GCC 与所需 VSCode 扩展可用 | PASS |
| `.ioc` 与 HAL 代码已生成 | PASS；`project/my_project/my_project.ioc`，CubeMX 6.15.0 / STM32CubeF4 V1.28.2 |
| 检查器路径与默认值处理 | PASS；已支持递归 `.ioc`、`Src/Inc` 布局，并由生成的 `usart.c` 确认 115200 |
| MCU、时钟、TIM10、SPI、DMA、UART、ADC、RTC 和安全输出 | PASS（静态） |
| PA2 `CHARG` | FAIL；当前是普通 Input + Pull-down，需在 CubeMX 改为 EXTI Rising/Falling + Pull-down 后重新生成 |
| Debug CMake/Ninja/GCC 构建 | PASS；38 个编译/链接步骤，RAM 2280 B、Flash 21436 B；仍是 Day 2 默认 `0x08000000` 链接布局 |
| `SystemCoreClock`、`HAL_GetTick()` 运行验证 | BLOCKED_HW：无硬件 |
| 串口启动信息、PWM 占空比和实板电平 | BLOCKED_HW：无硬件 |

DMA 映射依据 ST 的 [STM32F411 参考手册 RM0383](https://www.st.com/resource/en/reference_manual/dm00119316-stm32f411xce-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) 中 DMA2 request mapping；项目设置与 CMake/GCC 选项依据本机 UM1718 Rev 48，并可在 ST 的 [STM32CubeMX 用户手册 UM1718](https://www.st.com/resource/en/user_manual/dm00104712-stm32cubemx-for-stm32-configuration-and-initialization-c-code-generation-stmicroelectronics.pdf) 复核。
