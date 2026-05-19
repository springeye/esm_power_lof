"""
PlatformIO extra_script: [env:secure] 构建完成后，主机侧预加密并合并全量固件。

与 [env:release] 的差异：
    release  -> 明文全量固件（任何人可 read_flash dump 出来）
    secure   -> Flash Encryption (dev-mode) 密文全量固件（dump 出来是密文）

仅在 [env:secure] 生效。其他环境（含 release）不做任何事，零干扰。

固件二进制与 release 同构（同一套 -DBOARD_VARIANT_RELEASE / 产品引脚），
加密纯属 flash 层：用 espsecure 按各段真实 flash 偏移做 XTS-AES 预加密，
再用 esptool merge_bin 合并成可平刷的密文镜像。

前置条件：
    secure_keys/flash_encryption_key.bin 必须存在（人工生成并离线备份，不入库）。
    生成命令见下方报错提示或 docs/flash-encryption-guide.md。

eFuse 烧录（烧密钥 + SPI_BOOT_CRYPT_CNT，每芯片一次且不可逆）
*不在本脚本范围*，见 scripts/provision_efuse.py。

用法 (platformio.ini):
    extra_scripts = post:scripts/secure_package.py

也可手动运行（先 pio run -e secure）：
    python scripts/secure_package.py
"""

import subprocess
import sys
import os
import tempfile
import datetime
from pathlib import Path


# 与 partitions/secure_8MB.csv 保持一致（布局同 default_8MB，app0/app1 带 encrypted 标志）
# PCB 为 N16R8，flash 头声明 16MB 以匹配真实硬件，固件仅占用前 8MB
FLASH_SIZE_MB = 16
BOOTLOADER_OFFSET = 0x0000      # ESP32-S3 bootloader 偏移 = 0x0
PARTITIONS_OFFSET = 0x8000
APP_OFFSET = 0x10000
SPIFFS_OFFSET = 0x690000
SPIFFS_SIZE = 0x160000
# coredump 分区 (0x7F0000-0x800000) 由运行时自行管理，不预填充
FIRMWARE_SIZE = SPIFFS_OFFSET + SPIFFS_SIZE  # 0x7F0000 = 8323072 bytes

# 加密密钥（256-bit / XTS-AES-128，单 eFuse block）。绝不入库（.gitignore 已覆盖 secure_keys/）
KEY_RELPATH = "secure_keys/flash_encryption_key.bin"

# 需要按 flash 偏移预加密的段（spiffs 在 secure_8MB.csv 无 encrypted 标志，保持明文 0xFF）
ENCRYPT_TARGETS = (
    ("bootloader.bin", BOOTLOADER_OFFSET),
    ("partitions.bin", PARTITIONS_OFFSET),
    ("firmware.bin",   APP_OFFSET),
)


def _log(msg):
    print(f"[secure_package] {msg}")


import shutil


def _penv_python_path():
    home = os.environ.get("PLATFORMIO_HOME_DIR",
                          str(Path.home() / ".platformio"))
    p = Path(home) / "penv" / "Scripts" / "python.exe"
    if not p.exists():
        p = Path(home) / "penv" / "bin" / "python"
    return str(p) if p.exists() else None


def _python_candidates():
    """候选解释器（去重，保序）：penv 优先（与 release_package.py 一致），再系统。"""
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


def find_tool_python(require=True):
    """返回一个 `-m espsecure version` 真正可运行的解释器。

    penv 自带的 esptool/espsecure 在部分机器上 cryptography 缺 _cffi_backend
    （penv 未装 cffi），仅 merge_bin 能用、加密会崩。这里逐个候选实测筛选。
    """
    global _tool_python_cache
    if _tool_python_cache is not None:
        return _tool_python_cache

    override = os.environ.get("ESM_TOOL_PYTHON")
    candidates = ([override] if override else []) + _python_candidates()

    tried = []
    for cand in candidates:
        try:
            r = subprocess.run([cand, "-c", "import espsecure"],
                               capture_output=True, text=True, timeout=60)
        except Exception as e:
            tried.append(f"{cand} -> {e}")
            continue
        if r.returncode == 0:
            _tool_python_cache = cand
            _log(f"  [OK] espsecure 可用: {cand}")
            return cand
        last = (r.stderr or r.stdout).strip().splitlines()[-1:]
        tried.append(f"{cand} -> rc={r.returncode}: {last}")

    _log("[FAIL] 找不到可运行 espsecure 的 Python（加密无法进行）。已尝试：")
    for t in tried:
        _log(f"  - {t}")
    _log("常见原因：PlatformIO penv 的 cryptography 缺 _cffi_backend。修复其一：")
    penv = _penv_python_path() or "<penv-python>"
    _log(f'  A) "{penv}" -m pip install cffi')
    _log('  B) 在独立 venv 中  pip install esptool  并将其 python 加入 PATH')
    if require:
        sys.exit(1)
    return penv if isinstance(penv, str) else sys.executable


