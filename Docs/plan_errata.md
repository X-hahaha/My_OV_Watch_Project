# Day 1 计划与参考源码校对

检查日期：2026-09-05，2026-09-06 更新。参考根目录：`../../gpt/OV_Watch`（相对于本文件）。已结合 `../Hardware/SCH_Core.pdf` 与 `../Hardware/SCH_Back.pdf` 复核；用户确认实板与该套原理图对应，静态结论仍不能替代电平和时序测试。

| 编号 | 原始内容 / 差异 | 本项目处理 |
|---|---|---|
| E01 | 参考源码 `stm32f4xx_hal_timebase_tim.c` 和旧 `.ioc` 实际使用 TIM1；新工程无需为 1 ms HAL Tick 占用高级定时器 | 用户决定新工程固定使用 TIM10。TIM10 足以提供 1 ms Tick，并与 TIM1 Update 共用 `TIM1_UP_TIM10_IRQn`；共享向量名不表示启用 TIM1。Day 2 验证 `HAL_InitTick()`、`HAL_GetTick()` 和中断处理对象均为 TIM10 |
| E02 | `.ioc:18,21` 的 SPI1 TX DMA 为 HALFWORD；`Core/Src/spi.c:94-95` 为 BYTE，SPI 数据位为 8 | Day 2 生成配置以 BYTE 为起点；Day 4 / Day 7 用传输结果和波形验证长度、颜色字节序 |
| E03 | `.ioc:219` USART1 为 9600；`Core/Src/usart.c:44` 为 115200 | 新工程以源码的 115200、8N1 为初始配置；旧 BIN 串口速率待实测，不能由此认定实机波特率 |
| E04 | `Core/Src/system_stm32f4xx.c:108` 设置 Flash 偏移 `0x8000`；`Core/Src/main.c:75` 又写入 `SCB->VTOR = 0x0000C000U` | Day 3 统一链接地址与完整 VTOR 地址 `0x0800C000`，在应用早期设置并验证。旧代码可能依赖地址映射，不原样复制两处矛盾写法 |
| E05 | `BSP/TOUCH/CST816.h:12-15` 声明触摸 INT 为 PB1；`BSP/OWDG/WDOG.c:5-6` 将 PB1 用作 WDOG_EN | 原理图确认 PB1 是低有效 `#WDOG_EN`，没有把 `TP_INT` 接到 PB1。触摸头文件宏视为过期定义；新工程先轮询，不把 PB1 配为触摸输入 |
| E06 | 初稿只有器件表和参考源码，缺少原理图证据 | 现已按 `OV-Watch_V2.4` 原理图修订引脚和电源网络；用户于 2026-09-06 确认当前实板与该原理图对应 |
| E07 | 旧源码把 PA4 命名为 KEY2，原理图网络名为 `WAKE` | 两者指向同一实体 SW2 的不同用途：SW2 直接经 D4 拉起 `TPS_EN`，同时经 R14 把状态送到 PA4；系统上电后旧固件再把 PA4 当第二功能键。新工程采用 `WAKE` / `POWER_KEY` 主名称 |
| E08 | `Core/Src/gpio.c:53-60` 把 PC13 初始化成低电平输出 | 原理图将 PC13 标为 NC；新工程不复制此遗留 GPIO 配置 |
| E09 | `BSP/POWER/power.c:46-48` 将 PA2 高电平解释为 charging | PA2 没有直接接在 `CHRG#` 引脚上，而是接在 2 kΩ 电阻与指示 LED 之间的节点。`CHRG#` 充电时下拉会形成 LED 电流并使该节点为高，因此旧代码的高有效读取有电路依据；该节点也可能在充满时保持高，更稳妥的临时语义是“VBUS/充电电源存在”。配置为双边沿 EXTI + Pull-down，三状态实测后再确定最终命名 |
| E10 | 硬件表把 `AFC01-S18FCA-00` 与显示项列在一起，容易被理解为 LCD 型号 | 原理图表明它是 X2 的 18 针连接器型号；用户确认 LCD 为 P169H002-CTP，240×280、ST7789，SPI 初始速率沿用旧工程 50 Mbit/s |

这些差异均已记录；参考工程未修改。SPI DMA、UART、VTOR 和充电极性的内容是实现参考差异，不代表已确认旧 BIN 存在对应故障。
