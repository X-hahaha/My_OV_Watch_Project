# 测试矩阵

更新日期：2026-09-14。硬件已到位；下列旧清单的未执行项改为 `PENDING`，仅 ST-Link 供电无法覆盖的项目为 `DEFERRED_POWER`。`PARTIAL` 表示开发者已有基本观察但未完成该行的全部步骤；建立测试行或交叉编译通过不表示硬件功能通过。

| 编号 | 类别 | 操作 / 观察目标 | 实际结果 | 证据 | 状态 |
|---|---|---|---|---|---|
| BLD-001 | 工具 | 在载入项目环境后执行 check-toolchain.ps1，命令可运行且 GCC 版本匹配 | 助手 PowerShell 会话与用户终端均验证通过；OpenOCD 可用 | 见 toolchain.md；用户原始日志位于本机 evidence/day01 | PASS |
| BLD-002 | 工具 | CMake + Ninja 调用 Cortex-M4F GCC 编译最小目标文件 | 编译成功，ELF attributes 正确；不是 MCU 固件构建 | 见 toolchain.md | PASS |
| BLD-003 | 工具 | Day 2 配置检查器处理真实 CubeMX 默认字段与目录布局 | 支持递归 `.ioc`、`Src/Inc` 和 `Core/Src/Core/Inc`；默认 115200 由 `usart.c` 复核；不再产生原日志中的六个假失败 | `tools/check-day02-config.ps1`、`day02_cubemx_checklist.md`、`evidence/day02/terminal-ioc-check-002.txt` | PASS |
| BLD-004 | 构建 | 真实 CubeMX HAL 骨架执行 Debug CMake/Ninja/GCC 构建 | 38 步编译和链接成功；RAM 2280 B，Flash 21436 B | `project/my_project/build/Debug/my_project.elf`、`evidence/day02/terminal-cmake-debug-build-001.txt` | PASS |
| BLD-005 | 构建 | 从空缓存执行 Debug/Release CMake、Ninja 和 GNU Arm 构建 | 两种配置均完成 38 步；RAM 均为 2280 B，Flash 分别为 13748 B 和 11956 B；ELF/HEX/BIN/MAP 齐全 | `evidence/day03/*-configure-audit.log`、`evidence/day03/*-build-audit.log` | PASS |
| CFG-001 | CubeMX 配置 | 检查真实 `.ioc` 与生成代码中的 STM32F411CEU6、TIM10、时钟、DMA、无 RTOS 和安全启动电平 | 全部通过；PA2 为双边沿 EXTI2 + Pull-down，生成的中断处理函数分发 `CHARG_Pin` | `day02_cubemx_checklist.md`、`tools/check-day02-config.ps1` | PASS |
| CFG-002 | 启动静态配置 | 检查应用链接地址、向量表、MSP、Reset Handler、VTOR 和 Cortex-M4F ABI | `.isr_vector=0x0800C000`、MSP=`0x20020000`；Reset 向量与符号匹配；`SystemInit()` 使用 `0x0800C000` | `evidence/day03/elf-static-audit.log` | PASS |
| REL-001 | 固件标识 | 读取候选 BIN/HEX 大小与 SHA-256 | 已记录两个文件 | reference_firmware.md | PASS |
| BOOT-001 | 启动 | 启动 3 次，记录耗时、默认页、重启异常 | 开发者报告原 Keil 固件经 Bootloader 跳转，启动与界面正常；未提供逐次耗时 | 开发者会话反馈，未要求补拍接线照片 | PARTIAL |
| BOOT-002 | 启动 | 由现有 Bootloader 连续跳转新应用 10 次，并核对 MSP、PC 和 VTOR | 已实测跳转到 main；连续 10 次及每次寄存器未记录 | 开发者反馈、Day 3 静态审计、Day 5 任务日志 | PARTIAL |
| DBG-001 | 调试 | 使用 VS Code、Cortex-Debug、OpenOCD 和 ST-Link 验证断点、单步、变量、寄存器及调用栈 | 已读取 Day 5 变量/寄存器；曾有 F5 超时、调用栈/断点需 Reset Device 的现象，稳定性未全部关闭 | `.vscode/launch.json`、`evidence/day04/day3调试尾项/readme.txt`、`evidence/day05/GDB_log.txt` | PARTIAL |
| LCD-001 | 显示 | 页面颜色、方向、亮度和可见缺陷 | 开发者报告原固件 LCD 正常；P169H002-CTP 与 SPI 50 Mbit/s 为参数，未验证新 LCD 驱动 | 开发者会话反馈 | PARTIAL |
| TCH-001 | 输入 | 中心/边缘点按、四向滑动及返回 | 未执行 | 待提供 | PENDING |
| TCH-002 | 输入 | 两个按键短按/长按及唤醒 | 未执行 | 待提供 | PENDING |
| SNS-001 | 传感器 | 温湿度、气压、指南针的数值/单位/响应趋势 | 未执行 | 待提供 | PENDING |
| SNS-002 | 传感器 | 步数变化和心率页的等待/更新行为 | 未执行 | 待提供 | PENDING |
| COM-001 | 蓝牙 | 开关、发现、连接及已有数据交互 | 未执行 | 待提供 | PENDING |
| COM-002 | 串口 | 确认实际速率并保存原始数据；无输出如实记载 | 未执行 | 待提供 | PENDING |
| MEM-001 | 存储 | 设置修改、重进页面、重启保留，最后恢复设置 | 未执行 | 待提供 | PENDING |
| PWR-001 | 功耗行为 | 静置变暗/息屏时间及按键唤醒 | 未执行；没有电流测量 | 待提供 | DEFERRED_POWER |
| PWR-002 | 功耗行为 | 抬腕开关及运动唤醒响应 | 未执行 | 待提供 | DEFERRED_POWER |
| PWR-003 | 功耗行为 | 插拔充电线，记录图标、页面和息屏差异 | 未执行；PA2 暂按 VBUS/充电电源存在解释 | 待提供 | DEFERRED_POWER |
| UI-001 | 页面 | 遍历可见 UI，记录实际入口和退出方式 | 未执行 | 待提供 | PENDING |
| UI-002 | SPO2 展示 | 佩戴/移开时 0/15/30/60 秒显示值 | 未执行 | 待提供 | PENDING |
| UI-003 | NFC 展示 | 卡片内容和 UI 开关行为 | 未执行 | 待提供 | PENDING |
| GAME-001 | 游戏 | 2048、记忆游戏、计算器操作与重进 | 未执行 | 待提供 | PENDING |

