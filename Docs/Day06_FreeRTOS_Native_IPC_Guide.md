# Day 6：FreeRTOS 原生 IPC 与应用骨架指导手册

编写日期：2026-09-14。执行日期由开发者实际安排。适用仓库：`D:\embed_project\01_watch\software\my_project`。

起点：Day 5 本地 Tag `v0.5.0-freertos-kernel`，提交 `0f6e548`。FreeRTOS-Kernel V11.3.0、100 MHz、HAL TIM10/RTOS SysTick 各 1 kHz、应用地址 `0x0800C000`。本手册提供步骤和关键代码片段，由你实现、构建和实验，再交给助手审核；本手册不代表 Day 6 已完成。

## 1. 今天完成到什么程度

今天实现五类 IPC 小实验：任务通知、Stream Buffer、Queue、Mutex、Event Group，再加一个软件定时器，最后放进统一应用入口。先做小实验，逐个通过后整合，暂不搬旧工程全部任务，不接入 LVGL。

| 工作包 | 使用的对象 | 真实/模拟范围 | 完成证据 |
|---|---|---|---|
| 按键 EXTI → 按键任务 | Task Notification | 实板 KEY1/PA5 | 按键计数、去抖结果、IRQ 优先级 |
| UART DMA+IDLE → 解析任务 | Stream Buffer | 实板 USART1/115200/8N1 | 短包、跨缓冲区、连续流、异常计数 |
| 数据生产 → 消费 | Queue | 今天用带序号的模拟传感器样本 | 顺序、队列满、超时恢复 |
| 两个任务访问共享总线 | Mutex | 今天用模拟 I2C 临界区 | 无并发进入、超时、锁释放 |
| 初始化/唤醒/健康状态 | Event Group | 初始化/按键真实；充电软件注入 | 位变化、等待超时、事件消费 |
| 低频事件 | Software Timer | 1 s 周期的软件事件 | 回调计数、任务响应、无阻塞 |

仍只用 ST-Link 供电。PA2 的电气有效性和充电三状态不在今天验收范围；`CHARGE` 位必须标明软件注入。模拟队列和互斥锁通过，只证明 IPC 用法，不代表传感器驱动或真实软件 I2C 总线通过。ADC、SPI DMA、电源/低功耗及启动调试尾项继续按测试矩阵补齐。

建议顺序：基线复查 → 按键通知 → UART → Queue → Mutex → Event Group/Timer → 整体验收。每个实验先保存一份日志再进入下一个。

## 2. 开始前复查 Day 5 收尾版

在仓库根目录检查：

```powershell
git status --short
git log -3 --oneline
git show --no-patch v0.5.0-freertos-kernel
git -C project/my_project/ThirdParty/FreeRTOS-Kernel describe --tags --exact-match
. ./tools/enter-env.ps1
Set-Location project/my_project
cmake --preset Debug
cmake --build --preset Debug
Get-FileHash build/Debug/my_project.elf,build/Debug/my_project.bin -Algorithm SHA256
```

烧录时继续使用现有应用区与 Bootloader 方案，不擦除 Bootloader；BIN 的应用装载地址是 `0x0800C000`，HEX 自带地址。现有 APP FLAG 位于 `0x08008000`，不是新应用起始地址。

实板运行后记录：

1. 两个任务计数、Idle/Tick 计数持续增长，failure=NONE。
2. 两个栈余量、最小剩余 Heap；短时间应接近 Day 5 记录，不能只看某次刚创建时的数值。
3. 按 KEY1 后不再触发 port.c 优先级 Assert。收尾版已移除故障测试回调，此时不要求按键产生业务效果。
4. 保存 `Docs/evidence/day06/00-day05-baseline.txt`，包含提交和 ELF/BIN SHA-256。

已有启动交接限制仍存在：Bootloader 关闭全局中断，应用在全部 MX 初始化后恢复；正常启动已验证，但 LSE 失败的 HAL 超时路径未验证。本日不要为了 IPC 实验随意提前打开所有中断。

## 3. 目录和创建顺序

沿用你实际的目录大小写，在 `project/my_project` 下按需要新建：

```text
APP/
├─ CMakeLists.txt
├─ demo/day05_rtos_demo.c,h      # 保留 Day 5 示例及现有 Hook
├─ app_entry.c,h                # App_Start，统一创建与调度入口
├─ app_events.h                 # 位定义，不放对象定义
├─ app_diag.c,h                 # Day 6 计数/错误记录
├─ tasks/
│  ├─ key_task.c,h
│  ├─ uart_rx_task.c,h
│  ├─ sample_task.c,h
│  └─ status_task.c,h
└─ services/
   └─ bus_guard.c,h             # 今天只做模拟总线互斥实验
```

