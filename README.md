# LegionECMode.kext

适用于 **Lenovo Legion（拯救者）macOS 黑苹果**的自研内核级 **EC 功耗模式免密切换驱动**（IOKit Kext）。

> 如果你在找 **Lenovo Legion 5 Pro（2025，Core Ultra 7 255HX + RTX 5070 Laptop）** 的完整 OpenCore EFI 配置，请移步：[KSLengYu/Legion5Pro-2025-Hackintosh](https://github.com/KSLengYu/Legion5Pro-2025-Hackintosh)。

---

## 目录结构

```text
.
├── LICENSE
├── README.md
└── LegionECMode/
    ├── LegionECMode.cpp       # IOKit 内核驱动源码
    ├── Info.plist
    ├── build.sh               # 本地编译脚本
    └── prebuilt/
        └── LegionECMode.kext  # 已编译成品（可直接放入 EFI/OC/Kexts）
```

---

## 工作原理

传统通过用户态 ACPI 工具调用联想功耗模式切换往往需要 `sudo` 提权或频繁输入密码；`LegionECMode.kext` 直接在内核层对接主板 DSDT 中的 `_SB.GZFD.WMAA` 接口：

- 启动时通过 `IORegistryEntry::fromPath("IOACPIPlane:/_SB/GZFD")` 绑定主板 ACPI WMI 设备。
- 注册内核 sysctl 接口 `debug.legionec.mode`（开启普通用户读写权限，并在内核里严格校验白名单 `1` / `2` / `3`）：
  - **读取**：调用 `_SB.GZFD.WMAA(0, 0x2D, 0)` 返回当前 EC 模式（`1` = 安静、`2` = 均衡、`3` = 狂暴、`0xFF` = 自定义）。
  - **写入**：调用 `_SB.GZFD.WMAA(0, 0x2C, mode)` 通知主板 EC 实时切换风扇策略、整机功耗墙与电源指示灯颜色。

---

## 安装与使用

1. 将 `LegionECMode/prebuilt/LegionECMode.kext`（或自行编译出的 `build/LegionECMode.kext`）拷贝到 OpenCore 的 `EFI/OC/Kexts/` 目录。
2. 在 `EFI/OC/config.plist -> Kernel -> Add` 中启用 `LegionECMode.kext`。
3. 重启进系统后，无需 `sudo`，直接在终端即可查询和切换模式：

```bash
# 查看当前 EC 模式（1=安静, 2=均衡, 3=狂暴）
sysctl debug.legionec.mode

# 免密切换 EC 模式
sysctl -w debug.legionec.mode=1   # 安静模式
sysctl -w debug.legionec.mode=2   # 均衡模式
sysctl -w debug.legionec.mode=3   # 狂暴模式
```

> **兼容性说明**：使用前请确认你的联想机型 DSDT 中存在 `_SB.GZFD.WMAA`，且支持 `WMAA(0, 0x2C, 1/2/3)` 与 `WMAA(0, 0x2D, 0)`（已在 Legion 5 Pro 2025 / Core Ultra 7 255HX 上实测通过）。

---

## 从源码编译

需要已安装 Xcode 或 Apple Command Line Tools（含 macOS SDK）：

```bash
cd LegionECMode && ./build.sh
```

也可以直接前往 [Releases](https://github.com/KSLengYu/LegionECMode-kext/releases) 下载打包好的 `LegionECMode-v1.0.zip`。

---

## License

MIT License