实测步骤与逐项模板见 `reference_behavior.md`。每个 PASS/FAIL 需要固件身份、实际结果和证据；后续移植测试补充固件 commit 与 Debug/Release 配置。

开发者另报告原 Keil 固件传感器、通信和存储功能正常；此反馈未包含各行的逐项数值/操作，因此保留详细基线清单待补齐。**G0 尚未完整关闭**，G1 的连续启动和调试稳定性也未完整关闭。

## Day 4 基本验证与 Day 5 RTOS

| 编号 | 操作 / 观察目标 | 实际结果 | 证据 | 状态 |
|---|---|---|---|---|
| HAL-001 | 当前新工程 GPIO、TIM3 PWM、RTC、UART 基本运行 | 开发者已完成并报告正常；UART RX DMA+IDLE 收帧不在此范围 | 开发者反馈、`evidence/day04/`；Day 5 手册记录范围 | PASS_USER |
| HAL-002 | ADC 电池误差、SPI 阻塞/DMA、其余 EXTI | 尚未取得完整独立实测 | 待补 | PENDING |
| RTOS-001 | 内核固定版本、Port/Heap、构建及 ABI | V11.3.0/9b777ae；CM4F/heap_4；Debug/Release 全量 49 步均通过；硬浮点 ABI 正确 | `daily/2026-09-14-day05.md`、`day05_evidence_manifest.md` | PASS |
| RTOS-002 | 双任务调度、阻塞与抢占 | 低/高任务 +50/+2016 对应 Tick +10079；Idle 持续运行；源码中低任务忙等期间高任务仍可调度 | `evidence/day05/g_day05_rtos_stats.txt`；单通道波形仅辅助 | PASS |
| RTOS-003 | TIM10/SysTick 分工、向量入口、栈与 Heap | Handler 各一份来自 port.c；1 kHz 时基参数正确；双任务余栈 229 words、最小剩余 Heap 22296 B | `GDB_log.txt`、`elf-static-closeout.txt`、正常统计日志 | PASS |
| RTOS-004 | Malloc Failed Hook 注入 | failure=MALLOC | `evidence/day05/malloc-failed-hook.txt` | PASS |
| RTOS-005 | Stack Overflow Hook 注入 | failure=STACK_OVERFLOW | `evidence/day05/stack-overflow-hook.txt` | PASS |
| RTOS-006 | 不允许的 IRQ 优先级调用 FromISR | failure=ASSERT，port.c:888 与优先级断言对应 | `evidence/day05/irq-priority-assert.txt` | PASS |
| RTOS-007 | 恢复正常代码与故障开关 | 故障开关为 0；IRQ 恢复 5、删除测试回调；双配置构建/静态审核通过，收尾版 KEY1 实板复查留作 Day 6 起步 | Day 5 收尾源码、构建日志 | PASS_STATIC |
| RTOS-008 | 五类 IPC 与软件定时器 | 下一日实现，尚未验证 | Day 6 指导手册 | PENDING |

`PASS_USER` 仅标记开发者报告的基本硬件测试；`PASS_STATIC` 仅表示构建/静态检查。Day 5 内核检查点通过，Day 4 G2 保留 ADC/SPI 等延期项，Day 6 G3 需 IPC 验收后才通过。