这是建议拆分，允许先用一个 `day06_ipc_demo.c` 完成实验，再按职责整理。不要把第三方源码搬回 CubeMX 保留的 `Middlewares/Utilities`。更新 `APP/CMakeLists.txt`，把新增 `.c` 加入原有可执行目标，并加入对应头文件目录；不重复创建/链接第二套 FreeRTOS Kernel。

使用原生 API 的 `.c` 先包含 `FreeRTOS.h`，再包含小写 `task.h`、`queue.h`、`semphr.h`、`stream_buffer.h`、`event_groups.h`、`timers.h` 中需要的头文件。不要使用 `cmsis_os.h/cmsis_os2.h`。

### 3.1 统一入口

最终由 `main.c` 的 USER CODE 区调用一次 `App_Start()`，替换 `Day05Rtos_Start()`；全工程只启动一次调度器。暂时仍编译 Day 5 demo.c 以提供已有 Idle/Tick/Stack/Malloc Hook 和 configASSERT 诊断，**不要同时启动 Day 5 两任务**，也不要复制出第二份 Hook。

`App_Start()` 按下列顺序实现：

1. 关闭即将使用的 KEY1 事件源 IRQ，应用就绪标志保持 0；不要关闭 TIM10。
2. 创建所有 IPC 对象并检查句柄：Stream Buffer、样本 Queue、总线 Mutex、Event Group、Timer。
3. 创建全部任务并检查每次 `xTaskCreate()` 返回值；对象创建失败记录明确错误，不继续启动。
4. 启动调度器；如果返回，记录 `SCHEDULER_RETURNED` 并停在错误处理。
5. 第一阶段 InitTask 在调度器运行后完成就绪交接：清理 KEY1 EXTI/NVIC 挂起、设置 ISR 所需对象就绪标志，再使能 KEY1 IRQ、设置 `INIT_DONE`。其余任务等待 INIT_DONE 后开始工作。
6. UART 任务在调度器已运行、Stream Buffer 已创建后启动 RX DMA，检查 HAL 返回值。InitTask 随后删除自己或转入明确的永久阻塞状态。

IRQ 可能在任务句柄生成前或调度器启动前触发。回调必须在应用未就绪时直接返回，让 HAL 清理标志；不能只检查句柄非空就调用 FromISR。就绪标志应在对象和句柄全部稳定后、调度器运行期间设置。不要用 `vTaskDelay()` 去猜测对象是否准备好。

### 3.2 任务与栈的起始预算

| 任务 | 建议优先级 | 起始栈深度（words） | 阻塞点 |
|---|---:|---:|---|
| InitTask | 4 | 256 | 初始化结束后删除 |
| KeyTask | 3 | 256 | 通知等待、去抖延时 |
| UartRxTask | 4 | 512 | Stream Buffer 接收，100 ms 超时 |
| SampleProducer | 2 | 256 | 20 ms 周期延时、队列发送 |
| SampleConsumer | 2 | 256 | 队列接收，100 ms 超时 |
| BusTestTask | 1 | 256 | 获取 Mutex，有限超时 |
| StatusTask | 1 | 384 | 事件等待，有限超时 |

这只是初值，依据 High Water Mark 和最小剩余 Heap调整。Cortex-M4F 下一个 `StackType_t` 为 4 B，`xTaskCreate(..., 256, ...)` 是 1024 B 栈，不是 256 B；旧 CMSIS stack_size 字节数换成 words 时向上取整：`(bytes + sizeof(StackType_t) - 1) / sizeof(StackType_t)`。

继续使用 24 KiB heap_4、溢出检查、Malloc Hook、Assert；V11.3.0 的 `configKERNEL_PROVIDED_STATIC_MEMORY=1` 继续保留。Idle 和 Timer 静态栈也消耗 RAM；启动前后 Heap 会因动态应用对象/任务变化，初始化结束后应稳定。不要按某次余栈很大就立刻把任务栈减到极限。

### 3.3 本日中断规则

- HAL_Init 的 NVIC 分组应仍为 `NVIC_PRIORITYGROUP_4`，四位全用于抢占优先级。
- 调用 FromISR 的 IRQ 使用库优先级 5～15；数字 0～4 更紧急，不能调用 FreeRTOS API。HAL `SetPriority()` 传未移位的 5，不传 0x50。
- KEY1 EXTI9_5、USART1、DMA2_Stream5 当前均为 5；UART 与 RX DMA 必须保持相同抢占优先级，序列化同一 Stream Buffer 的写入和 DMA 位置更新。
- ISR 中先 `BaseType_t higher = pdFALSE`，调用适合的 FromISR API并处理返回值，末尾统一 `portYIELD_FROM_ISR(higher)`。只有使用 RTOS API 才需要此交接；不是每个纯 HAL IRQ 都必须人为 yield。
- ISR 不打印、不延时、不取 Mutex、不解析完整协议、不访问 LVGL。软件定时器回调运行在 Timer 服务任务中，使用普通非阻塞 API，不使用 FromISR。

