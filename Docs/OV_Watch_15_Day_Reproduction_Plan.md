# OV-Watch 软件复现与优化 15 天执行计划

> 文档版本：v1.3（2026-09-06 增加无硬件期间的 PC 端推进路线；HAL 时基固定为 TIM10，勘误见 plan_errata.md）
> 制定日期：2026-09-04
> 计划周期：15 天，每天约 6 小时，总计约 90 小时
> 目标平台：STM32F411CEU6 + VSCode + CMake + Ninja + Arm GNU Toolchain
> 参考工程：当前 `OV_Watch` 目录
> 新工程建议目录：`my_project`

## 1. 项目目标

从一个不启用 CubeMX FreeRTOS/LVGL 中间件的新 STM32CubeMX 工程开始，完成 OV-Watch 软件功能复现，并在功能等价后进行一次可量化的架构与可靠性优化。

本项目以学习和巩固为主要目的，重点包括：

- 掌握 Cortex-M4F 上 FreeRTOS Kernel 的手动移植过程。
- 使用 FreeRTOS 原生 API 理解任务调度、队列、任务通知、互斥锁、事件组、软件定时器和中断约束。
- 掌握 LVGL 官方源码集成、显示接口、输入接口、Tick、Timer Handler、内存和线程安全要求。
- 重新验证 STM32 的 GPIO、EXTI、ADC、RTC、TIM/PWM、SPI、DMA、UART 和低功耗模式。
- 理解并复用现有 UI、游戏和算法代码。
- 建立可复现的 CMake/GNU 构建、调试、测试和 Git 版本管理流程。
- 在功能复现后优化任务结构、GUI 线程安全、软件 I2C 并发、DMA 刷屏、看门狗和内存使用。

## 2. 已确认的范围

### 2.1 必须完成

- 应用继续从 `0x0800C000` 启动，并由现有 Bootloader 跳转。
- CubeMX 只负责生成 CMSIS、HAL、启动文件、时钟和 MCU 外设初始化代码。
- FreeRTOS 使用官方 FreeRTOS-Kernel V11.3.0 源码手动移植。
- 不使用 CMSIS-RTOS2 封装，业务层统一使用 FreeRTOS 原生 API。
- LVGL 使用官方 v8.4.0 源码手动移植。
- 使用 VSCode、CMake、Ninja、Arm GNU Toolchain 构建和调试。
- 恢复 LCD、触摸、RTC、电量、背光、按键、充电检测、EEPROM、蓝牙、传感器、心率、休眠和唤醒。
- 复用并恢复全部现有 UI 页面、字体和图片资源。
- 复用并恢复 2048、记忆游戏、计算器等应用逻辑。
- 建立构建、运行、硬件和压力测试记录。

### 2.2 明确按模拟功能复现

- SPO2：保留页面、交互和模拟显示值，不要求真实血氧算法。
- NFC：硬件没有 NFC 芯片，只保留页面、卡片数据和 UI 开关交互。

### 2.3 暂不纳入本周期

- LVGL 8.4 到 LVGL 9.x 的 API 升级。
- 自研医学级心率/血氧算法。
- 为旧 ARMCC 算法库逆向适配 GNU 工具链。
- 新硬件设计或 PCB 修改。
- 手机 App 新功能开发。

## 3. 基准与事实来源

现有功能是通过目录中的 HEX/BIN 验证的，尚未证明当前源码可以重新生成同样行为。因此使用两种基准：

1. 现有 HEX/BIN 是外部可观察行为基准。
2. 当前源码是接口、引脚、业务流程和资源的移植参考。

如果二者冲突，先记录差异，再结合原理图、逻辑分析仪结果和产品目标作出决定，禁止无记录地猜测。

当前已知源码风险：

- SPO2 测量路径被固定条件禁用，当前只具有页面效果。
- NFC 没有硬件驱动。
- `Hr_Ave_Filter()` 存在累加变量未初始化和缺少返回值问题。
- 三个 EM7028 `.lib` 未加入当前 Keil 工程，其中至少一个不能被 GNU Binutils 识别。
- `.ioc` 中 SPI DMA 数据宽度与最终 `spi.c` 不一致。
- 部分源码可能存在混合编码、文件名大小写不一致和编译器扩展差异。
- 原工程在调度器暂停期间执行长时间硬件初始化，不应原样保留到优化版本。

## 4. 固定技术决策

| 项目 | 决策 |
|---|---|
| MCU | STM32F411CEU6，Cortex-M4F |
| 系统时钟 | 100 MHz，复现阶段保持与旧工程一致 |
| RTC 时钟 | LSE |
| 应用 Flash | `ORIGIN = 0x0800C000`，`LENGTH = 0x74000` |
| SRAM | `ORIGIN = 0x20000000`，`LENGTH = 0x20000` |
| HAL 时基 | TIM10，避免与 FreeRTOS SysTick 冲突，并释放高级定时器 TIM1；旧源码的 TIM1 配置只作参考 |
| STM32CubeF4 | V1.28.2；CubeMX 生成时复制本工程实际需要的 CMSIS/HAL 文件 |
| FreeRTOS | V11.3.0，官方仓库固定 tag |
| FreeRTOS Port | `portable/GCC/ARM_CM4F` |
| FreeRTOS Heap | 初期 `heap_4.c`，最终根据 MAP 和峰值调整 |
| FreeRTOS API | 原生 API，不引入 `cmsis_os2.c` |
| LVGL | v8.4.0，官方仓库固定 tag |
| LVGL 颜色 | RGB565，`LV_COLOR_DEPTH=16`，验证字节交换 |
| 构建 | CMake + Ninja，禁止同时维护第二套源文件列表 |
| 编译器 | Arm GNU Toolchain 15.2.Rel1 |
| Debug 优化 | `-Og -g3` |
| Release 优化 | `-Os` |
| 调试 | Cortex-Debug + OpenOCD 或 ST-Link GDB Server |
| 版本策略 | 独立 Git 仓库，短分支、每日合并、里程碑 Tag |

## 5. 计划完成定义

满足以下全部条件才算项目完成：

### 5.1 构建和启动

