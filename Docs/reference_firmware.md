# 参考固件标识

记录日期：2026-09-05。来源目录：`../../gpt/OV_Watch/MDK-ARM/output`。该参考目录没有可读取的 Git 仓库，源码 commit 未知。

本表固定本次读取到的文件；手表当前运行的版本是否与其一致，尚待用户确认。没有执行烧录或擦除。

| 文件 | 大小（bytes） | SHA-256 |
|---|---:|---|
| OV_Watch.bin | 398168 | `33CFEA0A6B3C25FB309744532C0F87C03E9966D7EBC32DD92C71213596B470AA` |
| OV_Watch.hex | 1095112 | `8B131B6A27D6DA641FD20743C770A1E8EB2EEA41E258A8E9B8625304F9A8FDA5` |

BIN 大小约 388.84 KiB，是固件文件长度；HEX 大小包含文本编码开销，两者都不代替新工程的 MAP 内存统计。

## 启动布局的静态证据

- Keil 工程 `MDK-ARM/OV_Watch.uvprojx` 的应用区域为 `0x0800C000` / `0x74000`。
- HEX 首条扩展线性地址记录指向 `0x08000000`，首条数据记录偏移 `0xC000`，即首个数据地址 `0x0800C000`。
- BIN 首两个 little-endian 32 bit 字：MSP=`0x20019870`，Reset 向量=`0x0800C271`。MSP 在规划 SRAM 范围内；Reset 地址最低位为 1，去除 Thumb 标记后的地址为 `0x0800C270`。
- 以上支持“应用文件”这一判断，不能证明 Bootloader 实际跳转通过。还未做全 HEX/BIN 内容一致性比较。

应用 Flash 为 464 KiB，当前 BIN 长度未超过此容量。固件是否与当前硬件匹配、Bootloader 是否存在，仍需上板确认；烧录时不能把这个应用 BIN 当作从 `0x08000000` 开始的完整镜像。

## 复核命令

在仓库根目录执行：

```powershell
Get-Item ../gpt/OV_Watch/MDK-ARM/output/OV_Watch.bin, ../gpt/OV_Watch/MDK-ARM/output/OV_Watch.hex | Select-Object Name, Length
Get-FileHash ../gpt/OV_Watch/MDK-ARM/output/OV_Watch.bin, ../gpt/OV_Watch/MDK-ARM/output/OV_Watch.hex -Algorithm SHA256
```

只在确认了文件来源与硬件版本后，将测试记录的“固件身份”填成上述 SHA-256。无法确认时写“当前板上旧固件，来源未核实”。