## 4. 实验一：KEY1 外部中断唤醒任务

当前 KEY1 为 PA5、EXTI9_5，WAKE 为 PA4、EXTI4，两者不要混用。先只处理 KEY1；实际按下电平、触发边沿以原理图和 GPIO 实测为准，不因名字假定高/低有效。

采用**位通知**，不是计数通知：

```c
#define KEY_EVENT_EDGE (1UL << 0)

/* HAL_GPIO_EXTI_Callback 或其应用层转发中，示意代码 */
if (GPIO_Pin == KEY1_Pin && app_isr_ready != 0U) {
    BaseType_t higher = pdFALSE;
    BaseType_t rc = xTaskNotifyFromISR(key_task_handle,
                                     KEY_EVENT_EDGE,
                                     eSetBits, &higher);
    /* rc != pdPASS 时记录错误；不能在 ISR 里打印 */
    portYIELD_FROM_ISR(higher);
}
```

任务侧使用匹配的等待 API：

```c
uint32_t events;
if (xTaskNotifyWait(0U, UINT32_MAX, &events, portMAX_DELAY) == pdTRUE) {
    if ((events & KEY_EVENT_EDGE) != 0U) {
        vTaskDelay(pdMS_TO_TICKS(20U));
        /* 读 KEY1 电平并按自己的按下/释放状态机判定有效事件 */
    }
}
```

永久等待在此是明确设计：没有按键时任务不工作。去抖放在任务里；按住时只记一次按下，释放后允许下一次按下。若当前仅配置单边沿，可在任务中间隔读取直到释放，避免误把通知位合并当作丢键。

`eSetBits` 会合并重复的同一位，适合“发生过边沿”，不保证每个抖动边沿都被计数。不要配 `ulTaskNotifyTake()`；若专门学习计数通知，改为 `vTaskNotifyGiveFromISR()` + `ulTaskNotifyTake()`，并另写实验说明。

验收：启动前/启动中按键不触发空句柄 Assert；正常按 20 次，有效按下计数 20（约定的去抖间隔内过快操作另记）；长按只记一次；任务等待时 CPU 可以空闲。保存 `01-key-notify.txt`，包含 IRQ 次数、有效按下/释放次数和无故障状态。

## 5. 实验二：UART DMA+IDLE 写入 Stream Buffer

这是今天工作量最大的部分。ST-Link 调试连接不能自动当作 USART1 通道；使用已经验证的 3.3 V USB-UART 接线和公共地，板子仍由 ST-Link 供电。发送工具配置为 115200、8N1。

数据流为：USART1 RX → 循环 DMA 静态数组 → HAL RX 事件回调 → Stream Buffer → 单一解析任务。Stream Buffer 保存字节流，没有“每次写入等于一帧”的边界。

### 5.1 先修改 CubeMX 配置及旧 IRQ 代码

1. `.ioc` 中 USART1 RX 对应 DMA2_Stream5/Channel4 的 Mode 改为 Circular；TX DMA 保持原配置，不必改为 Circular。
2. USART1 与 RX DMA IRQ 均为抢占优先级 5、子优先级 0；重新生成并核对 usart.c/dma.c。
3. **移除 `USART1_IRQHandler()` USER CODE 区中提前检查/清除 IDLE 的旧代码**。保留 `HAL_UART_IRQHandler(&huart1)`，由 HAL 自己处理 IDLE并进入回调。
4. 使用 `HAL_UARTEx_ReceiveToIdle_DMA()`，不手动再次打开/清除 IDLE，不再循环调用旧 `HAL_UART_Receive_DMA()`。
5. Circular 模式保留 DMA HT、TC 中断，加上 IDLE事件。不要关闭 HT；连续流没有 IDLE时仍需要半满/全满回调搬运。

CubeMX 重生成后复查 APP/ThirdParty、CMake 胶水、VTOR 宏和 RTOS Handler 保护是否保留。`check-day02-config.ps1` 只检查原始 HAL 配置，不是 Day 6 完整验收器；出现 FAIL 要看具体字段，不为让旧检查器通过而撤销本日合理改动。

### 5.2 缓冲区与接收启动

起始参数建议：静态 DMA RX 数组 128 B，Stream Buffer 容量 512 B，触发阈值 1 B，任务侧每次取最多 64 B。不要把 DMA 数组建在会返回的函数栈上。STM32F411 无 D-cache，本日无需缓存维护，但 DMA 写入和 CPU 复制的时间约束仍然存在。

