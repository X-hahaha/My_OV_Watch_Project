# OV-Watch 软件复现

基于 STM32F411CEU6，从纯 HAL 工程开始，手动移植 FreeRTOS 与 LVGL，复现参考手表的软件功能。Day 2 CubeMX HAL 骨架和 Day 3 GNU 构建的 PC 端检查均已通过；应用位于 `0x0800C000`，Debug/Release 构建及 ELF 静态审计完成。Day 1 黑盒基线和 Day 3 启动调试实测因无硬件保留为 `BLOCKED_HW`。

## 当前检查点

1. [Day 2 执行记录](Docs/daily/2026-09-06-day02.md) 已关闭 PA2 `CHARG` EXTI 问题，检查器结果为 PASS。
2. [Day 3 执行记录](Docs/daily/2026-09-08-day03.md) 保存构建、链接地址、VTOR 和 VS Code 配置结论。
3. Day 4 按单独的每日任务推进；所有实板结果继续写入 [测试矩阵](Docs/test_matrix.md) 的 `BLOCKED_HW` 队列，取得设备后再补测。

## 技术约定

| 项目 | 目标 |
|---|---|
| MCU / 时钟 | STM32F411CEU6 / 100 MHz |
| 应用 Flash | `0x0800C000` 起，长度 `0x74000`，保留现有 Bootloader |
| SRAM | `0x20000000` 起，长度 `0x20000` |
| HAL 时基 | TIM10；APB2 计时器时钟 100 MHz，目标 1 ms Tick；释放高级定时器 TIM1 |
| 构建 | Arm GNU Toolchain 15.2.Rel1 + CMake + Ninja |
| STM32CubeF4 | V1.28.2，CubeMX 生成时复制所需 CMSIS/HAL 文件 |
| FreeRTOS | 官方 FreeRTOS-Kernel V11.3.0，手动移植，原生 API |
| LVGL | 官方 v8.4.0，手动移植 |
| CubeMX | 生成 CMSIS、HAL、启动代码、时钟和外设初始化 |
| 功能边界 | SPO2 作为模拟显示；NFC 作为无硬件的页面功能，实测表现仍需记录 |

工具链与 STM32CubeF4 已按表中版本核验。FreeRTOS 和 LVGL 尚未集成；Day 5 / Day 8 固定源码版本时记录实际 commit 和许可证。

## 文档索引

- [15 天计划副本](Docs/OV_Watch_15_Day_Reproduction_Plan.md)
- [计划和参考源码差异](Docs/plan_errata.md)
- [硬件映射](Docs/hardware_map.md)
- [Day 2 CubeMX 配置清单](Docs/day02_cubemx_checklist.md)
- [Day 2 执行记录](Docs/daily/2026-09-06-day02.md)
- [Day 3 执行记录](Docs/daily/2026-09-08-day03.md)
- [参考固件标识](Docs/reference_firmware.md)
- [旧算法库检查](Docs/legacy_libraries.md)
- [移植日志](Docs/porting_log.md)

本地参考工程为 `../gpt/OV_Watch`。本次工作只读取参考工程；新实现放在本仓库。参考 HEX/BIN 的可观察行为与参考源码分开验证；当前源码尚未证明能重建相同固件。

## 文件和 Git

源代码与新文档使用 UTF-8；`.gitattributes` 管理换行，不批量改写旧工程的混合编码文件。`.gitignore` 排除构建产物、本机路径、原始日志和视频；`Docs/evidence/README.md` 说明证据管理方法。

Day 1 的硬件基线 Tag 仍等待实板证据。CubeMX 生成工程位于 `project/my_project`，可通过 Debug/Release CMake Preset 构建；Day 3 已完成应用区布局、ELF/HEX/BIN/MAP 和 VS Code 静态调试配置，Bootloader 与 GDB 实测等待硬件。