- 一条命令可以生成 ELF、HEX、BIN 和 MAP。
- Debug 和 Release 两种构建配置均成功。
- `.isr_vector` 位于 `0x0800C000`。
- ELF 中 RAM/Flash 区域与链接脚本一致。
- Bootloader 能稳定跳转到新应用。
- `SCB->VTOR`、MSP、Reset Handler 和中断向量正确。
- CubeMX 中没有启用 FreeRTOS 或 LVGL 中间件。
- 项目中没有 CMSIS-RTOS2 业务 API 依赖。

### 5.2 功能

- 19 个现有 UI 源模块可以构建并完成对应页面/组件功能。
- 主页、菜单、设置、日历、计时器、环境、指南针、心率、充电、关于等页面正常。
- 2048、记忆游戏和计算器可以操作、退出和重新进入。
- SPO2 和 NFC 明确标记为模拟/展示功能。
- LCD、触摸、RTC、ADC、PWM、UART DMA、EEPROM 和主要传感器正常。
- 蓝牙开关和现有数据交互恢复。
- 息屏、休眠、充电和唤醒流程恢复。

### 5.3 稳定性

- 所有任务、队列、互斥锁和事件对象创建结果均检查。
- 不发生 FreeRTOS 栈溢出和 malloc 失败。
- 页面往返切换 100 次无 HardFault、泄漏或显示死锁。
- SPI DMA 连续刷新无 `HAL_BUSY` 死锁。
- 软件 I2C 失败后能够超时退出或恢复总线。
- 休眠/唤醒循环至少 30 次。
- Release 固件连续运行至少 6 小时。

### 5.4 可维护性和学习产出

- 有硬件引脚表、任务表、模块依赖图和消息流说明。
- 有 FreeRTOS 和 LVGL 手动移植笔记。
- 有每个阶段的测试记录和关键测量数据。
- 有优化前后 Flash、RAM、任务栈、刷新耗时和任务数量对比。
- Git 历史中每个里程碑都能独立检出和构建。

## 6. 项目目录规划

```text
my_project/
├─ CMakeLists.txt
├─ CMakePresets.json
├─ cmake/
│  └─ arm-none-eabi-gcc.cmake
├─ linker/
│  └─ STM32F411CEUx_APP.ld
├─ Core/
│  ├─ Inc/
│  └─ Src/
├─ Drivers/
│  ├─ CMSIS/
│  ├─ STM32F4xx_HAL_Driver/
│  └─ BSP/
│     ├─ lcd_st7789/
│     ├─ touch_cst816/
│     ├─ soft_i2c/
│     ├─ aht21/
│     ├─ spl06/
│     ├─ lsm303/
│     ├─ mpu6050/
│     ├─ em7028/
│     ├─ eeprom/
│     ├─ power/
│     └─ ble/
├─ Middlewares/
│  ├─ FreeRTOS-Kernel/
│  └─ lvgl/
├─ Config/
│  ├─ FreeRTOSConfig.h
│  └─ lv_conf.h
├─ App/
│  ├─ Tasks/
│  ├─ Services/
│  ├─ Model/
│  ├─ Games/
│  └─ UI/
├─ Docs/
│  ├─ hardware_map.md
│  ├─ task_design.md
│  ├─ porting_log.md
│  ├─ test_matrix.md
│  └─ daily/
└─ tools/
```

原工程保持只读参考，不将新文件直接混入旧 `Core`、`BSP`、`User` 和 `Middlewares`。

## 7. 总体实施方法

使用“两遍移植”降低定位难度：

### 第一遍：功能等价

- 保持旧工程的模块边界和业务行为。
- CMSIS-RTOS2 API 机械转换为 FreeRTOS 原生 API。
- 先修复 GNU 编译兼容问题，不立即大规模重构。
- 每接入一个模块就执行独立验收。
- 以旧 BIN 的可观察行为做对照。

### 第二遍：优化

- 必须在功能等价 Tag 之后进行。
- 优先减少轮询和任务数量。
- 保证 LVGL 单任务所有权。
- 为共享软件 I2C 增加互斥、超时和恢复。
- 为 DMA、UART、看门狗和低功耗增加明确状态机。
- 以数据证明优化结果，禁止只凭主观感受判断。

## 8. 每日时间模板

每天按约 6 小时安排：

| 环节 | 建议时间 | 内容 |
|---|---:|---|
| 昨日回顾 | 0.25 h | 检查提交、失败测试和遗留问题 |
| 原理学习 | 0.75 h | 阅读当天涉及的官方文档和关键源码 |
| 实现 | 3.5 h | 编码、构建、烧录、单步调试 |
| 验收测试 | 1.0 h | 按清单执行并保存证据 |
| 文档和 Git | 0.5 h | 更新日报、测试矩阵、提交和 Tag |

规则：当日核心验收没有通过时，不把未经确认的模块继续叠加到下一层。次日最多先使用 2 小时解决遗留问题；超过 2 小时则重新评估依赖或切换到可独立任务。

### 8.1 暂无硬件期间的执行策略（2026-09-06 起）

“Day”从连续日历日调整为工作包。近期拿不到实板时，继续完成能够在 PC 上客观验证的配置、构建、链接、静态检查、纯算法测试和接口移植；需要观察电平、波形、外设响应、Bootloader 跳转或长期运行的项目统一记为 `BLOCKED_HW`，不伪造 PASS，也不阻塞无硬件依赖的工作。

| 原工作包 | 暂无硬件时先完成 | 硬件恢复后的补测 |
|---|---|---|
| Day 1 | 工具链、仓库、原理图与参考源码静态基线 | 旧 BIN 黑盒行为、实板电平和版本证据 |
| Day 2 | CubeMX `.ioc`、生成代码静态检查、Debug 构建 | HAL Tick、UART、PWM 和启动电平 |
| Day 3 | Debug/Release 构建、应用链接脚本、ELF/HEX/BIN/MAP、`readelf`/`objdump` 检查、VSCode 配置 | Bootloader 跳转、GDB 连接、断点与寄存器验证 |
| Day 4 | 编写可选择启用的裸机诊断入口；为电池换算、PWM 映射、帧解析等纯逻辑增加主机测试 | GPIO、RTC、ADC、UART、SPI、DMA、EXTI 和波形全部实测 |
| Day 5-6 | FreeRTOS 源码、配置、原生 API 应用骨架通过交叉编译和静态检查 | 调度、Hook、ISR 唤醒、优先级和 GPIO 时序 |
| Day 7-13 | 按参考工程移植驱动与业务接口；用 mock、命令序列和资源构建验证不依赖硬件的部分 | LCD、触摸、传感器、蓝牙、EEPROM、充电、休眠和唤醒逐项上板 |
| Day 14-15 | 架构审查、编译告警、MAP/栈预算、文档与可复现构建 | 压力、功耗、循环唤醒和完整回归 |

