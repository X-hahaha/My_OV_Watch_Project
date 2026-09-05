# OV-Watch 软件复现

基于 STM32F411CEU6，从纯 HAL 工程开始，手动移植 FreeRTOS 与 LVGL，复现参考手表的软件功能。当前处于 **Day 1：环境与基线准备**，尚未生成 MCU 工程。

## 今天从这里开始

1. 阅读 [Day 1 执行记录](Docs/daily/2026-09-05-day01.md)，按其中的步骤继续。
2. 工具已安装，按 [工具链记录](Docs/toolchain.md) 在 VSCode 终端执行 `. ./tools/enter-env.ps1` 和 `./tools/check-toolchain.ps1`。
3. 用旧固件完成 [黑盒行为记录](Docs/reference_behavior.md) 和 [测试矩阵](Docs/test_matrix.md)。填写真实结果和证据后再判断 Day 1 是否通过。

## 技术约定

| 项目 | 目标 |
|---|---|
| MCU / 时钟 | STM32F411CEU6 / 100 MHz |
| 应用 Flash | `0x0800C000` 起，长度 `0x74000`，保留现有 Bootloader |
| SRAM | `0x20000000` 起，长度 `0x20000` |
| HAL 时基 | TIM1；计划原表格中的 TIM10 已在本仓库副本中修正 |
| 构建 | Arm GNU Toolchain 15.2.Rel1 + CMake + Ninja |
| FreeRTOS | 官方 FreeRTOS-Kernel V11.3.0，手动移植，原生 API |
| LVGL | 官方 v8.4.0，手动移植 |
| CubeMX | 生成 CMSIS、HAL、启动代码、时钟和外设初始化 |
| 功能边界 | SPO2 作为模拟显示；NFC 作为无硬件的页面功能，实测表现仍需记录 |

以上依赖版本是计划目标，尚未下载或核验依赖 tag。Day 5 / Day 8 集成时记录实际 commit 和许可证。

## 文档索引

- [15 天计划副本](Docs/OV_Watch_15_Day_Reproduction_Plan.md)
- [计划和参考源码差异](Docs/plan_errata.md)
- [硬件映射初稿](Docs/hardware_map.md)
- [参考固件标识](Docs/reference_firmware.md)
- [旧算法库检查](Docs/legacy_libraries.md)
- [移植日志](Docs/porting_log.md)

本地参考工程为 `../gpt/OV_Watch`。本次工作只读取参考工程；新实现放在本仓库。参考 HEX/BIN 的可观察行为与参考源码分开验证；当前源码尚未证明能重建相同固件。

## 文件和 Git

源代码与新文档使用 UTF-8；`.gitattributes` 管理换行，不批量改写旧工程的混合编码文件。`.gitignore` 排除构建产物、本机路径、原始日志和视频；`Docs/evidence/README.md` 说明证据管理方法。

Day 1 实测和工具链验收完成后再创建 `v0.1.0-baseline`。目前没有构建入口，Day 2 生成 HAL 工程，Day 3 建立 CMake 构建与启动验证。
