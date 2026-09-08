# 测试矩阵

更新日期：2026-09-08。实机测试均因近期无硬件而 `BLOCKED_HW`。建立测试行或交叉编译通过不表示硬件功能通过。

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
| BOOT-001 | 启动 | 录像启动 3 次，记录耗时、默认页、重启异常 | 未执行 | 待提供 | BLOCKED_HW |
| BOOT-002 | 启动 | 由现有 Bootloader 连续跳转新应用 10 次，并核对 MSP、PC 和 VTOR | 静态固件布局已通过；尚未连接硬件 | `daily/2026-09-08-day03.md` | BLOCKED_HW |
| DBG-001 | 调试 | 使用 VS Code、Cortex-Debug、OpenOCD 和 ST-Link 验证断点、单步、变量、寄存器及调用栈 | CMake、OpenOCD、SVD、任务和启动项路径已静态核对；尚未连接探针 | `.vscode/tasks.json`、`.vscode/launch.json` | BLOCKED_HW |
| LCD-001 | 显示 | 页面颜色、方向、亮度和可见缺陷 | 未执行；P169H002-CTP 与 50 Mbit/s 已确认为复现参数 | 待提供 | BLOCKED_HW |
| TCH-001 | 输入 | 中心/边缘点按、四向滑动及返回 | 未执行 | 待提供 | BLOCKED_HW |
| TCH-002 | 输入 | 两个按键短按/长按及唤醒 | 未执行 | 待提供 | BLOCKED_HW |
| SNS-001 | 传感器 | 温湿度、气压、指南针的数值/单位/响应趋势 | 未执行 | 待提供 | BLOCKED_HW |
| SNS-002 | 传感器 | 步数变化和心率页的等待/更新行为 | 未执行 | 待提供 | BLOCKED_HW |
| COM-001 | 蓝牙 | 开关、发现、连接及已有数据交互 | 未执行 | 待提供 | BLOCKED_HW |
| COM-002 | 串口 | 确认实际速率并保存原始数据；无输出如实记载 | 未执行 | 待提供 | BLOCKED_HW |
| MEM-001 | 存储 | 设置修改、重进页面、重启保留，最后恢复设置 | 未执行 | 待提供 | BLOCKED_HW |
| PWR-001 | 功耗行为 | 静置变暗/息屏时间及按键唤醒 | 未执行；没有电流测量 | 待提供 | BLOCKED_HW |
| PWR-002 | 功耗行为 | 抬腕开关及运动唤醒响应 | 未执行 | 待提供 | BLOCKED_HW |
| PWR-003 | 功耗行为 | 插拔充电线，记录图标、页面和息屏差异 | 未执行；PA2 暂按 VBUS/充电电源存在解释 | 待提供 | BLOCKED_HW |
| UI-001 | 页面 | 遍历可见 UI，记录实际入口和退出方式 | 未执行 | 待提供 | BLOCKED_HW |
| UI-002 | SPO2 展示 | 佩戴/移开时 0/15/30/60 秒显示值 | 未执行 | 待提供 | BLOCKED_HW |
| UI-003 | NFC 展示 | 卡片内容和 UI 开关行为 | 未执行 | 待提供 | BLOCKED_HW |
| GAME-001 | 游戏 | 2048、记忆游戏、计算器操作与重进 | 未执行 | 待提供 | BLOCKED_HW |

实测步骤与逐项模板见 `reference_behavior.md`。每个 PASS/FAIL 需要固件身份、实际结果和证据；后续移植测试补充固件 commit 与 Debug/Release 配置。

Day 1 已覆盖启动、显示、输入、传感器、蓝牙、存储、功耗七类观察项。**G0 尚未通过**；用户已确认实板与原理图对应，仍需补齐旧 BIN 黑盒基线和电气行为证据。
