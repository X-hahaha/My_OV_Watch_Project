# 移植日志

## 2026-09-05 / Day 1

- 新仓库初始分支 `main`，起始 commit `c3acd96`，开始时工作区干净。已有文件为硬件器件表和误命名的 `.gitignore.txtfile`。
- 将忽略文件改为 `.gitignore`，保留原规则并修正 `.vscode` 白名单；增加本地工具与证据目录忽略规则、`.gitattributes` 和 README。
- 读取计划和硬件表，形成硬件映射初稿、候选固件指纹、旧算法库检查、七类黑盒测试项。
- 核实旧工程的 TIM1 时基、SPI DMA 宽度差异、USART1 波特率差异、VTOR 两处不一致，以及 PB1 的触摸 INT 宏/看门狗使能冲突，见 `plan_errata.md`。
- 用户确认 CMake、Ninja、调试服务器尚未安装，要求协助安装；采用本仓库 `.tools` 下的固定版本 ZIP 包和终端环境脚本。
- 用户确认今天无法连接或操作硬件。全部真实设备观察仍待执行，未生成、烧录或验证新固件。
- 当前准备的代码只涉及安装和核验脚本；HAL、FreeRTOS、LVGL 实现尚未开始。
- 安装结束：CMake 4.4.3、Ninja 1.13.2、xPack OpenOCD 0.12.0-7，下载包散列均通过。工具命令检查与 CMake/Ninja/GCC 最小目标文件编译通过，ARM 架构和浮点 ABI 已核实，详见 `toolchain.md`。
- 用户终端复核工具链通过；`ST-LINK_gdbserver` 缺失由已通过的 OpenOCD 路径替代，实板探针连接仍未验证。
- 收到 `SCH_Core.pdf` 与 `SCH_Back.pdf` 后完成静态交叉核对：PA4 是 `WAKE`/电源键状态，SW2 硬件直接拉起 `TPS_EN`，PA3 `POWER_EN` 在启动后自锁；PB1 是低有效 `#WDOG_EN`；PA8 是蓝牙供电开关控制；PC13 是 NC；PA2 来自低有效 `CHRG#`，与旧源码的高有效注释疑似相反，留待实测。
- 完成 Day 1 待提交文件、忽略项和暂存快照审查；原始日志与本机工具未纳入 Git，G0 未通过前不创建基线 Tag。
- 用户决定新工程将 HAL 时基改为 TIM10，以释放高级定时器 TIM1；旧源码使用 TIM1 的事实保留为对照，Day 2 需验证共享中断处理函数实际操作 `htim10`。
- 完成 Day 2 CubeMX 配置清单：固定本机已有的 STM32CubeF4 V1.28.2、CMake/GCC、TIM10、DMA 和安全 GPIO 初始电平；尚未启动 CubeMX 或生成 MCU 工程。
- 新增只读 `tools/check-day02-config.ps1`；完整模拟生成目录通过，旧参考工程作为反例失败，无 `.ioc` 场景返回未就绪，退出码依次为 0、1、2。模拟路径覆盖 `htim10`、共享 IRQ、引脚宏、启动电平和初始化顺序；真实生成工程仍待检查。

## 2026-09-06 / Day 2

- 用户在 `project/my_project` 创建 `my_project.ioc` 并生成 Basic / CMake / GCC HAL 工程；使用 STM32CubeMX 6.15.0 与 STM32CubeF4 V1.28.2。
- 首次真实检查中的 UART 和五个生成文件失败均为检查器假阴性：CubeMX 对默认 115200 不写 `USART1.BaudRate`，但 `Src/usart.c` 明确生成 `huart1.Init.BaudRate = 115200`；工程采用 `Src/Inc`，旧检查器只查 `Core/Src/Core/Inc`。
- 修订 `check-day02-config.ps1`：递归发现仓库内唯一 `.ioc`，同时支持两种 CubeMX 目录布局，并在 `.ioc` 省略默认波特率时核对生成代码；新增 TIM3 CH3/Pulse 和 CHARG EXTI 检查。
- 修订后首次检查确认 TIM10、100 MHz 时钟、SPI、DMA、UART 115200、ADC、RTC、引脚映射、启动电平、初始化顺序和 TIM3 PWM 均正确；当时唯一真实问题是 PA2 仍为普通 Input + Pull-down。PA2 后续已改为双边沿 EXTI2 + Pull-down 并重新生成，2026-09-08 复核全部通过。
- 载入项目工具环境后执行 `cmake --preset Debug` 与 `cmake --build --preset Debug`，38 个编译/链接步骤通过；当前 HAL 骨架使用 RAM 2280 B、Flash 21436 B。Day 3 修改到 `0x0800C000` 前不烧录。
- 用户确认当前实板对应 `OV-Watch_V2.4` 原理图；LCD 型号为 P169H002-CTP，可沿用旧工程 SPI 50 Mbit/s；电池分压 1:2、Vref 3.3 V，保留 `raw * 2 * 3.3 / 4096`。
- 重新沿完整状态指示支路分析 PA2：它接在 `CHRG#` 串联电阻与 LED 之间，充电时开漏下拉会使 PA2 节点为高，旧代码的高有效读取有依据；该脚更可能表示 VBUS/充电电源存在，充电中与充满的区别仍需三状态实测。
- 因未来数日可能没有硬件，15 天计划改为 PC 工作包与 `BLOCKED_HW` 补测队列并行管理；继续推进构建、链接、静态检查、纯逻辑测试和可编译移植，不提前宣称硬件功能通过。

## 2026-09-08 / Day 3

- 完成应用区 GNU 构建：Flash 从 `0x0800C000` 开始、长度 `0x74000`，SRAM 为 `0x20000000`/`0x20000`；Debug 和 Release 均从空缓存完成 38 步构建并生成 ELF、HEX、BIN、MAP。
- Debug 使用 RAM 2280 B、Flash 13748 B；Release 使用 RAM 2280 B、Flash 11956 B。Cortex-M4F、VFPv4-D16 和 hard-float ABI 检查通过。
- `.isr_vector` 位于 `0x0800C000`，初始 MSP 为 `0x20020000`；Debug/Release Reset 向量去除 Thumb 位后分别匹配 `0x0800F194`/`0x0800EA94` 的 `Reset_Handler`。
- 修正 VTOR 配置宏为 `VECT_TAB_OFFSET=0x0000C000U`；反汇编确认 `SystemInit()` 在启动早期写入 `0x0800C000`，`main()` 中保留 `DSB/ISB` 防御性设置。
- 完成 VS Code Task、Cortex-Debug、OpenOCD、SVD 和 CMake Tools 静态配置；Task 可独立加载项目环境，CMake Tools 使用仓库内 CMake/Ninja。已有旧构建缓存以 `--fresh` 重建。
- Day 2 检查器最终全部 PASS。Day 3 PC Gate 为 PASS；Bootloader 连续跳转和 GDB 调试仍为 `BLOCKED_HW`，暂不创建 `v0.3.0-gnu-boot` Tag。

后续每次移植记录：变更原因、涉及模块、验证命令、实际结果、遗留项和对应 commit。
