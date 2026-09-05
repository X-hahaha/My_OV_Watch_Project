# Day 1 计划与参考源码校对

检查日期：2026-09-05。参考根目录：`../../gpt/OV_Watch`（相对于本文件）。已结合 `../Hardware/SCH_Core.pdf` 与 `../Hardware/SCH_Back.pdf` 复核；静态结论仍不能替代旧 BIN 和实板电平、时序测试。

| 编号 | 原始内容 / 差异 | 本项目处理 |
|---|---|---|
| E01 | 原计划第 4 节为 TIM10，Day 2 / Day 5 为 TIM1 | 本地计划副本第 4 节修正为 TIM1。`Core/Src/stm32f4xx_hal_timebase_tim.c:63` 使用 `htim1.Instance = TIM1`；`.ioc:107` 为 `NVIC.TimeBaseIP=TIM1`。共享 IRQ 名称 `TIM1_UP_TIM10_IRQn` 不表示启用了 TIM10 |
| E02 | `.ioc:18,21` 的 SPI1 TX DMA 为 HALFWORD；`Core/Src/spi.c:94-95` 为 BYTE，SPI 数据位为 8 | Day 2 生成配置以 BYTE 为起点；Day 4 / Day 7 用传输结果和波形验证长度、颜色字节序 |
| E03 | `.ioc:219` USART1 为 9600；`Core/Src/usart.c:44` 为 115200 | 新工程以源码的 115200、8N1 为初始配置；旧 BIN 串口速率待实测，不能由此认定实机波特率 |
| E04 | `Core/Src/system_stm32f4xx.c:108` 设置 Flash 偏移 `0x8000`；`Core/Src/main.c:75` 又写入 `SCB->VTOR = 0x0000C000U` | Day 3 统一链接地址与完整 VTOR 地址 `0x0800C000`，在应用早期设置并验证。旧代码可能依赖地址映射，不原样复制两处矛盾写法 |
| E05 | `BSP/TOUCH/CST816.h:12-15` 声明触摸 INT 为 PB1；`BSP/OWDG/WDOG.c:5-6` 将 PB1 用作 WDOG_EN | 原理图确认 PB1 是低有效 `#WDOG_EN`，没有把 `TP_INT` 接到 PB1。触摸头文件宏视为过期定义；新工程先轮询，不把 PB1 配为触摸输入 |
| E06 | 初稿只有器件表和参考源码，缺少原理图证据 | 现已按 `OV-Watch_V2.4` 原理图修订引脚和电源网络；仍需确认实板与该原理图版本一致 |
| E07 | 旧源码把 PA4 命名为 KEY2，原理图网络名为 `WAKE` | 两者指向同一实体 SW2 的不同用途：SW2 直接经 D4 拉起 `TPS_EN`，同时经 R14 把状态送到 PA4；系统上电后旧固件再把 PA4 当第二功能键。新工程采用 `WAKE` / `POWER_KEY` 主名称 |
| E08 | `Core/Src/gpio.c:53-60` 把 PC13 初始化成低电平输出 | 原理图将 PC13 标为 NC；新工程不复制此遗留 GPIO 配置 |
| E09 | `BSP/POWER/power.c:46-48` 将 PA2 高电平解释为 charging | 原理图中状态来源是 TP4056M 的低有效 `CHRG#`，旧逻辑疑似反相；在无 USB、充电中、充满三种状态实测前，不固化软件极性 |

这些差异均已记录；参考工程未修改。SPI DMA、UART、VTOR 和充电极性的内容是实现参考差异，不代表已确认旧 BIN 存在对应故障。