```c
static uint8_t uart_dma_rx[128];
/* App_Start 中创建并检查句柄 */
uart_stream = xStreamBufferCreate(512U, 1U);

/* UART 任务完成 INIT_DONE 等待后执行，先初始化上次位置和诊断。
   句柄与数组已就绪、调度器已运行；首次接收前没有活动 RX生产者。 */
uart_last_pos = 0U;
uart_rx_ready = 1U;
HAL_StatusTypeDef rc = HAL_UARTEx_ReceiveToIdle_DMA(&huart1,
                                                  uart_dma_rx,
                                                  sizeof(uart_dma_rx));
/* rc 必须为 HAL_OK；失败时清 uart_rx_ready并记录错误，不继续正常接收 */
```

Circular 模式只启动一次；HT、TC、IDLE 回调后接收不会停止，不在每次回调里 rearm。普通 Normal DMA 在 IDLE/TC 后重启有接收空窗；若先用它学习，只能记录有发送间隔的阶段性结果，不能据此宣称连续流收帧通过。

### 5.3 从 DMA 数组提取新数据

HAL 回调 `HAL_UARTEx_RxEventCallback(huart, Size)` 的 Size 是缓冲区内的结束位置，不是本次新收到的字节数。HT、TC、IDLE可能报告相同位置；UART 和 DMA IRQ 的到达顺序也可能使固定的 HT位置晚于某个 IDLE位置。

本手册推荐每次回调读取**实时 NDTR** 得到当前写入位置，统一做增量搬运，避免直接按固定 Size 拼接导致重复/假回绕：

```c
/* 只展示搬运核心；句柄/计数/ready 判断与头文件由你补齐 */
static uint16_t uart_last_pos;

static void UartPushFromISR(const uint8_t *data, size_t length,
                            BaseType_t *higher)
{
    if (length == 0U) return;
    size_t sent = xStreamBufferSendFromISR(uart_stream, data, length, higher);
    /* 分别累计 copied_bytes 和 dropped_bytes += length - sent */
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    (void)Size;
    if (huart->Instance != USART1 || uart_rx_ready == 0U) return;
    BaseType_t higher = pdFALSE;
    const uint16_t capacity = sizeof(uart_dma_rx);
    uint16_t pos = capacity - __HAL_DMA_GET_COUNTER(huart->hdmarx);
    if (pos == capacity) pos = 0U;

    if (pos > uart_last_pos) {
        UartPushFromISR(&uart_dma_rx[uart_last_pos],
                       pos - uart_last_pos, &higher);
    } else if (pos < uart_last_pos) {
        UartPushFromISR(&uart_dma_rx[uart_last_pos],
                       capacity - uart_last_pos, &higher);
        UartPushFromISR(&uart_dma_rx[0], pos, &higher);
    }
    uart_last_pos = pos;  /* 即使 Stream Buffer 满而丢弃也推进位置 */
    portYIELD_FROM_ISR(higher);
}
```

UART/RX DMA 同优先级，保证上述两个 ISR 不互相嵌套、位置及 Stream Buffer写入被序列化；不得再加其他任务/ISR写同一个 Stream Buffer。copied/dropped 只由这一逻辑生产者的已序列化 ISR更新；任务更新消费计数，避免多个上下文对同一 `counter++` 丢更新。

**此位置算法不能识别两次处理间 DMA 已经完整绕回一圈的情况。** 115200/8N1 下 64 B 半缓冲区约 5.56 ms，128 B约 11.11 ms；需保证 IRQ响应与复制足够及时、不会在未复制前被 DMA覆盖。本日不允许持续数毫秒以上关闭这些 IRQ/全局中断。`dropped_bytes=0` 只能说明 Stream Buffer 写入未被拒绝，还要用发送序号/字节内容验证是否漏了一圈。增大缓冲区也必须重新计算时间预算。

### 5.4 任务接收、解析和错误恢复

```c
uint8_t chunk[64];
for (;;) {
    size_t count = xStreamBufferReceive(uart_stream, chunk,
                                       sizeof(chunk), pdMS_TO_TICKS(100U));
    if (count > 0U) {
        /* 交给持久化的解析状态机，跨多次 receive 保存半帧状态 */
    }
    /* 超时允许检查错误标志、健康计数和需要执行的接收恢复 */
}
```

Stream Buffer 是单生产者/单消费者对象。本项目一个逻辑 RX 生产者由已序列化的 UART/DMA 回调构成；消费者只设一个 UART任务。不要再让 StatusTask 抢读。V11.3.0 默认借用任务通知 index 0 完成 Stream Buffer 等待；当前只有一个通知槽，所以不要再在这个消费者上混用自定义 index 0 通知等待。按键使用独立 KeyTask，UART错误先用 ISR 标志/计数加接收超时巡检。