无硬件期间允许提交 PC 端检查点，但名称必须反映实际证据。依赖运行结果的里程碑 Tag 暂缓创建；硬件恢复后按 Day 1 黑盒基线、Day 2/3 启动链、Day 4 裸机外设、后续集成的顺序补测。设备驱动即使已经编译成功，也保持 `UNVERIFIED_HW`，直到对应硬件测试通过。

## 9. 15 天详细计划

## Day 1：建立黑盒基线、工具链和 Git 仓库

### 学习目标

- 理解为什么已有 BIN 和当前源码必须分别作为行为基准与实现参考。
- 理解可复现构建需要固定哪些工具、依赖和配置。

### 工作内容

1. 安装并确认以下命令可从终端执行：
   - `arm-none-eabi-gcc`
   - `arm-none-eabi-gdb`
   - `cmake`
   - `ninja`
   - OpenOCD 或 ST-Link GDB Server
2. 创建独立的 `my_project` 目录并初始化 Git。
3. 添加 `.gitignore`、`.gitattributes`、README 和本计划文档副本。
4. 固定工具版本并记录到 `Docs/toolchain.md`。
5. 烧录旧 BIN，建立黑盒功能清单：
   - 录制完整启动和页面操作视频。
   - 保存串口输出。
   - 记录休眠、充电、按键、抬腕、蓝牙行为。
   - 确认 SPO2 是固定值还是变化值。
   - 确认 NFC 仅为 UI 功能。
6. 记录旧 BIN 的文件大小和 SHA-256。
7. 根据原理图与源码建立初版 `hardware_map.md`。
8. 对旧 `.lib` 做一次独立兼容性结论记录，但不让它阻塞主线。

### 交付物

- 新 Git 仓库。
- `README.md`。
- `Docs/toolchain.md`。
- `Docs/hardware_map.md` 初版。
- `Docs/reference_behavior.md`。
- 旧固件录屏、串口日志和散列值记录。

### 验收标准

- 所有构建和调试工具能在 VSCode 终端运行。
- Git 工作区干净。
- 能从文档中完整复述旧固件的启动、页面和休眠行为。
- 至少建立启动、显示、输入、传感器、蓝牙、存储、功耗七类测试项。

### Git 检查点

```text
chore(repo): initialize reproduction workspace
docs(baseline): record reference firmware behavior and hardware map
```

Tag：`v0.1.0-baseline`

## Day 2：创建纯 HAL CubeMX 工程

### 学习目标

- 独立完成时钟树、HAL 时基和外设资源规划。
- 理解 CubeMX 生成代码与手写中间件之间的边界。

### 工作内容

1. 创建 STM32F411CEUx 工程。
2. 明确不启用 FreeRTOS、CMSIS-RTOS 和 LVGL。
3. 配置 HSI + PLL 到 100 MHz，配置 LSE/RTC。
4. 配置 SWD。
5. 配置 SPI1、USART1、ADC1、TIM3 PWM、RTC。
6. 配置 SPI1 TX DMA、USART1 RX/TX DMA。
7. 配置 TIM10 为 HAL Timebase。
8. 按原理图补齐 GPIO、EXTI 和输出初始电平。
9. 保留 CubeMX 用户代码区，生成最小 HAL 工程。
10. 先用临时构建方式确认生成代码本身无缺失。

### 验收标准

- CubeMX 工程未启用两个目标中间件。
- `SystemCoreClock` 为 100 MHz。
- HAL Tick 由 TIM10 递增；共享中断向量名为 `TIM1_UP_TIM10_IRQn`，但 TIM1 不被启用。
- SWD 不被 GPIO 配置覆盖。
- 所有 DMA Stream、Channel 和 NVIC 优先级均有记录。
- 串口能输出启动信息，背光 PWM 占空比可调。

### Git 检查点

```text
feat(hal): generate minimal STM32F411 peripheral project
docs(hw): record clock DMA and interrupt assignments
```

Tag：`v0.2.0-hal-skeleton`

## Day 3：CMake、链接脚本、Bootloader 和 VSCode 调试闭环

### 学习目标

- 理解编译、汇编、链接、ELF、HEX、BIN 和 MAP 的关系。
- 掌握 Cortex-M 向量表和 Bootloader 跳转后的应用布局。

### 工作内容

1. 编写 Arm GNU CMake Toolchain 文件。
2. 编写顶层 `CMakeLists.txt` 和 Debug/Release Preset。
3. 编写应用链接脚本：

   ```ld
   FLASH (rx)  : ORIGIN = 0x0800C000, LENGTH = 0x74000
   RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 0x20000
   ```

4. 配置 Cortex-M4F 编译参数。
5. 配置 `--gc-sections`、MAP 和内存统计输出。
6. 自动生成 ELF、HEX、BIN。
7. 在应用早期设置 VTOR，并加入 `DSB/ISB`。
8. 配置 VSCode 调试启动项。
9. 使用 GDB 检查 MSP、PC、VTOR、Reset Handler。
10. 通过 Bootloader 启动至少 10 次。

### 验收标准

- `cmake --preset Debug` 和 `cmake --build --preset Debug` 成功。
- `.isr_vector` 位于 `0x0800C000`。
- 初始 MSP 位于 SRAM 范围。
- Bootloader 连续 10 次跳转成功。
- 断点、单步、变量、寄存器和调用栈可用。
- Debug/Release 构建产物分目录保存。

### Git 检查点

```text
build(cmake): add GNU Arm reproducible build
feat(boot): locate application at 0x0800C000
chore(debug): add VSCode Cortex debug configuration
```

Tag：`v0.3.0-gnu-boot`

## Day 4：裸机外设最小验证

### 学习目标

- 重新建立每个 MCU 外设的独立可测基线。
- 在引入 RTOS 前排除引脚、时钟、DMA 和中断错误。

### 工作内容

按顺序完成最小测试：

