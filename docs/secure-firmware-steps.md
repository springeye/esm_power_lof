# 加密固件操作手册：从打包到刷写

> 本文是**逐步操作手册**（照着做即可）。原理、dev/release 模式差异、故障恢复见
> [`flash-encryption-guide.md`](flash-encryption-guide.md)。命令以 **Windows PowerShell** 为准。

## 全流程总览

```
[一次性] 环境准备 ──> [一次性] 生成并备份密钥
                              │
                   ┌──────────┴──────────┐
              [每次发布] 打包          [每台设备一次] eFuse provisioning（不可逆）
              pio run -e secure        provision_efuse.py --commit
                   └──────────┬──────────┘
                        [每台设备] 刷写加密镜像 write_flash 0x0（不加 --encrypt）
                              │
                        [每台设备] 上电验证 + 读回确认为密文
```

阶段标记：🔧 一次性环境 ｜ 🔑 一次性密钥 ｜ 📦 每次发布 ｜ ⚠ 每台设备一次且不可逆 ｜ ✅ 验证

---

## 🔧 步骤 0：环境准备（每台开发机一次）

加密用到 `espsecure`/`espefuse`，它们依赖可用的 `cryptography`。本机 PlatformIO penv
默认缺 `cffi` → 直接用会报 `ModuleNotFoundError: No module named '_cffi_backend'`。
**二选一**修好，并记住要用的 Python：

