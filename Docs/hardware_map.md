# 硬件映射

更新日期：2026-09-06。状态：参考源码与 `Hardware/SCH_Core.pdf`、`Hardware/SCH_Back.pdf` 静态核对完成；用户确认当前实板与这套 `OV-Watch_V2.4` 原理图对应。因近期无法取得硬件，电平、时序和运行行为仍按“待实测”管理。

依据：仓库根目录 `硬件外设.xlsx` 的 `Sheet1!A1:C10`、两份原理图，以及参考工程 `../../gpt/OV_Watch`。硬件表列出 EM7028、MPU6050、AHT21、LSM303DLHC、SPL06-001、AFC01-S18FCA-00、BL24C02F-RRRC、TPS3823-33DBVR、KT6328A1，但没有引脚号；引脚网络以原理图为主要静态证据，旧源码用于解释历史用法和发现冲突。

## 外部连接

| 功能 | MCU 引脚 / 外设 | 源码配置与待确认点 | 参考文件 |
|---|---|---|---|
| LCD SCLK / MOSI | PB3 / PB5，SPI1 AF5 | P169H002-CTP，240×280，ST7789；8 bit、MSB first、CPOL=1 / CPHA=1、分频 2。用户确认可沿用旧工程已用速率，因此 PC 端基线固定为 50 Mbit/s | `Core/Src/spi.c`、`BSP/LCD/lcd_init.h`、用户硬件确认 |
| LCD RST / CS / DC | PB7 / PB8 / PB9 | 推挽输出，驱动初始化后写高 | `BSP/LCD/lcd_init.c` |
| LCD 背光 | PB0，TIM3_CH3 AF2 | PSC=99、ARR=300；若 TIM3 时钟为 100 MHz，PWM 约 3322.26 Hz；输出极性及亮度曲线待实测 | `Core/Src/tim.c` |
| 触摸 SDA / SCL | PB4 / PB6，软件 I2C | CST816，源码地址常量 0x15；原理图 R4 阵列提供 4.7 kΩ 上拉；先保留轮询 | `Hardware/SCH_Core.pdf`、`BSP/TOUCH/CST816.c`、`.h` |
| 触摸 RST | PA15 | 推挽输出，初始化写高 | `BSP/TOUCH/CST816.c`、`.h` |
| 触摸 INT | `TP_INT` 接口网络，未确认连接 MCU | 原理图未把 `TP_INT` 标到 PB1；旧头文件的 PB1 宏与原理图 `#WDOG_EN` 冲突。新工程先轮询，禁止把 PB1 配成触摸输入 | `Hardware/SCH_Core.pdf`、`BSP/TOUCH/CST816.h` |
| 传感器 SDA / SCL | PB13 / PB14，共享软件 I2C | AHT21、SPL06、LSM303、MPU6050、EM7028 共用；原理图 R1/R2 各提供 4.7 kΩ 上拉；以后按物理总线统一互斥和超时 | `Hardware/SCH_Back.pdf`、对应 `BSP/*/*.c` 的 `iic_bus_t` 初始化 |
| MPU6050 INT | PB12，EXTI15_10 | 下降沿、上拉；NVIC 5/0 | `BSP/MPU6050/mpu6050.c` |
| EEPROM SDA / SCL | PA11 / PA12，独立软件 I2C | BL24C02；原理图各有 4.7 kΩ 上拉；本项目不得同时将这两脚用于 USB | `Hardware/SCH_Core.pdf`、`BSP/BL24C02/BL24C02.c` |
| 蓝牙 TX / RX | PA9 / PA10，USART1 AF7 | MCU TX=PA9、RX=PA10；源码 115200、8N1；`.ioc` 为 9600，待核实实机 | `Core/Src/usart.c` |
| 蓝牙电源开关控制 | PA8，`BLE_EN` | 控制 BL1551B 的 ENB，由其开关 KT6328A1 的 3V3 供电；旧驱动写高开启、写低关闭 | `Hardware/SCH_Back.pdf`、`BSP/KT6328/KT6328.c` |
| 电池采样 | PA1，ADC1_IN1 | 12 bit；用户确认分压比为 1:2、Vref 为 3.3 V，沿用 `raw * 2 * 3.3 / 4096`。该式是复现基线，实际绝对误差以后再由电阻和参考电压误差评估 | `Core/Src/adc.c`、`BSP/POWER/power.c`、用户硬件确认 |
| 充电电源状态 | PA2，板级网络名 `CHARG`，EXTI2 | 建议双边沿 EXTI + 内部下拉。TP4056M `CHRG#` 充电时开漏下拉，经串联 2 kΩ 和指示 LED 支路后，PA2 所接的 LED/电阻节点会被抬高，因此旧源码读取高有效并非简单反相错误。按完整网络推断，该脚更接近检测 VBUS/充电电源存在，不能可靠区分“充电中”和“充满” | `Hardware/SCH_Core.pdf`、`BSP/POWER/power.c`、`.h`、TP4056 数据手册 |
| 供电自锁 | PA3，`POWER_EN` | MCU 写高，经 R17、D5 驱动 `TPS_EN`，用于启动后的电源保持；写低会释放保持条件 | `Hardware/SCH_Core.pdf`、`BSP/POWER/power.c`、`.h` |
| KEY1 | PA5，EXTI9_5 | 下降沿、上拉 | `BSP/KEY/key.c`、`.h` |
| 电源键状态 / WAKE（旧源码名 KEY2） | PA4，`WAKE`，EXTI4 | SW2 按下后，按键节点经 R14 到 PA4，表现为高电平；旧固件也把同一实体键当作第二功能键。新工程使用 `WAKE` 或 `POWER_KEY` 命名，必要时只在输入层保留 KEY2 别名 | `Hardware/SCH_Core.pdf`、`BSP/KEY/key.c`、`.h`、`User/Tasks/Src/user_KeyTask.c` |
| 传感器 LED 使能 | PB15，`LED_EN` | 驱动背板两组 LED 低边 MOSFET，栅极各有 10 kΩ 下拉；高电平开启，启动初始值设低 | `Hardware/SCH_Core.pdf`、`Hardware/SCH_Back.pdf` |
| 外部看门狗使能 | PB1，`#WDOG_EN` | 原理图确认低有效；旧驱动 Enable 写低、Disable 写高，与原理图一致 | `Hardware/SCH_Core.pdf`、`BSP/OWDG/WDOG.c` |
| 外部看门狗喂狗 | PB2 | 推挽输出翻转 | `BSP/OWDG/WDOG.c` |
| PC13 | NC | 原理图有明确未连接标记；旧 `gpio.c` 把它初始化为低输出，视为参考源码遗留配置，新工程不复制 | `Hardware/SCH_Core.pdf`、`Core/Src/gpio.c` |
| RTC LSE | PC14 / PC15 | 原理图连接 32.768 kHz 晶振 | `Hardware/SCH_Core.pdf`、`OV_Watch.ioc`、`Core/Src/rtc.c` |
| SWD | PA13 / PA14 | `.ioc` 的 SWDIO / SWCLK；生成工程时保留 Serial Wire | `OV_Watch.ioc` |