先传固定已知字节，逐字节比较；再加简单帧：`A5 LEN SEQ PAYLOAD CHECKSUM`，约定 LEN 的意义、最大长度和校验覆盖范围。解析器必须处理半帧、多帧连在一次 receive、非法 LEN、错误校验及重新同步；UART IDLE只是传输间隔，不是协议唯一帧边界。

在 `HAL_UART_ErrorCallback()` 中记录 ORE/FE/NE 等错误及恢复请求，不打印、不卡住。由 UART任务执行恢复：停用 RX就绪标志，屏蔽 USART/RX DMA相关 IRQ，调用合适的 HAL终止接收，清理错误/挂起标志及 DMA位置，按决定丢弃的流内容重置解析器，再恢复 ReceiveToIdle 与 IRQ。检查每个 HAL返回值；期间不能有生产者操作 Stream Buffer。若调用 `xStreamBufferReset()`，必须确保没有任务正在阻塞等待该流，本方案让唯一消费者自己在超时后恢复。恢复允许丢弃当前坏帧，但应记计数并能接收后续正确帧。

### 5.5 UART 验收用例

| 用例 | 发送方式 | 预期 |
|---|---|---|
| 短包 | 1/7/31 B 后留明显空闲 | IDLE可触发，新数据仅一次，内容一致 |
| 边界 | 64/128/129/255 B，带递增序号 | HT/TC/回绕无重复、无遗漏 |
| 半帧/连帧 | 拆开一帧发送、再连续发送多帧 | 解析器跨 receive正确组帧 |
| 连续流 | >=4096 B，全速重复递增数据/帧，尽量无人工停顿 | 不依赖 IDLE也能持续收；序号/内容正确 |
| 满缓冲区 | 在任务内故意延长消费间隔，发送超过流容量 | dropped_bytes增长，系统不死锁；恢复后可收到下一段正确数据 |
| 错误帧/恢复 | 非法 LEN/校验；若实际产生 UART错误则另记 | 有错误计数，后续正确帧可解析；未触发的电气错误不写 PASS |

正常测试保存 PC发送字节数、回调次数（按 HT/TC/IDLE分类可选）、copied/consumed/dropped、SEQ错误与校验错误数、HAL启动结果。PC工具可能自动分块，日志应说明发送间隔；正常负载要求无丢弃/序号错。负向满缓冲区实验允许可解释的丢弃，不能混入正常统计。

保存 `02-uart-stream-normal.txt`、`02-uart-stream-overflow.txt`、PC发送数据及原始接收/解析记录。

## 6. 实验三：模拟传感器 Queue

队列项按值复制，先不要传递指向生产者局部变量的指针：

```c
typedef struct {
    uint32_t seq;
    TickType_t produced_tick;
    int32_t value_milli;     /* 今天填可预测的模拟值 */
} Sample;

sample_queue = xQueueCreate(8U, sizeof(Sample));
```

生产者每 20 ms 用 `vTaskDelayUntil()` 发送一份样本，`xQueueSend(..., pdMS_TO_TICKS(50U))`，发送失败计数。消费者 `xQueueReceive(..., pdMS_TO_TICKS(100U))`，核对序号和数值，记录消费数及等待超时。每个样本的 seq 在尝试发送时增长，则队列满丢弃会体现在序号缺口；明确记录这条语义。

正常运行 60 s，队列不持续积压，发送/接收数量匹配（停下时把在队列里的项计入），顺序正确。负向测试让消费者故意每 500 ms处理一次，观察队列满及发送超时，随后恢复，系统能继续消费；不能通过无限阻塞掩盖队列满。

`xQueueOverwrite()` 只适合长度为 **1** 的“最新值邮箱”，不能用于这个 8项 FIFO。若另建最新值实验，说明旧样本被替换是设计行为，不能要求逐项无丢失。

保存 `03-queue.txt`，记录样本为模拟、队列长度、生产周期、超时、正常/负向计数。

## 7. 实验四：软件 I2C 总线 Mutex

今天建立 `xSemaphoreCreateMutex()` 的总线保护层，先用模拟临界区验证，不同时引入真实传感器驱动。两个任务必须经同一个接口、同一个 Mutex进入共享总线；只在其中一个任务加锁没有保护作用。

```c
if (xSemaphoreTake(bus_mutex, pdMS_TO_TICKS(50U)) == pdTRUE) {
    /* 一次完整模拟事务，完成/失败的所有分支最后都释放 */
    xSemaphoreGive(bus_mutex);  /* 检查结果或使用明确的断言 */
} else {
    /* 记 bus_timeout；不执行总线访问 */
}
```

在持锁区域设置 `in_bus` 并检查进入前必须为 0，退出时恢复 0；两任务分别记录进入、退出、超时和 max_in_bus。此计数由 Mutex保护，负向试验中故意去锁时计数并非可靠的原子测量，不能靠它证明所有并发情况。

