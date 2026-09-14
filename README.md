# OV-Watch 软件复现

基于 STM32F411CEU6，从纯 HAL 工程开始，手动移植 FreeRTOS 与 LVGL，复现参考手表的软件功能。当前完成 Day 5 FreeRTOS Kernel 最小移植与审核，下一步为 Day 6 原生 IPC 训练。应用位于 `0x0800C000`，保留现有 Bootloader。

## 当前检查点

1. [Day 2 执行记录](Docs/daily/2026-09-06-day02.md) 已关闭 PA2 `CHARG` EXTI 问题，检查器结果为 PASS。
2. [Day 3 执行记录](Docs/daily/2026-09-08-day03.md) 保存构建、链接地址、VTOR 和 VS Code 配置结论。
3. 硬件已到位，开发者报告 Bootloader、新应用调试及 GPIO/PWM/RTC/UART 基本验证正常。实验仅由 ST-Link 供电；ADC、SPI DMA、充电与低功耗等未完成项目继续保留，Day 4 G2 不因跳日自动通过。
4. [Day 5 执行记录](Docs/daily/2026-09-14-day05.md) 保存双任务、时基、三个故障实验及收尾审核结论。检查点为 `v0.5.0-freertos-kernel`；审核后的小幅清理已重新全量构建，未由助手重新烧录实板。

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

工具链与 STM32CubeF4 已按表中版本核验。FreeRTOS 内核已集成并通过 submodule 固定版本；LVGL 尚未集成，Day 8 固定源码版本时记录实际 commit 和许可证。

## 文档索引

- [15 天计划副本](Docs/OV_Watch_15_Day_Reproduction_Plan.md)
- [计划和参考源码差异](Docs/plan_errata.md)
- [硬件映射](Docs/hardware_map.md)
- [Day 2 CubeMX 配置清单](Docs/day02_cubemx_checklist.md)
- [Day 2 执行记录](Docs/daily/2026-09-06-day02.md)
- [Day 3 执行记录](Docs/daily/2026-09-08-day03.md)
- [Day 5 移植指导手册](Docs/Day05_FreeRTOS_Manual_Port_Guide.md)
- [Day 5 执行记录](Docs/daily/2026-09-14-day05.md)
- [Day 5 证据指纹](Docs/day05_evidence_manifest.md)
- [第三方源码版本](Docs/third_party.md)
- [测试矩阵](Docs/test_matrix.md)
- [参考固件标识](Docs/reference_firmware.md)
- [旧算法库检查](Docs/legacy_libraries.md)
- [移植日志](Docs/porting_log.md)

本地参考工程为 `../gpt/OV_Watch`。本次工作只读取参考工程；新实现放在本仓库。参考 HEX/BIN 的可观察行为与参考源码分开验证；当前源码尚未证明能重建相同固件。

## 文件和 Git

手动引入的第三方库统一放在 `project/my_project/ThirdParty`，FreeRTOS 内核通过 submodule 固定在 V11.3.0。首次克隆后从仓库根目录执行 `git submodule update --init --checkout -- project/my_project/ThirdParty/FreeRTOS-Kernel`。CubeMX 的 `Middlewares`、`Utilities` 是保留目录，重新生成时可能被清理，不能用于存放手动维护的库；见 [ST 官方说明](https://dev.st.com/stm32cube-docs/stm32cubemx/6.18.1/en/docs/markup/CubeMX_UserManual/chapters/04_4_stm32cubemx_user_interface.html)。

源代码与新文档使用 UTF-8；`.gitattributes` 管理换行，不批量改写旧工程的混合编码文件。`.gitignore` 排除构建产物、本机路径、原始日志和视频；`Docs/evidence/README.md` 说明证据管理方法。

CubeMX 生成工程位于 `project/my_project`，业务代码使用实际目录 `APP`。开发者负责实现、烧录和实验，助手负责每日审核、记录与下一日指导；每日检查点按已取得的证据打标签。Day 1 详细黑盒清单和 Day 3 连续启动/调试尾项仍按测试矩阵补齐。
