# Day 5 证据指纹

登记日期：2026-09-14。原始文件位于本机 `Docs/evidence/day05/`，默认不入 Git；以下 SHA-256 用于核对本次实际读到的文件。请另行备份原始证据。

开发者采集的原始硬件日志没有 ELF/BIN 身份，收尾前参考提交为 `cdd0268`；本次生成的 closeout 日志对应收尾后的源码审核与交叉编译，助手未重新烧录硬件。散列仅证明文件内容，不能补足未记录的采集身份。

| 文件 | 字节数 | SHA-256 |
|---|---:|---|
| `debug-build-closeout.txt` | 5755 | `374bd007ef00f60fbd87f316a69bdcb21eea77ee992f6802a5935f8f84fc8b15` |
| `debug-clean-build.txt` | 7323 | `ee36092301bb295366afddc56f005cb44b1f8074893ad4126d79dc58a1256051` |
| `debug-configure-closeout.txt` | 886 | `749055d24c4e9aa5d531b143206f7d43e2e73752a141cec59e300971b7b7c603` |
| `elf-static-closeout.txt` | 4963 | `10a9f679f41c4f177b2d719a6d15358d4c4df7afeaed299700f4f2b6e16bcef2` |
| `freertos-version.txt` | 602 | `3e543359e3c6e3cc692067e571d66dd4f3fc1fa194243aa8845a2e5a78d8c701` |
| `g_day05_rtos_stats.txt` | 774 | `f2a426a0a160faad1b3a100483540dc14e9f29e09651bcf73445e2116642af73` |
| `GDB_log.txt` | 663 | `d793a2ca462cad00b30901bf9bb2c0f2ab0becc0191a6763100082e9967f23ad` |
| `irq-priority-assert.txt` | 471 | `3951cb73954f6ed4da9665094783375fea993fb0678c32584854b6c243a68355` |
| `malloc-failed-hook.txt` | 307 | `e58c3b11e886763a370bb3b186e29f1a93b5e74490c6b9f85aa189c3ff9959d3` |
| `release-build-closeout.txt` | 5757 | `5faaa9f87fc326012aad767e3d6114b99cf6a0b5337a670ffe37da5dac4e2442` |
| `release-clean-build.txt` | 2327 | `09e1c9a92638beb3ebd889e0d86b3476ada407d35522596a01b88deeeb3c8c1f` |
| `release-configure-closeout.txt` | 890 | `51970549b85260c215b96086dc7f6a77f2738f36dd55960d2037ba7faf1cfc06` |
| `stack-overflow-hook.txt` | 325 | `4547b49f231f0bfa63a0572f2eb798f73dcb3842a50bfd4d8ae3a816240bd001` |
| `wave-closeout.txt` | 67 | `df28071d5870cdade48f8d67eb0c0ae6bc4e1041ee74e1afa9ed7388951d34fe` |
| `wave1.csv` | 73484 | `4347650b39feb5efa43f6ee3fd5eb7385f086f87f05cca20db9e4dc9136de0e3` |
| `wave1.npy` | 8548 | `84d6dc480bd2b02dd75509073c227e21cac00875593dfcb69c8c2410c29bee81` |
| `wave1.png` | 137377 | `4b8c7063095236fd91c82b08eb6f1ef1289faf95f0c3e901cb98582818031ec6` |

## 收尾产物

由 Arm GNU Toolchain 15.2.Rel1 / CMake 4.4.3 / Ninja 1.13.2 全量生成。产物在本机 build 目录，不入 Git。

| 文件（相对 project/my_project） | 字节数 | SHA-256 |
|---|---:|---|
| `build/Debug/my_project.elf` | 1321364 | `1826d35f8bb14fa278681bd832b0e1b695da9bbc194697fa068cc888a8be52a4` |
| `build/Debug/my_project.hex` | 63157 | `cb03e47b0a7188af5383557745fedcf70e89f4afa47b0d15ae977f205e27f3b6` |
| `build/Debug/my_project.bin` | 22412 | `0970787815776b730fbf4bc4cc62a79d6aaaefb045d6ba474e773743edd988ad` |
| `build/Debug/my_project.map` | 780025 | `812595eaeadeee0a5ab3aece1f692783db60f81f021034f1c7c75f1db63e4aa3` |
| `build/Release/my_project.elf` | 45000 | `d914e23a496bd1e1e5193d330f1bc4702cf869c4156e547238960733e0a41dc5` |
| `build/Release/my_project.hex` | 55478 | `c644db50b6118d108dc421b05518d857a00027cd44c46a87d8a3a4eccd720f72` |
| `build/Release/my_project.bin` | 19684 | `28b76c6b0301616cda2d38b3e28528992b18fb2a0ca5bf1774f48cf5bdf9716b` |
| `build/Release/my_project.map` | 229849 | `aec24c087b5d4dbeae953ed45f6f6093f17cb65e052837112e7c9062054339f2` |