一组正常实验：每次持锁短于对方 50 ms超时，max_in_bus=1、进入退出数匹配。另一组故意持锁 100 ms，对方超时、不得无锁进入，锁释放后重新获取成功。实验里的延时只用于模拟占用，不作为真实 I2C 事务实现；真实总线后续需独立的有限事务/错误恢复。

理解 Mutex 的优先级继承：低优先级任务持锁、高优先级任务等待时，低任务可能暂时继承高任务优先级，释放后恢复。它不等于“高任务永远能立刻取得锁”；要实测继承需单独安排确定的持锁/等待顺序并观察优先级，不以两任务能运行就声称测试过继承。

Mutex 只能在任务中使用，没有适用的 Mutex FromISR取锁方式。软件 Timer回调也不等待该锁。后续真实软件 I2C 的 Start/地址/读写/Stop/失败清理应整体位于一个锁边界，避免拆段后事务交错。

保存 `04-mutex.txt`，明确“模拟总线互斥验证”，包含锁超时及恢复数据。

## 8. 实验五：Event Group 与软件 Timer

### 8.1 先定义位的含义和清除者

使用低位，32位 Tick 配置下 Event Group只有低24位可用于应用，不使用内核保留高位。

```c
#define APP_EVT_INIT_DONE    (1UL << 0)
#define APP_EVT_CHARGE       (1UL << 1)
#define APP_EVT_WAKE         (1UL << 2)
#define APP_EVT_UART_HEALTH  (1UL << 3)
#define APP_EVT_SAMPLE_HEALTH (1UL << 4)
#define APP_EVT_TIMER_PULSE  (1UL << 5)
```

| 位 | 含义 | 设置/清除约定 |
|---|---|---|
| INIT_DONE | 初始化已完成的持续状态 | InitTask设置；等待者不自动清除 |
| CHARGE | 软件注入的充电状态 | 测试控制设置/清除；日志注明 SIMULATED |
| WAKE | 自上次消费起出现过一次唤醒请求 | KeyTask设置；StatusTask消费后清除 |
| UART_HEALTH/SAMPLE_HEALTH | 一个观察窗口内任务到达过健康检查点 | 任务定期设置；StatusTask检查并清除 |
| TIMER_PULSE | 出现过低频软件 Timer事件 | Timer回调设置；StatusTask消费后清除 |

健康位合并多次设置，不能计次数；若每次事件都不可丢，另用 Queue或计数通知。

各工作任务启动时等待 INIT_DONE：

```c
EventBits_t bits = xEventGroupWaitBits(app_events, APP_EVT_INIT_DONE,
                                     pdFALSE, pdTRUE, pdMS_TO_TICKS(1000U));
if ((bits & APP_EVT_INIT_DONE) == 0U) {
    /* 初始化等待超时，记录错误并进入明确处理 */
}
```

StatusTask 可以先用两个独立小测试理解 ANY/ALL以及 clear-on-exit；正常实现对持久状态和一次性事件分开等待/消费。不要把持续为 1 的 INIT_DONE或 CHARGE放进循环的 ANY等待掩码，否则等待会立即返回，形成空转。

健康实验每个窗口约1 s：两个工作任务设置各自健康位，StatusTask等待两个位全部到达并自动清除，再延时到下一观察窗口；负向测试在任务自身逻辑中停止报告其中一个位，预期 StatusTask等待超时并记录缺失位。启动时序和超时可能使窗口边界有偏差，日志必须说明；今天仅做健康状态观察，不直接复位板子。

CHARGE 位用一个明确的任务侧测试控制变量切换（例如 GDB改 `charge_sim_request`，由 StatusTask读取后设置/清除），不要从 PC给实际 PA2注入未确认电平，也不要把 GDB直接改内部 EventGroup对象当作API实验。

若额外学习 `xEventGroupSetBitsFromISR()`，要理解它**向 Timer服务任务排入命令**，不是当场完成所有等待者唤醒；检查返回值 pdPASS，队列满时记失败并 `portYIELD_FROM_ISR(higher)`。今天主路径可以由 KeyTask设置 WAKE，避免同时给 KEY1 ISR塞入两套机制。

### 8.2 一个 1 s 软件 Timer

```c
static void SlowTimerCallback(TimerHandle_t timer)
{
    (void)timer;
    /* 递增 timer_callback_count，并向 Event Group设置 TIMER_PULSE */
    xEventGroupSetBits(app_events, APP_EVT_TIMER_PULSE);
}

slow_timer = xTimerCreate("slow", pdMS_TO_TICKS(1000U), pdTRUE,
                          NULL, SlowTimerCallback);
/* 对象创建后检查非空；在 InitTask中启动并检查返回值 */
BaseType_t rc = xTimerStart(slow_timer, pdMS_TO_TICKS(100U));
```

