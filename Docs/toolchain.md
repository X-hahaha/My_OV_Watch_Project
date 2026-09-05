# 工具链安装与验证

核验日期：2026-09-05，Windows x64。用户确认原先未安装 CMake、Ninja 和调试服务器，并授权协助安装。现在三个工具已解压到本项目 `.tools`，下载 ZIP 的 SHA-256 均与发布资产的 digest 一致。

## 已核实版本

| 工具 | 实际版本 | 位置 / 状态 |
|---|---|---|
| Arm GNU GCC | Arm GNU Toolchain 15.2.Rel1；GCC 15.2.1 20251203 | 原有 `D:/arm-gnu-toolchain/bin`，PASS |
| Arm GNU GDB | 16.3.90.20250906-git，同属 15.2.Rel1 | 同上，PASS |
| GNU objdump | 2.45.1.20251203 | 同上，PASS |
| CMake | 4.4.3 | `.tools/cmake-4.4.3-windows-x86_64/bin`，PASS |
| Ninja | 1.13.2 | `.tools/ninja-1.13.2`，PASS |
| xPack OpenOCD | 包版本 0.12.0-7 | `.tools/xpack-openocd-0.12.0-7/bin`，PASS |
| Git | 2.54.0.windows.1 | 原有 `D:/Git/cmd`，PASS |
| STM32CubeMX | 安装元数据 `APP_VER=6.15.0`、revision `6.15.0-RC4` | `D:/STM32cubeMX/STM32CubeMX.exe` 已找到；GUI 启动待 Day 2 验证 |
| VSCode 插件 | C/C++ 1.34.3、CMake Tools 1.23.52、Cortex-Debug 1.12.1 | 用户扩展目录均已找到，PASS |

OpenOCD `--version` 原文为 `xPack Open On-Chip Debugger 0.12.0+dev-02228-ge5888bda3-dirty (2025-10-04-22:44)`。这是已下载包自身的版本字符串，不将它改写为 upstream 的纯 0.12.0 release。

STM32CubeMX 的 Windows 文件版本显示为 `>6.15.0-RC4`，安装器元数据同时给出 `APP_VER=6.15.0` 与 revision `6.15.0-RC4`，因此两项都保留。这里只证明安装文件可读取；创建和生成工程时再记录 GUI About 信息与实际生成结果。

本项目选择 OpenOCD 调试，因此核验脚本中 `ST-LINK_gdbserver: MISSING` 不影响总体 PASS；两者只需一项可用。USB 驱动、下载器识别和板上 GDB 连接未验证。

## 每次打开 VSCode 终端

先打开 `my_project` 文件夹，再在项目根目录的 PowerShell 终端执行：

```powershell
. ./tools/enter-env.ps1
./tools/check-toolchain.ps1
```

第一行前面的点和空格要保留。脚本将 `.tools` 中的三个工具目录加入当前终端 PATH，关闭终端后失效；没有修改用户或系统 PATH。

如果 GCC 不在另一台电脑的 PATH 中，指定它的真实安装路径，例如：

```powershell
. ./tools/enter-env.ps1 -ArmGnuBin 'D:/arm-gnu-toolchain/bin'
```

核验脚本最后应输出 `Toolchain executables PASS`，退出码为 0。它只启动各工具的版本查询，不连接或操作手表。再分别运行以下命令，可以理解各工具的职责：

```powershell
arm-none-eabi-gcc --version
arm-none-eabi-gdb --version
cmake --version
ninja --version
openocd --version
```