def get_version(project_root):
    for args in (["git", "describe", "--tags", "--always", "--dirty"],
                 ["git", "rev-parse", "--short", "HEAD"]):
        try:
            result = subprocess.run(
                args, cwd=str(project_root), capture_output=True, text=True
            )
            if result.returncode == 0 and result.stdout.strip():
                return result.stdout.strip()
        except Exception:
            pass
    return datetime.datetime.now().strftime("%Y%m%d")


def verify_binaries(build_dir):
    for name, offset in ENCRYPT_TARGETS:
        path = build_dir / name
        if not path.exists():
            _log(f"[FAIL] 缺少构建产物: {path}")
            sys.exit(1)
        size = path.stat().st_size
        _log(f"  [OK] {name}  offset=0x{offset:06X}  size={size} bytes ({size/1024:.1f} KB)")


def require_key(project_root):
    key_path = project_root / KEY_RELPATH
    if key_path.exists():
        size = key_path.stat().st_size
        if size != 32:
            _log(f"[FAIL] 密钥长度异常: {key_path} = {size} bytes（应为 32 bytes / 256-bit XTS-AES-128）")
            sys.exit(1)
        _log(f"  [OK] 加密密钥: {key_path} (256-bit)")
        return key_path

    _log("[FAIL] 未找到 Flash Encryption 密钥，无法加密打包。")
    _log(f"       期望路径: {key_path}")
    _log("       请人工生成并*离线备份*（密钥丢失=无法再为设备产对应固件；泄漏=防 dump 失效）：")
    py = find_tool_python(require=False)
    _log(f'       "{py}" -m espsecure generate_flash_encryption_key -l 256 "{key_path}"')
    _log("       详见 docs/flash-encryption-guide.md")
    sys.exit(1)


def encrypt_segment(penv_py, key_path, src_path, address, out_path):
    """espsecure XTS-AES 预加密单段（-x = --aes_xts）。"""
    cmd = [
        penv_py, "-m", "espsecure", "encrypt_flash_data",
        "-k", str(key_path),
        "-a", hex(address),
        "-x",
        "-o", str(out_path),
        str(src_path),
    ]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        _log(f"[FAIL] espsecure 加密失败 ({src_path.name} @ {hex(address)}):")
        _log(result.stdout.strip())
        _log(result.stderr.strip())
        sys.exit(1)
    _log(f"  [OK] 已加密 {src_path.name} @ {hex(address)} -> {out_path.name}")


def create_empty_spiffs(tmp_dir):
    path = tmp_dir / "spiffs_empty.bin"
    with open(path, "wb") as f:
        f.write(b'\xff' * SPIFFS_SIZE)
    _log(f"  [OK] 创建空 SPIFFS 镜像（明文）  offset=0x{SPIFFS_OFFSET:06X}  size={SPIFFS_SIZE} bytes")
    return path