1. GPIO 输入输出和关键默认电平。
2. TIM3 PWM 背光，验证 0%、25%、50%、75%、100%。
3. RTC 走时、日期读写和 Wakeup 中断。
4. ADC 电池通道，多次采样并计算电压。
5. USART1 阻塞收发。
6. USART1 DMA + IDLE 接收。
7. SPI1 阻塞发送。
8. SPI1 DMA 发送及完成中断。
9. 按键、充电、MPU 相关 EXTI。
10. 使用逻辑分析仪记录 UART、SPI、PWM 波形。

### 验收标准

- 所有外设拥有独立测试入口和 PASS/FAIL 日志。
- RTC 运行 30 分钟无明显异常。
- ADC 结果稳定且换算关系有记录。
- PWM 频率和占空比与计算一致。
- UART DMA+IDLE 能处理短帧和连续帧。
- SPI DMA 长度和字节宽度与 HAL 配置一致。
- 中断服务函数不包含阻塞操作。

### Git 检查点

```text
test(hal): add peripheral bring-up diagnostics
fix(dma): align SPI DMA width with runtime configuration
```

Tag：`v0.4.0-peripheral-baseline`

## Day 5：手动移植 FreeRTOS Kernel

### 学习目标

- 理解 FreeRTOS Kernel 必需源码、portable 层和内存管理器。
- 理解 SVC、PendSV、SysTick 在上下文切换中的职责。

### 工作内容

1. 从官方仓库固定 FreeRTOS-Kernel V11.3.0。
2. 加入必需内核源码：
   - `tasks.c`
   - `queue.c`
   - `list.c`
   - `timers.c`
   - `event_groups.c`
   - `stream_buffer.c`
3. 加入 `portable/GCC/ARM_CM4F`。
4. 加入 `portable/MemMang/heap_4.c`。
5. 手写并逐项理解 `FreeRTOSConfig.h`。
6. 映射 SVC、PendSV 和 SysTick Handler。
7. 确保 HAL Tick 仍来自 TIM10。
8. 创建两个测试任务，验证优先级、阻塞和抢占。
9. 增加 Idle Hook、Tick Hook、Stack Overflow Hook、Malloc Failed Hook。
10. 通过 GPIO 波形测量任务切换和周期。

### 初始配置建议

- `configTICK_RATE_HZ = 1000`
- `configMAX_PRIORITIES = 8`
- `configUSE_PREEMPTION = 1`
- `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY = 5`
- `configCHECK_FOR_STACK_OVERFLOW = 2`
- `configUSE_MALLOC_FAILED_HOOK = 1`
- `configUSE_TIMERS = 1`
- `configUSE_MUTEXES = 1`
- `configUSE_COUNTING_SEMAPHORES = 1`
- `configUSE_RECURSIVE_MUTEXES = 0`，除非实际证明需要
- `configUSE_TICKLESS_IDLE = 0`，复现稳定后再开启

### 验收标准

- 工程不存在 `cmsis_os.h`、`cmsis_os2.h` 和 `cmsis_os2.c` 依赖。
- 两个任务按预期抢占和阻塞。
- SysTick 为 1 kHz，HAL Tick 与 RTOS Tick 互不干扰。
- 人为减小任务栈能触发 Stack Overflow Hook。
- 人为耗尽 Heap 能触发 Malloc Failed Hook。
- 错误中断优先级能被 `configASSERT` 捕获。

### Git 检查点

```text
feat(rtos): manually port FreeRTOS Kernel V11.3.0
test(rtos): verify scheduling hooks and failure detection
```

Tag：`v0.5.0-freertos-kernel`

## Day 6：FreeRTOS 原生 API 训练与应用骨架

### 学习目标

- 通过硬件事件理解不同 IPC 工具的适用场景。
- 掌握 ISR 与任务之间的正确通信方式。

### 工作内容

1. 按键 EXTI 使用 `xTaskNotifyFromISR()` 唤醒任务。
2. UART DMA+IDLE 数据写入 Stream Buffer。
3. 建立传感器数据生产者/消费者队列。
4. 建立软件 I2C 总线互斥锁。
5. 使用 Event Group 表示初始化完成、充电、唤醒和任务健康状态。
6. 使用软件定时器完成一个低频非阻塞事件。
7. 建立应用任务入口和统一任务创建函数。
8. 将旧 API 做机械映射：

   | 旧 API | 新 API |
   |---|---|
   | `osThreadNew` | `xTaskCreate` |
   | `osDelay` | `vTaskDelay(pdMS_TO_TICKS())` |
   | `osMessageQueueNew` | `xQueueCreate` |
   | `osMessageQueuePut` | `xQueueSend`/`xQueueOverwrite` |
   | `osMessageQueueGet` | `xQueueReceive` |
   | `osEventFlagsNew` | `xEventGroupCreate` |
   | `osEventFlagsSet` | `xEventGroupSetBits` 或 FromISR 版本 |
   | `osEventFlagsWait` | `xEventGroupWaitBits` |
   | `osKernelStart` | `vTaskStartScheduler` |

9. 记录 FreeRTOS 栈深度单位为 `StackType_t`，完成旧字节数到新栈深度的换算。

### 验收标准

- 五类 IPC 小实验全部通过。
- 所有 ISR 使用 `FromISR` API 和 `portYIELD_FROM_ISR()`。
- 所有等待都具有明确超时或设计为永久事件等待。
- 不在中断中打印、延时或访问 LVGL。
- 任务和 IPC 创建失败会进入可识别错误处理。

### Git 检查点

```text
feat(rtos): add native IPC and ISR communication exercises
feat(app): add initial task and service skeleton
```

Tag：`v0.6.0-rtos-foundation`

## Day 7：ST7789 显示与 SPI DMA

### 学习目标

- 理解 LCD 命令、地址窗口、RGB565、SPI 模式和 DMA 异步完成流程。

### 工作内容

1. 移植 LCD GPIO 和 ST7789 初始化序列。
2. 使用阻塞 SPI 完成纯色、色条、点、线、矩形测试。
3. 验证 240×280 方向和 Y 偏移。
4. 验证 SPI CPOL/CPHA 和最高可靠速率。
5. 实现异步 `lcd_flush_async()`。
6. 使用状态变量或同步对象保证同时只有一次 DMA 传输。
7. DMA 完成回调清理状态并通知上层。
8. 检查 `HAL_SPI_Transmit_DMA()` 返回值。
9. 测量整屏和局部区域传输耗时。