更准确地说，`arm-none-eabi-gcc` 是编译驱动：它可协调预处理、C 编译、汇编和链接。`gcc -S` 会保留汇编文件，`gcc -c` 通常直接得到目标文件而不保留中间 `.s`；链接阶段通常先生成带符号和段信息的 ELF，再由 `arm-none-eabi-objcopy` 转换成 HEX/BIN。CMake 根据 `CMakeLists.txt` 和 toolchain 文件生成 Ninja 构建规则，Ninja 按依赖关系调用 GNU 工具链。调试时 GDB 通常通过 TCP 上的 GDB Remote Serial Protocol 连接 OpenOCD；OpenOCD 控制 ST-Link 探针，探针在物理接口上执行 SWD 访问，并协助完成复位、寄存器/内存访问、断点和烧录。

若终端明确报“禁止运行脚本”，只对当前终端使用 `Set-ExecutionPolicy -Scope Process Bypass`，然后重新执行前两条命令；不要因此修改整机策略。

## 安装方式与固定来源

重建环境时，先保持 Arm GNU Toolchain 15.2.Rel1 可用，然后执行：

```powershell
./tools/install-toolchain.ps1
. ./tools/enter-env.ps1
./tools/check-toolchain.ps1
```

安装脚本读取 `tools/toolchain-lock.json`，下载固定版本并检查 SHA-256，成功后解压。它不执行网络上的脚本，也不修改注册表。下载缓存位于 `.tools/downloads`，整个 `.tools` 被 Git 忽略。

| 包 | 发布者页面 | ZIP SHA-256 |
|---|---|---|
| CMake 4.4.3 | [Kitware/CMake](https://github.com/Kitware/CMake/releases/tag/v4.4.3)，[CMake 下载页](https://cmake.org/download/) | `4d52ebab7193a698651639ed80d8d04fd903358843572cf44c7fd234cb7c26ab` |
| Ninja 1.13.2 | [ninja-build/ninja](https://github.com/ninja-build/ninja/releases/tag/v1.13.2) | `07fc8261b42b20e71d1720b39068c2e14ffcee6396b76fb7a795fb460b78dc65` |
| xPack OpenOCD 0.12.0-7 | [xpack-dev-tools/openocd-xpack](https://github.com/xpack-dev-tools/openocd-xpack/releases/tag/v0.12.0-7) | `6bfd3c97135aafef8affc9af1acf34fd0e2b9ca26044506f6abd7f95b7630052` |

OpenOCD 采用 xPack 提供的 Windows 二进制分发。检查时 GitHub latest API 返回 0.12.0-7，本次固定此包；固定版本不等于承诺它始终是最新版本。下载包的精确 URL、解压位置和散列值以 lock 文件为准，版本升级需要同步修改并重新验证。

## 本次验证结果

1. 安装后的 `check-toolchain.ps1` 在 PowerShell 7 和 Windows PowerShell 5.1 中均退出码 0：必需工具可启动，GCC 匹配 15.2.Rel1，OpenOCD 可启动。用户随后在自己的终端复核通过，原始输出保存在本机且默认不纳入 Git 的 `Docs/evidence/day01/terminal-toolchain-check-001.log`。
2. 在被忽略的 `build/day1-smoke-src` 中创建临时 CMake OBJECT library；以 Ninja 生成器和 GNU ARM toolchain 编译含整数与浮点函数的 C 文件，成功生成 `probe.c.obj`。
3. 编译参数包含 `-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -Wall -Wextra -Werror -Og -g3`。
4. `arm-none-eabi-readelf -h -A` 确认 ELF32、little endian、ARM、v7E-M、Thumb-2、VFPv4-D16、VFP register arguments。
5. 已找到 OpenOCD 的 `openocd/scripts/interface/stlink.cfg` 与 `openocd/scripts/target/stm32f4x.cfg`。

首次临时 CMake 调用因 PowerShell 对未引用的 `-D...=arm-gcc.cmake` 参数解析失败；将整个 `-D` 参数用引号包住并使用明确路径后，重新配置及编译成功。

这次验证证明构建工具能够协同生成 ARM 目标文件。尚未链接 MCU ELF、生成新 HEX/BIN/MAP、连接下载器或验证 Bootloader。正式构建入口仍按 Day 3 建立。