源码 `lcd_init.h` 定义显示为 240×280，采用 ST7789 驱动；用户确认 LCD 型号为 P169H002-CTP。硬件表中的 AFC01-S18FCA-00 对应原理图 X2 的 18 针连接器型号，不应再当作 LCD 模组型号。屏幕方向、可视区域偏移和触摸坐标仍需实际验证。

## 电源键与自锁顺序

SW2 是一只同时参与硬件上电和软件输入的实体键。断电时 MCU 不可能靠 PA4 主动输出使能信号；实际顺序是：按下 SW2 后，BAT 直接送到按键节点，该节点一路经 D4 拉起 `TPS_EN`，使 TPS63020DSJR 输出 3V3，另一路经 R14 送到 PA4 的 `WAKE`。MCU 启动后必须及时把 PA3 `POWER_EN` 拉高，并经 R17、D5 继续维持 `TPS_EN`。释放 SW2 后，系统依靠 PA3 自锁供电；关机路径把 PA3 拉低，才会释放保持。

因此，“PA4 是 WAKE”与“旧源码把 PA4 当 KEY2 使用”可以同时成立：前者是原理图网络名和上电用途，后者是系统上电后对同一实体键的软件用法。需要修正的是“未上电的 MCU 通过 PA4 拉高 TPS_EN”这一因果关系。

## 时钟、DMA 和中断

旧源码 `Core/Src/main.c` 使用 HSI + PLL，M=8、N=100、P=2、Q=4；配置目标 SYSCLK/HCLK=100 MHz、APB1=50 MHz、APB2=100 MHz。上板后观察 `SystemCoreClock` 与 RCC 寄存器。

| 资源 | 配置参考 | IRQ 抢占 / 子优先级 |
|---|---|---|
| HAL Tick | 新工程选 TIM10；APB2 计时器时钟 100 MHz，可用 PSC=99、ARR=999 得到 1 ms；IRQ 名称 `TIM1_UP_TIM10_IRQn` | 初始优先级 15/0；共享向量名不表示启用 TIM1，实际参数以 CubeMX 生成的 `HAL_InitTick()` 为准 |
| RTC Wakeup | LSE；RTC_WKUP_IRQn | 5/0 |
| SPI1 TX DMA | DMA2_Stream2 / Channel2；Normal；BYTE/BYTE | 5/0 |
| USART1 RX DMA | DMA2_Stream5 / Channel4；Normal；BYTE/BYTE | 5/0 |
| USART1 TX DMA | DMA2_Stream7 / Channel4；Normal；BYTE/BYTE | 5/0 |
| USART1 | RX DMA + IDLE 路径 | 5/0 |
| EXTI2 / EXTI4 / EXTI9_5 / EXTI15_10 | 充电状态 / WAKE（旧名 KEY2）/ KEY1 / MPU6050 | 均为 5/0 |

依据：`Core/Src/{dma,spi,usart,rtc,stm32f4xx_hal_timebase_tim}.c` 和对应 BSP 初始化。旧源码与旧 `.ioc` 的 HAL 时基实际是 TIM1；新工程按用户决定改用 TIM10。这里的其他优先级是旧源码事实；Day 5 引入 FreeRTOS 时重新检查每个调用 RTOS API 的中断。

## 仍需实板补证据

- 测量 PA2 在无 USB、充电中和充满三种状态的实际电平，确认它表示 VBUS 存在还是能进一步表示充电阶段；在此之前，业务代码避免把 `ChargeCheck()` 直接命名为“正在充电”。
- 测量按下 SW2 到 3V3 建立、PA3 拉高以及释放 SW2 的时间关系，确定启动代码允许的自锁时限。
- 实测三组软件 I2C 的上拉阻值、上升时间和供电域。原理图均标出 4.7 kΩ 外部上拉；旧 `IICInit()` 初始设为推挽，上层再把 SDA 切成开漏，初始化方式仍需修正。
- 实机蓝牙串口速率；复核 LCD 在 50 Mbit/s 下的长期稳定性，以及电池换算的绝对误差。后两项不再阻塞 PC 端按旧工程参数实现。