回调是 Timer服务任务上下文，只递增自己的计数、设置位或以0等待发送一个轻量消息；不做 UART阻塞发送、I2C事务、长计算、`vTaskDelay()`，不等待其他对象。`xTimerStart()==pdPASS` 表示命令入队，不表示回调已执行。

当前 Timer任务优先级2、命令队列10、栈256 words，先保持；若高优先级任务不阻塞，Timer也会被饿死。Timer服务队列满应有明确失败计数，不能通过在回调里永久等队列解决。

验收：60 s运行内回调计数与运行Tick相符，StatusTask处理到事件，INIT_DONE始终保留；CHARGE模拟状态可切换；停止某任务健康报告可观察超时、恢复报告后可重新满足。Event Group可能合并 TIMER_PULSE，状态任务处理次数不应硬要求等于所有回调数；精确计数以独立 callback_count为准。

保存 `05-events-timer.txt`，分别标注持续状态、一次性事件、健康窗口和模拟充电。

## 9. 整合时的诊断与错误处理

建立可在 GDB观察的 `g_day06_stats`，至少包含：

- 初始化阶段/首次错误码，任务或 IPC创建失败的对象名/编号。
- 按键 IRQ数、有效按下/释放数。
- UART copied/consumed/dropped、解析帧数、SEQ/校验/HAL错误和恢复数。
- Queue尝试发送/成功发送/消费/发送超时数。
- Mutex进入/退出/等待超时/max_in_bus。
- Event Group健康超时/缺失位、软件 Timer回调数。
- 各任务最小剩余栈 words、当前/最小剩余 Heap bytes。

统计字段分别指定唯一写入上下文；`volatile` 只保证编译器可见访问，不会把读改写操作变成原子操作。需要跨上下文复位统计时先停用对应事件源/实验，或用合适的短临界区。GDB暂停所有任务后读取的快照才适合对照同一时刻的队列/计数关系。

保留 Day 5 的四个 Hook与 configASSERT。Day 6 错误码应区分 TASK_CREATE、QUEUE_CREATE、STREAM_CREATE、MUTEX_CREATE、EVENT_CREATE、TIMER_CREATE、UART_START及INIT_TIMEOUT；不能出现对象创建失败后对 NULL继续调用API。正常用户超时/队列满等运行期事件应计数并按预期处理，不一律当成致命断言。

日志先用 GDB快照，若要 UART打印，统一一个任务负责输出并限制频率；IRQ只写诊断。不要每个样本/字节都打印，也不要多个任务同时调用未经保护的 HAL TX。

## 10. 最终验收和需要提交的材料

### 10.1 构建及源码检查

在 `project/my_project` 执行 Debug/Release重新配置和全量构建，保存四份 configure/build日志：

```powershell
. ../../tools/enter-env.ps1
cmake --fresh --preset Debug
cmake --build --preset Debug --clean-first
cmake --fresh --preset Release
cmake --build --preset Release --clean-first
```

复查 APP入口只有一次 scheduler启动、任务与对象创建结果均处理、ISR API优先级合法、所有等待有明确超时/永久等待设计、HAL仍TIM10、三个 RTOS异常入口仍来自Port、向量表仍 `0x0800C000`。前面的阶段日志也记录构建配置和固件指纹。

### 10.2 通过条件

- 五类 IPC均有正常路径与相应负向/边界实验；Timer正常运行。
- 最终整合版连续运行至少5 min，任务均能响应，没有 Assert/Stack/Malloc故障，初始化完成后 Heap没有继续下降，栈余量已记录。
- 正常UART负载中字节/帧序号正确、无不可解释的重复/丢失；满缓冲区负向测试有计数和恢复。
- 没有ISR空句柄/调度器未启动时调用RTOS、没有ISR打印/延时/Mutex；UART RX ISR写入已序列化。
- Debug实板验证完整；Release至少重新烧录完成按键、UART与Timer综合冒烟，优化后功能仍正常。没有完成哪项就如实登记PENDING，不能用Debug构建结果代替Release运行。
- 未覆盖的充电、传感器/I2C真实事务、ADC/SPI等范围仍清楚标记。

### 10.3 原始证据与每日记录

原始文件保存在已忽略的 `Docs/evidence/day06/`；请不要删除Day5证据。每日记录建立 `Docs/daily/<实际日期>-day06.md`，至少写明以下字段：

```text
日期与固件提交（未提交修改要注明） / Debug或Release / ELF与BIN SHA-256
板型 / 仅ST-Link供电 / USART接线和PC发送工具参数
实验编号 / 对象长度与超时 / 操作步骤
实际计数、错误/丢弃及恢复结果 / 栈words与Heap bytes
真实/模拟/延期范围 / 证据文件名 / 尚未解决的问题
```