### 验收标准

- 阻塞模式色条和坐标测试正确。
- DMA 模式连续刷新至少 10 分钟无花屏。
- 不出现未处理的 `HAL_BUSY`。
- DMA 完成回调只对应 SPI1。
- 可输出刷新次数、忙冲突次数和传输耗时。
- 示波器或逻辑分析仪波形符合 SPI 配置。

### Git 检查点

```text
feat(display): port ST7789 blocking driver
feat(display): add guarded SPI DMA flush path
test(display): add color coordinate and throughput tests
```

Tag：`v0.7.0-lcd-dma`

## Day 8：手动移植 LVGL 8.4

### 学习目标

- 理解 LVGL 配置、绘图缓冲、刷新回调、Tick 和 Timer Handler。

### 工作内容

1. 从官方仓库固定 LVGL v8.4.0。
2. 从官方模板生成 `lv_conf.h`。
3. 第一轮只开启必要 widget 和默认字体。
4. 配置 RGB565 和颜色字节顺序。
5. 先建立一个约 1/10 屏幕大小的绘图缓冲。
6. 实现 `flush_cb` 并连接 ST7789 DMA。
7. DMA 完成时调用 `lv_disp_flush_ready()`。
8. 在 LVGL 初始化完成后，Tick Hook 中仅调用 `lv_tick_inc(1)`；初始化前由状态标志保护。
9. 创建唯一 GUI 任务调用 `lv_timer_handler()`。
10. 完成 Label、Button、Arc、动画和定时更新示例。
11. 单缓冲通过后再启用两个 1/10 屏幕缓冲。

### 内存预估

240×280、RGB565、两个 1/10 屏幕缓冲约占：

```text
240 × 280 ÷ 10 × 2 bytes × 2 buffers = 26,880 bytes
```

### 验收标准

- LVGL 来自官方源码且版本固定。
- 屏幕分辨率、方向、颜色和偏移正确。
- 单缓冲和双缓冲均能运行。
- 动画持续运行 30 分钟无 DMA 死锁。
- 除 `lv_tick_inc()` 和允许的刷新完成接口外，中断不调用 LVGL。
- `lv_timer_handler()` 只在 GUI 任务执行。

### Git 检查点

```text
feat(lvgl): manually integrate LVGL v8.4.0
feat(lvgl): add DMA display port and dual draw buffers
```

Tag：`v0.8.0-lvgl-display`

## Day 9：CST816 触摸与基础 UI

### 学习目标

- 理解 LVGL 输入设备读取模型和触摸坐标转换。
- 建立 UI 单线程所有权原则。

### 工作内容

1. 移植 CST816 软件 I2C 和复位流程。
2. 验证芯片 ID、触摸点、坐标和手势寄存器。
3. 注册 LVGL pointer 输入设备。
4. 验证横纵方向和边界坐标。
5. 接入 `ui.c`、PageManager、主页和菜单。
6. 建立 App → GUI 消息结构，禁止后台任务直接更新控件。
7. 处理源码编码和文件名大小写问题。
8. 建立页面创建、进入、退出、销毁规则。

### 验收标准

- 屏幕四角和中心触摸坐标正确。
- 点击、长按、滑动无明显丢失。
- 主页和菜单能够往返切换。
- 页面销毁后不再访问旧对象。
- 后台任务不直接调用 `lv_*`。
- 连续操作 30 分钟无异常。

### Git 检查点

```text
feat(input): port CST816 and LVGL input device
feat(ui): port page manager home and menu screens
```

Tag：`v0.9.0-touch-ui-base`

## Day 10：软件 I2C 与环境/姿态传感器

### 学习目标

- 理解多设备共享软件 I2C 的时序、互斥、超时和故障恢复。

### 工作内容

1. 整理三条软件 I2C 总线：
   - PB13/PB14：主要传感器共享总线。
   - PB4/PB6：触摸总线。
   - PA11/PA12：EEPROM 总线。
2. 为软件 I2C 增加：
   - 总线互斥锁。
   - ACK 超时。
   - SDA 卡低恢复脉冲。
   - 错误计数和日志。
3. 依次接入 AHT21、SPL06-001、LSM303。
4. 每个设备先验证 ID/状态，再初始化，再读取数据。
5. 对温湿度、气压、海拔和方位进行范围检查。
6. 使用逻辑分析仪保存成功和失败波形。

### 验收标准

- 三条总线相互独立且定义清晰。
- AHT21、SPL06、LSM303 初始化结果可查询。
- 多设备轮询 30 分钟无总线永久阻塞。
- 拔除或模拟无 ACK 时任务能超时返回。
- 重新接入设备后能够恢复或通过重初始化恢复。
- 传感器数据具备合理范围和错误状态。

### Git 检查点

```text
refactor(i2c): add shared bus locking timeout and recovery
feat(sensor): port AHT21 SPL06 and LSM303 drivers
```

Tag：`v0.10.0-environment-sensors`

## Day 11：MPU6050、EM7028、EEPROM、蓝牙及电源外设

### 学习目标

- 理解不同采样周期、外部中断、DMA 通信和数据持久化的组合设计。

### 工作内容

1. 移植 MPU6050 和 DMP 代码。
2. 恢复计步、抬腕状态和 MPU 中断。
3. 移植 EM7028 初始化和心率采样。
4. 修复 `Hr_Ave_Filter()`，为心率算法添加测试输入。
5. 不启用旧血氧和血压 `.lib`。
6. 移植 BL24C02 并验证掉电保存。
7. 恢复电池 ADC、充电检测和外部看门狗 GPIO。
8. 恢复 KT6328 使能和 UART DMA+IDLE 通信。
9. 定义传感器数据快照结构，避免 UI 直接访问正在更新的数据。

### 验收标准

- MPU6050 DMP 初始化成功，计步可变化。
- 抬腕中断或状态检测不在 ISR 中执行复杂计算。
- 心率原始值、峰值和最终值可以调试观察。
- 心率滤波函数无未初始化变量和缺少返回值警告。
- EEPROM 写入、重启、读取一致。
- UART 连续帧、短帧和空闲线检测正常。
- 所有传感器都有连接错误状态。

### Git 检查点

```text
feat(sensor): port MPU6050 DMP and EM7028 heart-rate path
fix(algo): correct heart-rate average filter
feat(storage): port EEPROM persistence
feat(comm): port KT6328 UART DMA protocol
```