def merge_firmware(penv_py, enc_paths, spiffs_path, output_path):
    cmd = [
        penv_py, "-m", "esptool",
        "--chip", "esp32s3",
        "merge_bin",
        "--flash_mode", "dio",
        "--flash_size", f"{FLASH_SIZE_MB}MB",
        "--flash_freq", "80m",
        "-o", str(output_path),
        hex(BOOTLOADER_OFFSET), str(enc_paths["bootloader.bin"]),
        hex(PARTITIONS_OFFSET), str(enc_paths["partitions.bin"]),
        hex(APP_OFFSET),        str(enc_paths["firmware.bin"]),
        hex(SPIFFS_OFFSET),     str(spiffs_path),
    ]

    _log("正在合并全量密文固件 ...")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        _log(f"[FAIL] esptool 合并失败:\n{result.stderr}")
        sys.exit(1)
    if result.stderr:
        for line in result.stderr.strip().split('\n'):
            _log(f"  {line}")

    size = output_path.stat().st_size
    if size == FIRMWARE_SIZE:
        _log(f"[OK] 全量密文固件已生成: {output_path} ({size} bytes = {size/(1024*1024):.1f}MB)")
    else:
        _log(f"[WARN] 固件大小异常: {size} bytes (预期 {FIRMWARE_SIZE} bytes)")


def do_package(project_root, build_dir, release_dir):
    """核心打包逻辑，供 standalone 和 extra_script 共用。"""
    _log(f"构建目录: {build_dir}")
    verify_binaries(build_dir)
    key_path = require_key(project_root)

    penv_py = find_tool_python()
    _log(f"使用工具 python: {penv_py}")

    release_dir.mkdir(parents=True, exist_ok=True)

    version = get_version(project_root)
    date_str = datetime.datetime.now().strftime("%Y%m%d")
    filename = f"esm_power_firmware_secure_{version}_{date_str}.bin"
    output_path = release_dir / filename

    with tempfile.TemporaryDirectory() as tmp_dir:
        tmp_path = Path(tmp_dir)

        enc_paths = {}
        for name, address in ENCRYPT_TARGETS:
            out = tmp_path / f"{name}.enc"
            encrypt_segment(penv_py, key_path, build_dir / name, address, out)
            enc_paths[name] = out

        spiffs_path = create_empty_spiffs(tmp_path)
        merge_firmware(penv_py, enc_paths, spiffs_path, output_path)

    _log(f"固件文件: {output_path}")
    _log("烧录（镜像已是密文，*不要*加 --encrypt；目标设备须已烧入匹配密钥）：")
    _log(f"  esptool.py --chip esp32s3 --port <COM> --baud 921600 write_flash 0x0 {filename}")
    _log("设备 eFuse provisioning（每芯片一次、不可逆）见 scripts/provision_efuse.py")
    return output_path


try:
    from SCons.Script import Import  # type: ignore
    Import("env")  # type: ignore[name-defined]
    _IS_EXTRA_SCRIPT = True
except (ImportError, AttributeError):
    _IS_EXTRA_SCRIPT = False


def _extra_script_post_action(target, source, env):
    """SCons post-action 回调: firmware.bin 构建完成后触发。"""
    project_dir = Path(env.subst("$PROJECT_DIR"))
    build_dir = Path(env.subst("$BUILD_DIR"))
    release_dir = project_dir / "release"

    _log("=" * 60)
    _log("ESP32-S3 Flash Encryption (dev-mode) 加密固件打包 (post-build)")
    _log("=" * 60)

    do_package(project_dir, build_dir, release_dir)


if _IS_EXTRA_SCRIPT:
    PIOENV = env.subst("$PIOENV")  # type: ignore[name-defined]
    if PIOENV == "secure":
        _log(f"已注册 post-build 钩子 (PIOENV={PIOENV})")
        env.AddPostAction(  # type: ignore[name-defined]
            "$BUILD_DIR/${PROGNAME}.bin",
            _extra_script_post_action
        )

if __name__ == "__main__":
    project_root = Path(__file__).resolve().parent.parent
    build_dir = project_root / ".pio" / "build" / "secure"
    release_dir = project_root / "release"

    print("=" * 60)
    print("ESP32-S3 Flash Encryption (dev-mode) 加密固件打包")
    print("=" * 60)
    print("\n[1/2] 编译 secure 构建 ...")
    subprocess.run(["pio", "run", "-e", "secure"], cwd=str(project_root), check=True)
    print("\n[2/2] 主机侧预加密 + 合并全量固件 ...")
    do_package(project_root, build_dir, release_dir)
    print("\n完成")