建议保存：00基线、01按键、02UART正常/溢出、03Queue、04Mutex、05Event/Timer、06整合5min、Release冒烟、Debug/Release构建日志、FreeRTOS版本日志。双通道抢占波形若方便补采，注明PB15/PB2接点与时间刻度；不重复进行已通过的故障实验，除非本日改动引入新问题。

### 10.4 Git收尾

完成后先自查：

```powershell
git status --short
git diff --check
git diff
git diff --cached
```

提交业务源码、`.ioc`及合理的生成代码、APP CMake和每日记录；不提交build、工具缓存、原始波形或未解释的故障注入。可以先分别提交IPC实验和应用骨架，最后交给助手审核证据与当前源码，审核通过后才创建 `v0.6.0-rtos-foundation`。**本次只创建Day5标签，不提前创建Day6标签。**

完成后发给助手：“Day06完成，日志在Docs/evidence/day06，请核验并收尾，之后给下一天手册。”

## 11. 常见问题定位

| 现象 | 优先检查 |
|---|---|
| 按键再次卡在port.c:888 | 实际gpio.c IRQ是否仍4；ioc/生成代码是否一致；是否调用合法FromISR |
| 只收到64/128 B事件，短包没有回调 | USART IRQ是否预清IDLE；是否真正用了ReceiveToIdle；USART NVIC是否启用 |
| 每到回绕位置重复一大段 | 是否误把Size当增量、是否旧HT位置晚到、是否两个ISR更新位置互相嵌套 |
| 大量连续数据丢失，dropped仍0 | DMA是否绕回覆盖、IRQ是否延迟过久；用SEQ检查，不只看Stream Buffer拒绝数 |
| UART任务通知不按预期 | 是否混用Stream Buffer默认index0与自定义通知；是否多个任务抢读 |
| Queue消费者读出被改写数据 | 是否入队的是局部变量指针；改为按值样本或明确定义所有权 |
| Mutex经常超时 | 是否有错误分支忘记Give；锁内是否长打印/阻塞；两个任务是否同一锁 |
| Event等待循环不阻塞 | 是否等待了持续为1的状态位，或未清除一次性位；ANY/ALL是否匹配 |
| 软件Timer卡住/事件延迟 | 回调是否阻塞；高任务是否空转；Timer命令队列是否满 |
| Malloc Hook启动就触发 | 对象/任务是否重复创建；是否同时启动Day5和Day6任务；记录失败对象与最小Heap |

## 12. 与旧CMSIS API的对应关系

| 旧API | 本日原生API | 需要重新决定的语义 |
|---|---|---|
| osThreadNew | xTaskCreate | 栈bytes→words、优先级、返回值与句柄 |
| osDelay | vTaskDelay(pdMS_TO_TICKS(ms)) | 时间单位；周期工作用vTaskDelayUntil |
| osMessageQueueNew | xQueueCreate | 项长度、复制值还是指针所有权 |
| osMessageQueuePut | xQueueSend / 长度1时xQueueOverwrite | 等待时间、满时丢弃策略、ISR变体 |
| osMessageQueueGet | xQueueReceive | 超时和消费者个数 |
| osEventFlagsNew/Set/Wait | xEventGroupCreate/SetBits/WaitBits | 状态/事件、ANY/ALL、自动清除、ISR延迟执行 |
| osKernelStart | vTaskStartScheduler | 只启动一次，返回代表启动失败 |

API名称对应不代表行为自动等价；今天以这五个可观测实验建立应用边界，不做全工程文本替换。

## 13. 本地源码参考

本手册按当前固定版本源码编写，遇到API疑问先查对应头文件注释：

- [task.h](../project/my_project/ThirdParty/FreeRTOS-Kernel/include/task.h)：Task Notification、栈单位、创建/延时。
- [stream_buffer.h](../project/my_project/ThirdParty/FreeRTOS-Kernel/include/stream_buffer.h)：单写单读、部分写入、通知槽。
- [queue.h](../project/my_project/ThirdParty/FreeRTOS-Kernel/include/queue.h)、[semphr.h](../project/my_project/ThirdParty/FreeRTOS-Kernel/include/semphr.h)：按值队列、Mutex及超时。
- [event_groups.h](../project/my_project/ThirdParty/FreeRTOS-Kernel/include/event_groups.h)：应用位、自动清除、FromISR向Timer任务排入命令。
- [timers.h](../project/my_project/ThirdParty/FreeRTOS-Kernel/include/timers.h)：软件Timer命令和回调上下文。
- [stm32f4xx_hal_uart.c](../project/my_project/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c)：ReceiveToIdle、HT/TC/IDLE、Circular模式回调处理。
- [Day5审核记录](daily/2026-09-14-day05.md)、[测试矩阵](test_matrix.md)：已经验证和继续延期的范围。