Tag：`v0.11.0-full-hardware`

## Day 12：全部 UI、游戏和应用逻辑

### 学习目标

- 理解旧 UI、页面管理、游戏状态机和硬件数据访问层。

### 工作内容

1. 接入剩余全部页面和组件：
   - About
   - Calendar
   - Charg
   - Compass
   - Computer
   - DateTimeSet
   - Env
   - Game2048
   - GameMem
   - GameSelect
   - HR
   - Megboxes
   - NFCCard
   - OffTime
   - Set
   - SPO2
   - Timer
2. 接入 15 个自定义字体和指南针图片资源。
3. 根据 `lv_conf.h` 只启用实际使用的组件和字体。
4. 将硬件读取与 UI 更新改为消息/数据快照。
5. NFC 页面明确显示或注释为模拟功能。
6. SPO2 页面使用明确的模拟值来源。
7. 为 2048、记忆游戏和计算器分别记录核心状态机。
8. 逐页记录进入、操作、退出和销毁结果。

### 验收标准

- 所有页面均能进入和退出。
- 中文字体和图标无乱码。
- 2048 可滑动、合并、重新开始和退出。
- 记忆游戏可完整进行一轮并重新开始。
- 计算器基本运算和错误输入处理正常。
- NFC/SPO2 不宣称具有未实现的真实硬件能力。
- 页面切换 100 次无 HardFault。
- 比较旧 BIN 后形成页面差异清单。

### Git 检查点

```text
feat(ui): port all application pages fonts and image assets
feat(game): port 2048 memory game and calculator
docs(ui): document simulated NFC and SPO2 behavior
```

Tag：`v0.12.0-feature-parity-ui`

## Day 13：运行模式、充电、低功耗和完整集成

### 学习目标

- 理解 FreeRTOS、HAL Tick、RTC、外设状态和 STOP 模式之间的关系。

### 工作内容

1. 建立电源状态机：
   - Active
   - ScreenOff
   - PreparingSleep
   - StopMode
   - Waking
   - Charging
2. 恢复自动息屏和自动休眠。
3. 进入 STOP 前停止或确认完成：
   - LCD DMA
   - UART DMA
   - 传感器采样
   - 非必要外设
4. 配置 RTC、按键、MPU 和充电唤醒源。
5. 唤醒后恢复系统时钟、HAL Tick、SPI、UART、LCD 和传感器状态。
6. 恢复设置保存、当日计步保存和日期变化处理。
7. 集成充电页面、蓝牙状态和主页数据更新。
8. 对比旧 BIN 的运行行为。

### 验收标准

- 手动和自动息屏正常。
- 进入 STOP 前没有未完成 DMA。
- 四类计划唤醒源分别验证。
- 连续休眠/唤醒 30 次。
- 唤醒后触摸、屏幕、RTC、UART 和传感器正常。
- 充电插拔状态可靠且无重复页面切换。
- 设置掉电保存正确。

### Git 检查点

```text
feat(power): add explicit active sleep wake and charging state machine
feat(app): complete integrated watch behavior
test(power): verify repeated stop-mode wake cycles
```

Tag：`v1.0.0-feature-parity`

## Day 14：任务架构与可靠性优化

### 学习目标

- 用测量结果驱动 FreeRTOS 架构优化。
- 理解线程所有权、事件驱动和健康看门狗设计。

### 优化前必须记录

- 任务数量和优先级。
- 每个任务栈大小和高水位。
- 当前最小剩余 Heap。
- Flash/RAM 使用。
- GUI 刷新耗时。
- 空闲任务占比或运行时间统计。
- I2C、DMA 和 UART 错误计数。

### 工作内容

1. 将 LVGL 调用限制在 `GuiTask`。
2. 使用 GUI 消息队列处理页面和控件更新。
3. 合并职责相近的轮询任务，目标架构：
   - `GuiTask`
   - `SensorTask`
   - `InputTask`，若能由通知和 GUI 输入合并可省略
   - `CommTask`
   - `PowerTask`
   - `StorageTask`
   - `HealthTask`
4. 长度为 1、仅表达“发生了某事”的队列改为任务通知或事件位。
5. 周期任务使用 `vTaskDelayUntil()`。
6. 看门狗改为健康投票：关键任务在时间窗内全部上报后才能喂狗。
7. 根据栈高水位缩减过大的栈，并保留至少 25% 余量。
8. 增加 Reset Cause、HardFault、栈溢出和 malloc 失败诊断。
9. 不在 Tick Hook 中累计业务计时，改为 RTOS Tick 差值或软件定时器。
10. 重新测量优化指标。

### 验收标准

- 功能回归结果与 `v1.0.0-feature-parity` 一致。
- 除明确例外外，只有 GUI 任务调用 LVGL。
- 无共享 I2C 并发访问。
- 看门狗能识别一个故意卡死的关键任务并触发复位。
- 任务数量、RAM、CPU 占用或可靠性至少有一项获得可量化改善。
- 优化前后数据写入文档。

### Git 检查点

```text
refactor(rtos): consolidate polling tasks and enforce GUI ownership
feat(watchdog): feed only after critical task health votes
perf(memory): tune task stacks using high-water marks
```

Tag：`v1.1.0-architecture-optimized`

## Day 15：完整回归、压力测试和交付

### 学习目标

- 建立可重复、带证据的嵌入式项目交付流程。

### 工作内容

1. 执行完整测试矩阵。
2. 页面和游戏持续交互测试。
3. SPI DMA 刷屏、UART DMA 和传感器并发测试。
4. 软件 I2C 无 ACK 和总线卡低测试。
5. 休眠/唤醒 30 次。
6. 充电插拔 20 次。
7. 人为触发任务栈、malloc、Assert 和看门狗测试。
8. Release 固件运行至少 6 小时。
9. 检查 MAP、Flash、RAM、栈高水位和最小 Heap。
10. 从全新构建目录验证一键构建。
11. 整理 README、架构、移植、测试和已知限制。
12. 生成最终 HEX/BIN 的 SHA-256。

### 验收标准

- 测试矩阵的 P0 项全部通过。
- P1 项没有未说明失败。
- 工作区干净，Release 可从零构建。
- 最终 Tag 能独立检出和构建。
- 最终固件、MAP、散列值、测试报告和已知限制齐全。
- 对比旧 BIN 的差异均已解释。

