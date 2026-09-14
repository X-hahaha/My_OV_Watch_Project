# 第三方源码登记

更新日期：2026-09-14。

| 库 | 版本 / 提交 | 来源 | 许可证 | 本地修改 |
|---|---|---|---|---|
| FreeRTOS-Kernel | V11.3.0 / `9b777ae5c5b8e9e456065a00294d1e5f5f9facf5` | [官方仓库](https://github.com/FreeRTOS/FreeRTOS-Kernel/tree/V11.3.0) | MIT，原始 LICENSE.md 保留 | 无；移植配置和构建胶水放在父仓库 |

FreeRTOS 使用 `project/my_project/ThirdParty/FreeRTOS-Kernel` submodule；父仓库 gitlink 固定完整提交，不随上游分支移动。首次加入检查点为父仓库 `68cc6cd`，2026-09-14 重新核验版本和内核工作区。

当前 Port 为 `portable/GCC/ARM_CM4F`，分配器为 `portable/MemMang/heap_4.c`；只编译 CMake 列出的八个文件。应用配置位于 `Config/FreeRTOSConfig.h`，不编辑内核，也不引入 CMSIS-RTOS 包装层。

可从仓库根目录交叉检查：

```powershell
git ls-tree HEAD project/my_project/ThirdParty/FreeRTOS-Kernel
git -C project/my_project/ThirdParty/FreeRTOS-Kernel rev-parse HEAD
git -C project/my_project/ThirdParty/FreeRTOS-Kernel describe --tags --exact-match
git -C project/my_project/ThirdParty/FreeRTOS-Kernel status --short
```

前两项 SHA 必须一致，精确 Tag 必须为 V11.3.0，最后一项应无输出。克隆后初始化方式见 README；若本机 git-submodule 的 shell helper 报错，先修复 Git 环境，不使用任意最新版源码代替固定提交。

LVGL 尚未加入，后续单独登记实际版本、提交及许可证。
