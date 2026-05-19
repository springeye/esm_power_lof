"""
ESP32-S3 Flash Encryption (dev-mode) 设备 eFuse provisioning —— 独立手动工具。

*** 不可逆 / 每芯片仅一次 ***
本工具烧写 eFuse：把 Flash Encryption 密钥写入 BLOCK_KEY0，并把
SPI_BOOT_CRYPT_CNT 置 1 启用加密。eFuse 是一次性熔丝，写下去无法擦除/回退。

dev-mode（保持设备可重刷、可调试）刻意*不*烧以下项：
    - DIS_DOWNLOAD_MANUAL_ENCRYPT   （烧了=UART 无法再下载明文，锁死）
    - SPI_BOOT_CRYPT_CNT 的写保护    （写保护=无法再切换，锁死）
    - DIS_DOWNLOAD_MODE / JTAG 相关

本工具*不是* PlatformIO 编译钩子，不会被 pio run 触发，必须人工显式执行。

默认 dry-run：只打印将要执行的命令，不碰硬件。
真正烧录需同时满足：--commit + --port + 交互式键入确认短语。

用法：
    # 1) 先看设备当前 eFuse 状态（只读，安全）
    python scripts/provision_efuse.py --port COM7 --summary-only

    # 2) dry-run：打印将执行的烧录命令（不写 eFuse）
    python scripts/provision_efuse.py --port COM7

    # 3) 真正烧录（不可逆！会二次交互确认 + espefuse 自身还会再确认一次）
    python scripts/provision_efuse.py --port COM7 --commit

前置：secure_keys/flash_encryption_key.bin 已生成并*离线备份*。
详见 docs/flash-encryption-guide.md
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent
KEY_RELPATH = "secure_keys/flash_encryption_key.bin"

CHIP = "esp32s3"
KEY_BLOCK = "BLOCK_KEY0"
KEY_PURPOSE = "XTS_AES_128_KEY"   # 256-bit / 单 block / ESP32-S3
CONFIRM_PHRASE = "BURN-EFUSE-IRREVERSIBLE"


def _log(msg):
    print(f"[provision_efuse] {msg}")


def _penv_python_path():
    home = os.environ.get("PLATFORMIO_HOME_DIR",
                          str(Path.home() / ".platformio"))
    p = Path(home) / "penv" / "Scripts" / "python.exe"
    if not p.exists():
        p = Path(home) / "penv" / "bin" / "python"
    return str(p) if p.exists() else None


def _python_candidates():
    cands = []
    penv = _penv_python_path()
    if penv:
        cands.append(penv)
    for c in (sys.executable,
              shutil.which("python"),
              shutil.which("python3"),
              shutil.which("py")):
        if c:
            cands.append(c)
    seen, out = set(), []
    for c in cands:
        k = os.path.normcase(os.path.abspath(c)) if os.path.sep in c else c
        if k not in seen:
            seen.add(k)
            out.append(c)
    return out


_tool_python_cache = None


def find_tool_python():
    """返回一个 `-m espefuse version` 真正可运行的解释器。

    penv 自带 esptool 在部分机器上 cryptography 缺 _cffi_backend，espefuse
    烧 XTS 密钥会崩。逐个候选实测，全失败则给出修复指引并退出。
    """
    global _tool_python_cache
    if _tool_python_cache is not None:
        return _tool_python_cache

    override = os.environ.get("ESM_TOOL_PYTHON")
    candidates = ([override] if override else []) + _python_candidates()

    tried = []
    for cand in candidates:
        try:
            r = subprocess.run([cand, "-c", "import espefuse"],
                               capture_output=True, text=True, timeout=60)
        except Exception as e:
            tried.append(f"{cand} -> {e}")
            continue
        if r.returncode == 0:
            _tool_python_cache = cand
            _log(f"espefuse 可用: {cand}")
            return cand
        tried.append(f"{cand} -> rc={r.returncode}")

    _log("[FAIL] 找不到可运行 espefuse 的 Python。已尝试：")
    for t in tried:
        _log(f"  - {t}")
    _log("常见原因：PlatformIO penv 的 cryptography 缺 _cffi_backend。修复其一：")
    penv = _penv_python_path() or "<penv-python>"
    _log(f'  A) "{penv}" -m pip install cffi')
    _log('  B) 在独立 venv 中  pip install esptool  并将其 python 加入 PATH')
    sys.exit(1)


def require_key():
    key_path = PROJECT_ROOT / KEY_RELPATH
    if not key_path.exists():
        _log(f"[FAIL] 未找到密钥: {key_path}")
        py = _penv_python_path() or sys.executable
        _log("先生成并*离线备份*密钥：")
        _log(f'  "{py}" -m espsecure generate_flash_encryption_key -l 256 "{key_path}"')
        sys.exit(1)
    size = key_path.stat().st_size
    if size != 32:
        _log(f"[FAIL] 密钥长度异常: {size} bytes（应为 32 bytes / 256-bit XTS-AES-128）")
        sys.exit(1)
    return key_path


def run_summary(penv_py, port):
    cmd = [penv_py, "-m", "espefuse", "--chip", CHIP, "--port", port, "summary"]
    _log(f"读取 eFuse 状态: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    print(result.stdout)
    if result.stderr.strip():
        print(result.stderr)
    if result.returncode != 0:
        _log("[FAIL] espefuse summary 失败（检查 --port / 设备是否进入下载模式）")
        sys.exit(1)
    return result.stdout


def already_provisioned(summary_text):
    """best-effort：SPI_BOOT_CRYPT_CNT 非 0b000 视为已启用，禁止重复烧。"""
    for line in summary_text.splitlines():
        if "SPI_BOOT_CRYPT_CNT" in line:
            m = re.search(r"0b([01]{3})", line)
            if m and m.group(1) != "000":
                return True, line.strip()
            return False, line.strip()
    return False, "(未在 summary 中找到 SPI_BOOT_CRYPT_CNT 行，请人工核对)"


def build_burn_commands(penv_py, port, key_path):
    return [
        # 1) 烧密钥到 BLOCK_KEY0（espefuse 默认对密钥块读保护：芯片可用、主机读不回，符合防 dump）
        [penv_py, "-m", "espefuse", "--chip", CHIP, "--port", port,
         "burn_key", KEY_BLOCK, str(key_path), KEY_PURPOSE],
        # 2) 启用 Flash Encryption（置 1 个 bit；dev-mode：不写保护、不 max 到 0b111）
        [penv_py, "-m", "espefuse", "--chip", CHIP, "--port", port,
         "burn_efuse", "SPI_BOOT_CRYPT_CNT", "1"],
    ]


def print_plan(cmds):
    _log("将按顺序执行以下*不可逆* eFuse 操作：")
    for i, c in enumerate(cmds, 1):
        print(f"    [{i}] {' '.join(c)}")
    _log("dev-mode 刻意*不*执行：DIS_DOWNLOAD_MANUAL_ENCRYPT / 写保护 SPI_BOOT_CRYPT_CNT / JTAG 锁")


def confirm_interactive(port):
    print()
    print("=" * 70)
    print("  ⚠  不可逆操作警告 (ESP32-S3 eFuse)")
    print("=" * 70)
    print("  - eFuse 是一次性熔丝，本操作*无法撤销*，每颗芯片只应执行一次。")
    print("  - 烧录过程中*切勿断电*，否则 flash 内容会损坏。")
    print("  - 务必确认 secure_keys/flash_encryption_key.bin 已*离线备份*：")
    print("      丢失=无法再为本设备产对应固件；泄漏=防 dump 失效。")
    print(f"  - 目标串口: {port}")
    print("=" * 70)
    a = input(f"请逐字键入目标串口以确认 [{port}]: ").strip()
    if a != port:
        _log("串口不匹配，已中止。")
        return False
    b = input(f'请逐字键入确认短语 [{CONFIRM_PHRASE}]: ').strip()
    if b != CONFIRM_PHRASE:
        _log("确认短语不匹配，已中止。")
        return False
    return True


def main():
    ap = argparse.ArgumentParser(
        description="ESP32-S3 Flash Encryption (dev-mode) eFuse provisioning（默认 dry-run）")
    ap.add_argument("--port", help="设备串口，如 COM7 / /dev/ttyUSB0")
    ap.add_argument("--commit", action="store_true",
                    help="真正烧录 eFuse（不可逆）。不加=dry-run 只打印命令")
    ap.add_argument("--summary-only", action="store_true",
                    help="只读：打印设备当前 eFuse 状态后退出")
    args = ap.parse_args()

    if not args.port:
        _log("未指定 --port。dry-run 预览将执行的命令（端口用 <COM> 占位）：")
        preview_py = _penv_python_path() or sys.executable
        cmds = build_burn_commands(preview_py, "<COM>", PROJECT_ROOT / KEY_RELPATH)
        print_plan(cmds)
        _log("加 --port 可读取设备状态；加 --port + --commit 才会真正烧录。")
        return

    penv_py = find_tool_python()

    if args.summary_only:
        run_summary(penv_py, args.port)
        return

    key_path = require_key()
    summary = run_summary(penv_py, args.port)
    done, line = already_provisioned(summary)
    _log(f"SPI_BOOT_CRYPT_CNT: {line}")
    if done:
        _log("[ABORT] 设备似乎已启用 Flash Encryption，禁止重复 provisioning。")
        sys.exit(1)

    cmds = build_burn_commands(penv_py, args.port, key_path)
    print_plan(cmds)

    if not args.commit:
        _log("dry-run（未加 --commit）：未写入任何 eFuse。")
        _log("确认无误后，加 --commit 重新执行以真正烧录。")
        return

    if not confirm_interactive(args.port):
        sys.exit(1)

    for i, c in enumerate(cmds, 1):
        _log(f"执行 [{i}/{len(cmds)}]: {' '.join(c)}")
        # 不加 --do-not-confirm：保留 espefuse 自身的二次确认（纵深防御）
        rc = subprocess.run(c).returncode
        if rc != 0:
            _log(f"[FAIL] 第 {i} 步失败 (rc={rc})，已停止。请核对设备状态。")
            sys.exit(1)

    _log("=" * 60)
    _log("provisioning 完成。建议复核：")
    _log(f'  "{penv_py}" -m espefuse --chip {CHIP} --port {args.port} summary')
    _log("现在可平刷加密镜像（write_flash 0x0，*不要*加 --encrypt）。")
    _log("=" * 60)


if __name__ == "__main__":
    main()