### Git 检查点

```text
test(system): complete full hardware and stress regression
docs(release): publish architecture porting and validation report
chore(release): prepare OV-Watch reproduction release
```

Tag：`v1.2.0-reproduction-complete`

## 10. Git 版本管理规范

当前参考工程目录不是 Git 仓库。新工程应创建独立仓库，不直接在参考目录根部初始化后混合跟踪旧工程。

### 10.1 分支模型

个人、短周期项目使用轻量主干开发：

- `main`：每天结束时必须可构建，里程碑 Tag 只打在此分支。
- `feat/dayNN-topic`：当天功能分支。
- `fix/short-description`：处理已确认缺陷。
- `experiment/short-description`：可能被丢弃的实验，不直接合入主线。

示例：

```text
feat/day05-freertos-port
feat/day08-lvgl-port
fix/spi-dma-busy-state
experiment/lvgl-buffer-size
```

### 10.2 每日流程

```text
main 更新并确认干净
  -> 创建当天短分支
  -> 小步实现和本地提交
  -> 执行当天验收
  -> 更新日报和测试矩阵
  -> 查看 git diff 和提交历史
  -> 合并回 main
  -> 再次从 main 构建
  -> 必要时创建里程碑 Tag
```

禁止在验收失败时创建正式里程碑 Tag。

### 10.3 Commit 格式

```text
<type>(<scope>): <imperative summary>
```

允许类型：

- `build`：CMake、链接脚本、工具链。
- `feat`：新功能。
- `fix`：缺陷修复。
- `refactor`：不改变行为的重构。
- `test`：测试代码和测试记录。
- `docs`：文档。
- `perf`：性能或内存优化。
- `chore`：仓库维护和依赖版本。

示例：

```text
build(cmake): add Cortex-M4F GNU toolchain file
feat(rtos): add button notification from EXTI ISR
fix(display): reject overlapping SPI DMA transfers
refactor(ui): route sensor updates through GUI queue
test(power): cover repeated RTC wake-up cycles
```

一个提交只处理一个逻辑问题；禁止使用 `update`、`fix bug`、`day work` 等无法理解的说明。

### 10.4 Tag 规则

```text
v0.1.0-baseline
v0.2.0-hal-skeleton
v0.3.0-gnu-boot
v0.4.0-peripheral-baseline
v0.5.0-freertos-kernel
v0.6.0-rtos-foundation
v0.7.0-lcd-dma
v0.8.0-lvgl-display
v0.9.0-touch-ui-base
v0.10.0-environment-sensors
v0.11.0-full-hardware
v0.12.0-feature-parity-ui
v1.0.0-feature-parity
v1.1.0-architecture-optimized
v1.2.0-reproduction-complete
```

### 10.5 不应提交的文件

- `build/`、`out/` 和 CMake 临时文件。
- `.elf`、`.axf`、`.o`、`.d`、`.lst`。
- 日常生成的 `.hex`、`.bin`、`.map`。
- VSCode 用户机器相关状态。
- 临时串口日志和大型录屏原文件。
- 含绝对路径的私人配置。

正式发布固件可作为 Release 附件保存；仓库中记录版本号、文件大小和 SHA-256。

### 10.6 第三方依赖管理

优先使用 Git submodule 并固定官方 Tag：

- `Middlewares/FreeRTOS-Kernel` → V11.3.0。
- `Middlewares/lvgl` → v8.4.0。

如果不用 submodule，则将官方源码作为 vendor 快照提交，并在 `Docs/third_party.md` 记录：

- 官方仓库地址。
- Tag。
- Commit SHA。
- 下载日期。
- 本地修改清单。
- 许可证文件位置。

禁止直接修改第三方源码解决应用问题；优先通过配置、port 层或适配器处理。确需修改时单独提交并记录原因。

## 11. 测试矩阵规范

测试项统一编号：

| 前缀 | 类别 | 示例 |
|---|---|---|
| `BLD` | 构建 | BLD-001 Debug 从零构建 |
| `BOOT` | 启动 | BOOT-002 Bootloader 连续跳转 |
| `RTOS` | FreeRTOS | RTOS-004 ISR 任务通知 |
| `LCD` | 显示 | LCD-006 DMA 连续刷新 |
| `TCH` | 触摸 | TCH-003 四角坐标 |
| `SNS` | 传感器 | SNS-007 I2C 无 ACK 超时 |
| `UI` | 页面 | UI-012 页面反复进入退出 |
| `GAME` | 游戏 | GAME-002 2048 重新开始 |
| `COM` | 通信 | COM-004 UART DMA 连续帧 |
| `PWR` | 功耗 | PWR-006 RTC 唤醒 |
| `MEM` | 内存 | MEM-003 任务栈高水位 |
| `REL` | 发布 | REL-002 Release 散列值 |

每条测试记录包含：

```text
测试编号：
固件 Commit/Tag：
构建类型：Debug / Release
前置条件：
操作步骤：
期望结果：
实际结果：
证据：串口日志 / 截图 / 波形 / 视频 / GDB
状态：PASS / FAIL / BLOCKED_HW
备注：
```

## 12. 每日协作与监督机制

### 12.1 每日开始时

提交以下信息：

```text
当前计划日：Day N
当前分支：
昨日最后 Commit：
昨日未通过项：
今天可投入时间：
希望重点理解的问题：
```

协助内容：

- 检查当天目标是否依赖未通过项目。
- 把当天任务拆成 30～90 分钟的小步骤。
- 提前指出可能的寄存器、RTOS、CMake 或内存风险。
- 给出调试观察点和验收命令。

### 12.2 遇到问题时

一次尽量提交完整证据：

```text
预期行为：
实际行为：
最小复现步骤：
Build 配置和 Commit：
完整错误信息或串口日志：
相关源码路径：
已经尝试过什么：
硬件波形或寄存器状态：
```

协助过程按以下顺序进行：

1. 复现或静态检查。
2. 缩小到构建、启动、外设、RTOS、驱动、LVGL或业务层。
3. 解释根因。
4. 提供最小修复。
5. 补充回归测试。
6. 检查是否可以提交。

### 12.3 每日结束时

提交日报：