**方案 A（推荐，最省事）——给 penv 补 cffi：**

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\python.exe" -m pip install cffi
```

**方案 B——独立 venv，再用环境变量指给脚本：**

```powershell
python -m venv "$env:USERPROFILE\esmfe-venv"
& "$env:USERPROFILE\esmfe-venv\Scripts\python.exe" -m pip install "esptool>=4.7,<5"
$env:ESM_TOOL_PYTHON = "$env:USERPROFILE\esmfe-venv\Scripts\python.exe"
```

> 方案 B 的 `$env:ESM_TOOL_PYTHON` 只在当前 PowerShell 会话有效；每开新窗口要重设，
> 或写入系统环境变量永久生效。`secure_package.py` / `provision_efuse.py` 会优先用它。

**自检**（应打印 `OK`，不报 `_cffi_backend`）：

```powershell
$PY = $env:ESM_TOOL_PYTHON; if (-not $PY) { $PY = "$env:USERPROFILE\.platformio\penv\Scripts\python.exe" }
& $PY -c "import espsecure, espefuse; print('OK')"
```

---

## 🔑 步骤 1：生成并备份加密密钥（整条产品线一次）

**全产品线共用一把密钥**——同一把才能让同一镜像刷多台设备。只生成一次，永久妥善保存。

```powershell
New-Item -ItemType Directory -Force secure_keys | Out-Null
& $PY -m espsecure generate_flash_encryption_key -l 256 secure_keys\flash_encryption_key.bin
```

- `-l 256` → 256-bit，对应 ESP32-S3 **XTS-AES-128**，占单个 eFuse block。
- 生成后 **立即离线备份**（加密 U 盘 / 密码管理器 / KMS）。
- 该文件**绝不入库**：`.gitignore` 已覆盖 `secure_keys/`、`*.key`、`*.bin`。
- ⚠ 密钥**丢失** = 无法再为已烧录设备产出可用固件；**泄漏** = 防 dump 完全失效。

校验（应为 `32`）：

```powershell
(Get-Item secure_keys\flash_encryption_key.bin).Length
```

---

## 📦 步骤 2：打包加密固件（每次发布）

```powershell
pio run -e secure
```

`post:scripts/secure_package.py` 会自动：校验密钥 → 按 flash 偏移分段 XTS 加密
（bootloader@`0x0`、partitions@`0x8000`、app@`0x10000`）→ `merge_bin` 合并 → 产出：

```
release\esm_power_firmware_secure_<版本>_<日期>.bin
```

**成功判据**：日志末尾出现 `[OK] 全量密文固件已生成 ... (8323072 bytes = 7.9MB)`，
且 `release\` 下生成上述 `.bin`。仅 `secure` 触发本脚本，`release`/`esp32s3` 不受影响。

> 已编译过、只想重新打包：`python scripts\secure_package.py`（内部会先跑 `pio run -e secure`）。

### （可选但推荐）打包自检：解密对拍

证明镜像可被正确解密（= 已烧密钥设备能启动），不需要硬件：

```powershell
$BIN = (Get-ChildItem release\esm_power_firmware_secure_*.bin | Sort-Object LastWriteTime | Select-Object -Last 1).FullName
$fw  = ".pio\build\secure\firmware.bin"
$len = (Get-Item $fw).Length
# 从合并镜像 0x10000 处取出 app 密文段
$buf = [System.IO.File]::ReadAllBytes($BIN)[0x10000 .. (0x10000 + $len - 1)]
[System.IO.File]::WriteAllBytes("$env:TEMP\app.enc", $buf)
& $PY -m espsecure decrypt_flash_data -k secure_keys\flash_encryption_key.bin -a 0x10000 -x -o "$env:TEMP\app.dec" "$env:TEMP\app.enc"
"明文 : " + (Get-FileHash $fw -Algorithm SHA256).Hash
"解密 : " + (Get-FileHash "$env:TEMP\app.dec" -Algorithm SHA256).Hash
```

两个 SHA256 **完全一致** = 通过。

---

## ⚠ 步骤 3：设备 eFuse provisioning（每台设备一次，**不可逆**）

把设备接上电脑并进入**下载模式**（按住 BOOT → 短按 RESET → 松开 BOOT），确认串口号（如 `COM7`）。

> ⚠ eFuse 是一次性熔丝，本步**无法撤销，每颗芯片只做一次**；过程中**切勿断电**。
> 本工具是独立手动脚本，`pio run` **不会**触发它。

**3.1 先只读看设备当前状态（安全）：**

```powershell
python scripts\provision_efuse.py --port COM7 --summary-only
```

若 `SPI_BOOT_CRYPT_CNT` 已非 `0b000` → 该设备已 provisioned，**跳到步骤 4**。

**3.2 dry-run 预览将执行的不可逆命令（不碰硬件）：**

```powershell
python scripts\provision_efuse.py --port COM7
```

核对它将执行（dev-mode，刻意**不**写保护、**不**烧 `DIS_DOWNLOAD_MANUAL_ENCRYPT`）：

```
[1] ... espefuse --chip esp32s3 --port COM7 burn_key BLOCK_KEY0 secure_keys\flash_encryption_key.bin XTS_AES_128_KEY
[2] ... espefuse --chip esp32s3 --port COM7 burn_efuse SPI_BOOT_CRYPT_CNT 1
```

**3.3 真正烧录（不可逆）：**

```powershell
python scripts\provision_efuse.py --port COM7 --commit
```

会要求你**逐字键入串口号**和确认短语 `BURN-EFUSE-IRREVERSIBLE`，之后 espefuse 自身还会再确认一次。
完成后脚本会提示复核命令。

---

## 步骤 4：刷写加密镜像到设备（每台设备）

镜像**已是密文**，按普通全量刷机，**绝不要加 `--encrypt`**（加了=二次加密，无法启动）。
设备仍在下载模式：

```powershell
$BIN = (Get-ChildItem release\esm_power_firmware_secure_*.bin | Sort-Object LastWriteTime | Select-Object -Last 1).FullName
& $PY -m esptool --chip esp32s3 --port COM7 --baud 921600 write_flash 0x0 $BIN
```

前提：该设备已完成步骤 3（已烧入**同一把**密钥）。

> 也可用 Web 刷机（https://espressif.github.io/esptool-js/）或 Flash Download Tool：
> 芯片选 ESP32-S3，地址 `0x0`，选上面那个 `.bin`，**不要**勾任何加密选项。

### 📌 为什么这里不能加 `--encrypt`？什么时候才用它？

`--encrypt`（GUI 工具里的 **Encrypt** 勾选框）= **"我发的是明文，芯片你用 eFuse 里的密钥边写边加密"**。
让密文进 flash 有两条路，本项目走第一条：

| 路径 | 谁加密 | 线上数据 | 主机要密钥吗 | 刷写方式 |
|------|--------|---------|-------------|---------|
| **主机预加密**（本项目 secure 方案）| 打包时 `espsecure` | 已是**密文** | 要 | 普通 `write_flash`，**不加** `--encrypt` |
| 设备侧加密 `--encrypt` | **芯片自己** | **明文** | 不要 | `write_flash --encrypt` |

- **本步镜像已是密文**：再 `--encrypt` → 芯片二次加密 → 读取只解密一次 → 仍是乱码 → **无法启动**。
  这就是"双重加密"陷阱，所以这里**绝不加** `--encrypt`、GUI **不勾** Encrypt（有 `DoNotChgBin` 则勾上）。
- **`--encrypt` 何时有用**：dev-mode 下**联调快速迭代**——跳过打包，直接把
  `.pio\build\secure\firmware.bin`（**明文**）用 `--encrypt` 刷到 `0x10000`，芯片自动加密：

  ```powershell
  & $PY -m esptool --chip esp32s3 --port COM7 write_flash --encrypt 0x10000 .pio\build\secure\firmware.bin
  ```

  前提：设备已 provisioned 且处于 dev-mode（没烧 `DIS_DOWNLOAD_MANUAL_ENCRYPT`）。
  好处：刷机机**无需持有密钥**。这条快速通道正是我们选 dev-mode 而非 release-mode 的原因之一
  （release-mode 会烧掉 `--encrypt` 能力）。

---

## ✅ 步骤 5：上电验证

```powershell
& $PY -m esptool --chip esp32s3 --port COM7 read_flash 0x10000 0x1000 "$env:TEMP\dump.bin"
& $PY -c "b=open(r'$env:TEMP\dump.bin','rb').read(); print('FF空白' if b[:16]==b'\xff'*16 else '密文(正常，防dump成立)')"
```

- 设备短按 RESET 应正常启动（开机动画 → 主界面）。
- `read_flash` 读回应为**密文** → 防 dump 成立。
- 屏幕无显示/反复重启：多半是误加了 `--encrypt`、或密钥与设备 eFuse 不匹配 →
  用正确镜像/密钥重刷；细节见 `flash-encryption-guide.md` 第 9 节。

---

## 后续维护（dev-mode 可重刷）

- 出新版本：重做**步骤 2**，对已 provisioned 的设备直接做**步骤 4**（无需再做步骤 3）。
- 联调快速迭代：可 `write_flash --encrypt` 直接下**明文** app（bootloader 在线加密），免预打包。
- OTA：运行时由设备自行加密 app1/otadata，无需主机预处理。

---

## 错误速查

| 现象 | 原因 / 处理 |
|------|------------|
| 打包报 `No module named '_cffi_backend'` / 找不到 espsecure | 未做步骤 0，按 A 或 B 修复 |
| 打包报 `未找到 Flash Encryption 密钥` | 未做步骤 1，生成并备份密钥 |
| provision 报“设备似乎已启用” | 该设备已 provisioned，跳到步骤 4 |
| 刷完反复重启 / 黑屏 | 误加 `--encrypt`，或密钥与设备 eFuse 不匹配 → 正确镜像重刷 |
| `espefuse` 连不上 | 未进下载模式 / 串口号错 / 串口被占用 |
| 换了新密钥后老设备起不来 | FE 密钥每芯片固化，老设备只认其 eFuse 内那把密钥的镜像 |

## 一页速查（命令清单）

```powershell
# 0 环境（一次）          & "$env:USERPROFILE\.platformio\penv\Scripts\python.exe" -m pip install cffi
# 0 选用的 Python          $PY = "$env:USERPROFILE\.platformio\penv\Scripts\python.exe"   # 或你的 venv python
# 1 生成密钥（一次）       & $PY -m espsecure generate_flash_encryption_key -l 256 secure_keys\flash_encryption_key.bin   # 然后离线备份！
# 2 打包（每次发布）       pio run -e secure
# 3 烧 eFuse（每设备一次） python scripts\provision_efuse.py --port COM7 --summary-only
#                          python scripts\provision_efuse.py --port COM7            # dry-run 预览
#                          python scripts\provision_efuse.py --port COM7 --commit   # 不可逆
# 4 刷写（每设备）         & $PY -m esptool --chip esp32s3 --port COM7 --baud 921600 write_flash 0x0 <release\...secure_....bin>   # 不加 --encrypt
# 5 验证                   设备应正常启动；read_flash 读回为密文
```
