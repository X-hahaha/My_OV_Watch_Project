# EM7028 旧算法库检查

日期：2026-09-05。工具：Arm GNU Toolchain 15.2.Rel1 的 `arm-none-eabi-objdump -f`。参考根目录 `../../gpt/OV_Watch`。

| 文件 | 大小（bytes） | objdump 结果 | 本周期处理 |
|---|---:|---|---|
| BSP/EM7028/HeartRatelib.lib | 9682 | 退出码 0；成员 smallram.o 为 elf32-littlearm、armv6s-m | 格式可读取；未验证 GNU 链接、ABI 或运行兼容性 |
| BSP/EM7028/libBp.lib | 34738 | 退出码 0；成员 blood.o 为 elf32-littlearm、armv6s-m | 格式可读取；未验证 GNU 链接、ABI 或运行兼容性 |
| BSP/EM7028/libBpm.lib | 12214 | 退出码 1；libbpm_jx_06.o: file format not recognized | 当前 GNU objdump 无法识别，暂不集成 |

散列值：

```text
HeartRatelib.lib  06AB0186408D5661134132DC6DA8391058BBC77FE62A08907DFD1B179B33BC92
libBp.lib        82E722FD9EE6E05183250EBAF1C6881C27F33EBEA6910BC1650181E8ED99A576
libBpm.lib       056A0487D647147CF87CE01F0E23976D30271DAF89D1B8E010CD6A8E56689421
```

检查 `MDK-ARM/OV_Watch.uvprojx` 未找到这三个 `.lib` 的引用。此次没有进行链接实验，也不将“能被 objdump 读取”等同于“能替换进 Cortex-M4F hard-float 工程”。

本周期先按原有可见心率路径和 SPO2 模拟页面复现。若以后确需引入库，应单独核对 ELF attributes、浮点参数 ABI、未定义符号、编译器运行库和许可证；当前检查不阻塞 HAL / FreeRTOS 主线。

复核示例：

```powershell
arm-none-eabi-objdump -f ../gpt/OV_Watch/BSP/EM7028/HeartRatelib.lib
arm-none-eabi-objdump -f ../gpt/OV_Watch/BSP/EM7028/libBp.lib
arm-none-eabi-objdump -f ../gpt/OV_Watch/BSP/EM7028/libBpm.lib
```