```text
日期 / Day：
投入时间：
完成内容：
未完成内容：
今日 Commit：
构建结果：
通过的测试编号：
失败的测试编号：
Flash / RAM：
最小剩余 Heap：
相关任务栈高水位：
硬件证据：
当前阻塞：
明日计划：
```

协助输出：

- 判断当天 Gate 为 PASS、CONDITIONAL PASS 或 FAIL。
- 审查当天 diff、构建配置和关键代码。
- 指出需要补充的测试和文档。
- 给出是否合并 `main`、是否创建 Tag 的建议。
- 根据进度调整下一天任务，但不静默降低最终验收标准。

### 12.4 协作边界

- 可以持续阅读和修改工作区代码、检查 CMake、审查 Git diff、运行本机构建和静态测试。
- 需要真实硬件动作时，由开发者完成烧录、按键、触摸、接线和波形采集，再提供结果。
- 默认通过同一项目会话每天继续；开发者发来日报或“开始 Day N”后进入当天流程。
- 长周期工作可以保留明确目标和验收循环，但硬件项目仍需要开发者按日提供物理设备结果。

## 13. 每阶段门禁

| Gate | 时间 | 必须满足 |
|---|---|---|
| G0 范围冻结 | Day 1 | 行为基线、模拟功能和工具链明确 |
| G1 构建启动 | Day 3 | CMake、GNU、链接地址、Bootloader、调试全部通过 |
| G2 裸机外设 | Day 4 | 核心 HAL 外设独立通过 |
| G3 RTOS | Day 6 | FreeRTOS 原生移植与 IPC 验证通过 |
| G4 图形输入 | Day 9 | LCD DMA、LVGL、触摸、基础页面通过 |
| G5 全硬件 | Day 11 | 传感器、存储、蓝牙和电源外设通过 |
| G6 功能等价 | Day 13 | 全页面、游戏、低功耗和业务流程通过 |
| G7 优化 | Day 14 | 优化有数据且无功能回退 |
| G8 发布 | Day 15 | 全部 P0 测试和发布检查通过 |

## 14. 关键性能和资源指标

每天构建至少记录一次：

- Text、Data、BSS。
- Flash 总占用和剩余空间。
- 静态 RAM 占用。
- FreeRTOS 当前和最小剩余 Heap。
- 每个任务的栈高水位。
- LVGL 内存当前使用、峰值和碎片率。
- LCD 单次 Flush 像素数和耗时。
- SPI DMA `HAL_BUSY` 次数。
- I2C NACK、Timeout 和 Recovery 次数。
- UART 丢帧、溢出和解析失败次数。
- 系统 Reset Cause。

目标预算：

- 应用 Flash 绝不能超过 464 KiB，期望控制在约 440 KiB 内。
- SRAM 峰值期望控制在约 110 KiB 内，并为中断嵌套和异常留余量。
- 正常场景中所有任务栈保留至少 25% 余量。
- 正常运行不允许出现未解释的 DMA Busy、I2C Timeout 或消息丢失。

## 15. 风险与应对

| 风险 | 影响 | 应对 |
|---|---|---|
| BIN 与源码版本不一致 | 无法机械获得完全相同表现 | 旧 BIN 做黑盒基线，逐项记录差异 |
| Bootloader 跳转配置错误 | 应用不启动或中断跑飞 | Day 3 单独验证 VTOR、MSP、PC 和链接地址 |
| GNU 与 ARMCC 行为差异 | 编译失败或运行结果变化 | 打开告警，处理隐式声明、未初始化和 ABI 差异 |
| 混合源码编码 | 中文乱码或 GCC 解析错误 | 逐文件识别并统一为 UTF-8，单独提交 |
| Flash 余量不足 | 最终无法链接 | `-Os`、GC Sections、裁剪 LVGL 和字体，逐日记录 |
| SRAM 不足 | 随机崩溃、任务创建失败 | MAP、Heap、栈高水位和 LVGL 内存联合预算 |
| LVGL 多线程访问 | 随机崩溃或对象损坏 | 唯一 GUI 任务和消息队列 |
| SPI DMA 重入 | 刷屏死锁 | 单传输状态机、检查返回值、完成回调闭环 |
| 软件 I2C 共享冲突 | 传感器随机失败 | 总线互斥、超时、恢复和错误计数 |
| 低功耗时钟恢复错误 | 唤醒后外设失效 | 显式电源状态机和逐唤醒源测试 |
| 看门狗被盲目喂养 | 系统死锁未被发现 | 关键任务健康投票 |
| 旧算法库不兼容 GNU | 链接失败 | 本周期不依赖未使用库；真实算法另立任务 |
| 日程过紧 | 集成阶段拥堵 | 每日 Gate、小步提交、模拟功能范围不扩张 |

## 16. 官方参考

- FreeRTOS Kernel releases：<https://github.com/FreeRTOS/FreeRTOS-Kernel/releases>
- FreeRTOS Kernel repository：<https://github.com/FreeRTOS/FreeRTOS-Kernel>
- FreeRTOS Kernel Book：<https://github.com/FreeRTOS/FreeRTOS-Kernel-Book>
- LVGL v8.4 documentation：<https://docs.lvgl.io/8.4/>
- LVGL releases：<https://github.com/lvgl/lvgl/releases>
- LVGL display port：<https://docs.lvgl.io/8.4/porting/display.html>
- LVGL OS and interrupt rules：<https://docs.lvgl.io/8.4/porting/os.html>

## 17. 最终交付清单

- [ ] `my_project` 完整 Git 仓库。
- [ ] CMake Toolchain 和 Presets。
- [ ] `0x0800C000` GNU 链接脚本。
- [ ] 固定版本的 FreeRTOS V11.3.0。
- [ ] 固定版本的 LVGL v8.4.0。
- [ ] 全部 HAL、BSP、应用、UI 和游戏源码。
- [ ] Debug/Release ELF、HEX、BIN、MAP。
- [ ] 最终固件 SHA-256。
- [ ] 黑盒行为基线。
- [ ] 硬件引脚和中断表。
- [ ] FreeRTOS 手动移植说明。
- [ ] LVGL 手动移植说明。
- [ ] 任务、队列、通知和事件关系说明。
- [ ] 测试矩阵及证据。
- [ ] 优化前后量化对比。
- [ ] 已知限制：SPO2 模拟、NFC 无硬件。
- [ ] `v1.2.0-reproduction-complete` Tag。
