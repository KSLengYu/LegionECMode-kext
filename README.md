# LegionECMode.kext

[中文](#中文) | [English](#english)

---

## 中文

给 Lenovo Legion（拯救者）macOS 用的 IOKit 驱动，在内核里直接调用 ACPI 的 `_SB.GZFD.WMAA` 读写 EC 功耗模式，不需要用户态 `sudo`。

目前在 **Lenovo Legion 5 Pro 2025（Core Ultra 7 255HX）+ macOS 26.7.2** 上测试通过。配套的 OpenCore EFI 配置在另一个仓库：[KSLengYu/Legion5Pro-2025-Hackintosh](https://github.com/KSLengYu/Legion5Pro-2025-Hackintosh)。

### 实现方式

驱动加载后查找 `IOACPIPlane:/_SB/GZFD`，并注册一个普通用户可读写（`CTLFLAG_ANYBODY`）的 sysctl 节点 `debug.legionec.mode`：

- 读取：调用 `_SB.GZFD.WMAA(0, 0x2D, 0)`，返回当前 EC 模式（`1` 安静 / `2` 均衡 / `3` 狂暴 / `0xFF` 自定义）。
- 写入：仅放行 `1` / `2` / `3`，调用 `_SB.GZFD.WMAA(0, 0x2C, mode)`，由 EC 同步切换功耗墙、风扇策略和电源键灯色。

### 安装与使用

把 `LegionECMode/prebuilt/LegionECMode.kext`（或自己编译出来的 kext）放到 `EFI/OC/Kexts/`，在 `config.plist -> Kernel -> Add` 里启用后重启。

进系统后可以直接通过 `sysctl` 读写，不用加 `sudo`：

```bash
# 查看当前模式
sysctl debug.legionec.mode

# 切换模式（1=安静, 2=均衡, 3=狂暴）
sysctl -w debug.legionec.mode=1
sysctl -w debug.legionec.mode=2
sysctl -w debug.legionec.mode=3
```

前提是机器 DSDT 里有 `_SB.GZFD.WMAA` 且支持 `0x2C` / `0x2D` 参数，其他联想机型使用前建议先核对自己的 DSDT。

### 编译

装好 Xcode 或 Command Line Tools 后直接运行构建脚本：

```bash
cd LegionECMode
./build.sh
```

源码在 `LegionECMode/LegionECMode.cpp`，编译脚本在 `LegionECMode/build.sh`。也可以直接去 [Releases](https://github.com/KSLengYu/LegionECMode-kext/releases) 下载打包好的 `LegionECMode-v1.0.zip`。

### License

MIT

---

## English

An IOKit kext for Lenovo Legion Hackintosh builds that reads and switches EC thermal/power modes via ACPI (`_SB.GZFD.WMAA`) in kernel space, without requiring `sudo` in userspace.

Tested on a **Lenovo Legion 5 Pro (2025, Core Ultra 7 255HX) running macOS 26.7.2**. The OpenCore EFI for this machine is in [KSLengYu/Legion5Pro-2025-Hackintosh](https://github.com/KSLengYu/Legion5Pro-2025-Hackintosh).

### How it works

On start, the kext looks up `IOACPIPlane:/_SB/GZFD` and registers `debug.legionec.mode` with `CTLFLAG_ANYBODY`:

- Read: calls `_SB.GZFD.WMAA(0, 0x2D, 0)` and returns the current EC mode (`1` = Quiet, `2` = Balanced, `3` = Performance, `0xFF` = Custom).
- Write: accepts only `1`, `2`, or `3` and calls `_SB.GZFD.WMAA(0, 0x2C, mode)`. The EC updates the power limits, fan curve, and power LED color accordingly.

### Install & usage

Copy `LegionECMode/prebuilt/LegionECMode.kext` (or a self-built kext) to `EFI/OC/Kexts/`, enable it under `config.plist -> Kernel -> Add`, and reboot.

Once booted, query or switch modes directly without `sudo`:

```bash
# Read current mode
sysctl debug.legionec.mode

# Switch mode (1=Quiet, 2=Balanced, 3=Performance)
sysctl -w debug.legionec.mode=1
sysctl -w debug.legionec.mode=2
sysctl -w debug.legionec.mode=3
```

Requires `_SB.GZFD.WMAA` with `0x2C` / `0x2D` support in your machine's DSDT. Check your DSDT before using on other Legion models.

### Build from source

With Xcode or Apple Command Line Tools installed:

```bash
cd LegionECMode
./build.sh
```

Source is in `LegionECMode/LegionECMode.cpp` and build script in `LegionECMode/build.sh`. Prebuilt `LegionECMode-v1.0.zip` is also available in [Releases](https://github.com/KSLengYu/LegionECMode-kext/releases).

### License

MIT
