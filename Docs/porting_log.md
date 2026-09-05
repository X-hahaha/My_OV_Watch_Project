# 移植日志

## 2026-09-05 / Day 1

- 新仓库初始分支 `main`，起始 commit `c3acd96`，开始时工作区干净。已有文件为硬件器件表和误命名的 `.gitignore.txtfile`。
- 将忽略文件改为 `.gitignore`，保留原规则并修正 `.vscode` 白名单；增加本地工具与证据目录忽略规则、`.gitattributes` 和 README。
- 读取计划和硬件表，形成硬件映射初稿、候选固件指纹、旧算法库检查、七类黑盒测试项。
- 核实 TIM1 时基、SPI DMA 宽度差异、USART1 波特率差异、VTOR 两处不一致，以及 PB1 的触摸 INT 宏/看门狗使能冲突，见 `plan_errata.md`。
- 用户确认 CMake、Ninja、调试服务器尚未安装，要求协助安装；采用本仓库 `.tools` 下的固定版本 ZIP 包和终端环境脚本。
- 用户确认今天无法连接或操作硬件。全部真实设备观察仍待执行，未生成、烧录或验证新固件。
- 当前准备的代码只涉及安装和核验脚本；HAL、FreeRTOS、LVGL 实现尚未开始。
- 安装结束：CMake 4.4.3、Ninja 1.13.2、xPack OpenOCD 0.12.0-7，下载包散列均通过。工具命令检查与 CMake/Ninja/GCC 最小目标文件编译通过，ARM 架构和浮点 ABI 已核实，详见 `toolchain.md`。
- 用户终端复核工具链通过；`ST-LINK_gdbserver` 缺失由已通过的 OpenOCD 路径替代，实板探针连接仍未验证。
- 收到 `SCH_Core.pdf` 与 `SCH_Back.pdf` 后完成静态交叉核对：PA4 是 `WAKE`/电源键状态，SW2 硬件直接拉起 `TPS_EN`，PA3 `POWER_EN` 在启动后自锁；PB1 是低有效 `#WDOG_EN`；PA8 是蓝牙供电开关控制；PC13 是 NC；PA2 来自低有效 `CHRG#`，与旧源码的高有效注释疑似相反，留待实测。
- 完成 Day 1 待提交文件、忽略项和暂存快照审查；原始日志与本机工具未纳入 Git，G0 未通过前不创建基线 Tag。

后续每次移植记录：变更原因、涉及模块、验证命令、实际结果、遗留项和对应 commit。
