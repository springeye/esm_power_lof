# Flash Encryption（dev-mode）加密固件运维指南

> 内部运维文档。终端用户刷机仍看 `docs/flashing-guide.md`，本文件面向产线/开发负责人。
>
> **只想照步骤操作？看 [`secure-firmware-steps.md`](secure-firmware-steps.md)（打包→刷写逐步手册）。**
> 本文侧重原理、dev/release 差异、验证与故障恢复。

## 1. 这是什么 / 为什么

`release` env 产出**明文**全量固件，任何人用 esptool `read_flash` 即可 dump 出来逆向。
`secure` env 在 **不改动 release 流程** 的前提下，产出 **ESP32-S3 Flash Encryption
(Development Mode)** 的**密文**镜像：flash 内容以 XTS-AES-128 加密存储，dump 出来是密文。

固件二进制与 `release` **完全同构**（同一套 `-DBOARD_VARIANT_RELEASE` / 产品引脚），
加密纯属 flash 层 + 一次性 eFuse，**无任何固件源码差异**。

| 模式 | 可重刷/调试 | 防 dump | 本项目 |
|------|------------|---------|--------|
| **Development**（本方案）| 是（设备仍可反复平刷、调试）| 是 | ✅ 采用 |
| Release | 否（锁死，UART 无法再下明文）| 是（更强）| ❌ 不在范围 |

> 仅 Flash Encryption；**不含** Secure Boot v2（不防刷入未授权固件，只防 dump）。

## 2. 不可逆性 —— 先读这一段

- eFuse 是一次性熔丝：烧密钥、置 `SPI_BOOT_CRYPT_CNT` **无法撤销，每芯片仅一次**。
- provisioning 烧录中 **切勿断电**，否则 flash 内容损坏。
- **密钥即一切**：`secure_keys/flash_encryption_key.bin`
  - 丢失 → 无法再为该设备产出可用固件；
  - 泄漏 → 防 dump 完全失效。
  - 必须**离线备份**（加密 U 盘 / 密码管理器 / KMS），**绝不入库**
    （`.gitignore` 已覆盖 `secure_keys/`、`*.key`、`*.bin`）。

## 3. 一次性准备：生成并备份密钥

整个项目/产品线**共用一把密钥**（同一把才能让同一镜像刷多台设备）。只生成一次：

```powershell
# penv python 路径示例：%USERPROFILE%\.platformio\penv\Scripts\python.exe
& "$env:USERPROFILE\.platformio\penv\Scripts\python.exe" -m espsecure `
    generate_flash_encryption_key -l 256 secure_keys\flash_encryption_key.bin
```

- `-l 256` → 256-bit，对应 ESP32-S3 **XTS-AES-128**，占用单个 eFuse block。
- 生成后**立即离线备份**。脚本不会覆盖已存在的密钥。

## 4. 构建加密固件

```bash
pio run -e secure
```

`post:scripts/secure_package.py` 自动：

1. 校验 `secure_keys/flash_encryption_key.bin`（缺失会报错并给出生成命令）；
2. 用 `espsecure encrypt_flash_data -x` 按真实 flash 偏移分别加密：
   bootloader@`0x0`、partitions@`0x8000`、app@`0x10000`；
3. `esptool merge_bin` 合并三段密文 + 空 SPIFFS（明文 `0xFF`）；
4. 产出 `release/esm_power_firmware_secure_{ver}_{date}.bin`。

> 仅 `secure` env 触发该脚本；`release` / `esp32s3` / `test` 完全不受影响。

## 5. 每台设备一次：eFuse provisioning（不可逆）

`scripts/provision_efuse.py` 是**独立手动工具**，不会被 `pio run` 触发。

```bash
# (a) 只读查看设备当前 eFuse（安全）
python scripts/provision_efuse.py --port COM7 --summary-only

# (b) dry-run：打印将执行的不可逆命令，不碰硬件
python scripts/provision_efuse.py --port COM7

# (c) 真正烧录：二次交互确认 + espefuse 自身再确认（纵深防御）
python scripts/provision_efuse.py --port COM7 --commit
```

烧录两步（dev-mode）：

1. `burn_key BLOCK_KEY0 <key> XTS_AES_128_KEY` —— 密钥块默认读保护（芯片可用、主机读不回）；
2. `burn_efuse SPI_BOOT_CRYPT_CNT 1` —— 启用加密。

dev-mode 刻意**不**执行：`DIS_DOWNLOAD_MANUAL_ENCRYPT`、`SPI_BOOT_CRYPT_CNT` 写保护、
JTAG/下载模式锁 —— 因此设备保持可重刷、可调试。脚本检测到设备已启用加密会**拒绝重复烧录**。

## 6. 烧录加密镜像

镜像**已是密文**，按普通全量刷机即可，**绝不要加 `--encrypt`**
（加了等于二次加密，设备无法解密启动）：

```bash
esptool.py --chip esp32s3 --port COM7 --baud 921600 \
    write_flash 0x0 release/esm_power_firmware_secure_<ver>_<date>.bin
```

前提：该设备已完成第 5 步 provisioning（已烧入**同一把**密钥）。

## 7. dev-mode 重刷行为

- 设备保持可重刷：可继续平刷新的**加密镜像**（步骤 4→6）。
- 也可用 `write_flash --encrypt` 直接下**明文** app（bootloader 在线加密）做快速联调。
- OTA：运行时 app1/otadata 由设备自行加密，无需主机预处理；首刷只含
  bootloader/partitions/app + 空 SPIFFS（与 release 打包一致）。

## 8. 验证（不烧死芯片）

构建侧（无需硬件）——证明加密可逆且偏移正确：

```bash
PENV=...\.platformio\penv\Scripts\python.exe
$PENV -m espsecure encrypt_flash_data -k secure_keys\flash_encryption_key.bin \
    -a 0x10000 -x -o app.enc .pio\build\secure\firmware.bin
$PENV -m espsecure decrypt_flash_data -k secure_keys\flash_encryption_key.bin \
    -a 0x10000 -x -o app.dec app.enc
# app.dec 应与 .pio\build\secure\firmware.bin 二进制一致
```

设备侧（需一颗**可牺牲**的 ESP32-S3）：provisioning → 平刷加密镜像 → 正常启动；
`esptool read_flash 0x10000 0x1000 dump.bin` 读回应为**密文**（验证防 dump）；
再平刷一次验证 dev-mode 可重刷。

## 9. 故障与恢复

| 现象 | 原因 / 处理 |
|------|------------|
| 启动循环 / 无显示 | 误用 `--encrypt` 二次加密；或密钥与设备 eFuse 不匹配。用正确镜像重刷 |
| `secure_package.py` 报缺密钥 | 按第 3 步生成并备份 |
| provisioning 报已启用 | 设备已 provisioned，跳过第 5 步，直接第 6 步刷机 |
| 换了新密钥后旧设备无法启动 | FE 密钥每芯片固化，旧设备只能用其 eFuse 内那把密钥的镜像 |

## 10. 相关文件

- `platformio.ini` → `[env:secure]`（克隆 `[env:release]` + `secure_8MB.csv` + 本套脚本）
- `partitions/secure_8MB.csv`（app0/app1 带 `encrypted` 标志）
- `scripts/secure_package.py`（post-build 预加密打包，`PIOENV==secure` 门控）
- `scripts/provision_efuse.py`（独立手动 eFuse 工具，默认 dry-run）
- `secure_keys/flash_encryption_key.bin`（**不入库**，离线备份）
