# LegionECMode.kext

适用于 Lenovo Legion（DSDT 中包含 `_SB.GZFD.WMAA` 接口）的轻量级 macOS 内核扩展，在内核态读写主板 EC 散热/功耗模式（安静 / 均衡 / 狂暴），免去用户态 `sudo` 权限与密码弹窗。

## 工作原理

- 启动时通过 `IORegistryEntry::fromPath("IOACPIPlane:/_SB/GZFD")` 绑定主板 ACPI WMI 设备。
- 注册 sysctl OID `debug.legionec.mode`（开启 `CTLFLAG_ANYBODY | CTLFLAG_RW` 并仅放行 `1`/`2`/`3` 白名单值）：
  - 读取：调用 `_SB.GZFD.WMAA(0, 0x2D, 0)` 返回当前模式（`1` = 安静、`2` = 均衡、`3` = 狂暴、`0xFF` = 自定义）。
  - 写入：调用 `_SB.GZFD.WMAA(0, 0x2C, mode)` 切换模式，EC 会同步更新风扇策略、功耗墙和电源指示灯颜色。

## 编译与安装

```bash
./build.sh
```

将生成的 `build/LegionECMode.kext`（或 `prebuilt/LegionECMode.kext`）放入 `EFI/OC/Kexts/`，并在 OpenCore `config.plist -> Kernel -> Add` 中启用，重启后可用终端验证：

```bash
sysctl debug.legionec.mode
sysctl -w debug.legionec.mode=2
```
