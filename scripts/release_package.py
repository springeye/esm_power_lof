"""
PlatformIO extra_script: release 构建完成后自动合并全量固件。

用法 (platformio.ini):
    extra_scripts = post:scripts/release_package.py

仅在 [env:release] 生效。其他环境不做任何事。

也可手动运行:
    python scripts/release_package.py
"""

import subprocess
import sys
import os
import tempfile
import datetime
from pathlib import Path


# 与 partitions/default_8MB.csv 保持一致
FLASH_SIZE_MB = 8
BOOTLOADER_OFFSET = 0x0000
PARTITIONS_OFFSET = 0x8000
APP_OFFSET = 0x10000
SPIFFS_OFFSET = 0x690000
SPIFFS_SIZE = 0x160000
# coredump 分区 (0x7F0000-0x800000) 由运行时自行管理，不预填充
FIRMWARE_SIZE = SPIFFS_OFFSET + SPIFFS_SIZE  # 0x7F0000 = 8323072 bytes


def _log(msg):
    print(f"[release_package] {msg}")


def find_esptool():
    home = os.environ.get("PLATFORMIO_HOME_DIR",
                          str(Path.home() / ".platformio"))

    penv_python = Path(home) / "penv" / "Scripts" / "python.exe"
    if not penv_python.exists():
        penv_python = Path(home) / "penv" / "bin" / "python"

    if penv_python.exists():
        return [str(penv_python), "-m", "esptool"]

    esptool_dir = Path(home) / "packages" / "tool-esptoolpy"
    if esptool_dir.exists():
        versions = sorted(esptool_dir.iterdir(), reverse=True)
        for v in versions:
            esptool_py = v / "esptool.py"
            if esptool_py.exists():
                if penv_python.exists():
                    return [str(penv_python), str(esptool_py)]
                return [sys.executable, str(esptool_py)]

    _log("[WARN] 未找到 PlatformIO 的 esptool, 尝试系统 esptool ...")
    return [sys.executable, "-m", "esptool"]


def get_version(project_root):
    try:
        result = subprocess.run(
            ["git", "describe", "--tags", "--always", "--dirty"],
            cwd=str(project_root), capture_output=True, text=True
        )
        if result.returncode == 0:
            return result.stdout.strip()
    except Exception:
        pass

    try:
        result = subprocess.run(
            ["git", "rev-parse", "--short", "HEAD"],
            cwd=str(project_root), capture_output=True, text=True
        )
        if result.returncode == 0:
            return result.stdout.strip()
    except Exception:
        pass

    return datetime.datetime.now().strftime("%Y%m%d")


def verify_binaries(build_dir):
    required = {
        "bootloader.bin": BOOTLOADER_OFFSET,
        "partitions.bin": PARTITIONS_OFFSET,
        "firmware.bin":   APP_OFFSET,
    }
    for name, offset in required.items():
        path = build_dir / name
        if not path.exists():
            _log(f"[FAIL] 缺少构建产物: {path}")
            sys.exit(1)
        size = path.stat().st_size
        _log(f"  [OK] {name}  offset=0x{offset:06X}  size={size} bytes ({size/1024:.1f} KB)")


def create_empty_spiffs(tmp_dir):
    path = tmp_dir / "spiffs_empty.bin"
    with open(path, "wb") as f:
        f.write(b'\xff' * SPIFFS_SIZE)
    _log(f"  [OK] 创建空 SPIFFS 镜像  offset=0x{SPIFFS_OFFSET:06X}  size={SPIFFS_SIZE} bytes ({SPIFFS_SIZE/1024:.1f} KB)")
    return path


def merge_firmware(esptool_cmd, build_dir, spiffs_path, output_path):
    cmd = esptool_cmd + [
        "--chip", "esp32s3",
        "merge_bin",
        "--flash_mode", "dio",
        "--flash_size", f"{FLASH_SIZE_MB}MB",
        "--flash_freq", "80m",
        "-o", str(output_path),
        hex(BOOTLOADER_OFFSET), str(build_dir / "bootloader.bin"),
        hex(PARTITIONS_OFFSET), str(build_dir / "partitions.bin"),
        hex(APP_OFFSET),       str(build_dir / "firmware.bin"),
        hex(SPIFFS_OFFSET),    str(spiffs_path),
    ]

    _log("正在合并全量固件 ...")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        _log(f"[FAIL] esptool 合并失败:\n{result.stderr}")
        sys.exit(1)
    if result.stderr:
        # esptool 把正常信息输出到 stderr
        for line in result.stderr.strip().split('\n'):
            _log(f"  {line}")

    size = output_path.stat().st_size
    if size == FIRMWARE_SIZE:
        _log(f"[OK] 全量固件已生成: {output_path} ({size} bytes = {size/(1024*1024):.1f}MB)")
    else:
        _log(f"[WARN] 固件大小异常: {size} bytes (预期 {FIRMWARE_SIZE} bytes)")


def do_package(project_root, build_dir, release_dir):
    """核心打包逻辑，供 standalone 和 extra_script 共用"""
    _log(f"构建目录: {build_dir}")
    verify_binaries(build_dir)

    esptool_cmd = find_esptool()
    _log(f"使用 esptool: {' '.join(esptool_cmd)}")

    release_dir.mkdir(parents=True, exist_ok=True)

    version = get_version(project_root)
    date_str = datetime.datetime.now().strftime("%Y%m%d")
    filename = f"esm_power_firmware_{version}_{date_str}.bin"
    output_path = release_dir / filename

    with tempfile.TemporaryDirectory() as tmp_dir:
        tmp_path = Path(tmp_dir)
        spiffs_path = create_empty_spiffs(tmp_path)
        merge_firmware(esptool_cmd, build_dir, spiffs_path, output_path)

    _log(f"固件文件: {output_path}")
    _log(f"烧录: esptool.py --chip esp32s3 --port <COM> --baud 921600 write_flash 0x0 {filename}")
    return output_path


try:
    from SCons.Script import Import  # type: ignore
    Import("env")  # type: ignore[name-defined]
    _IS_EXTRA_SCRIPT = True
except (ImportError, AttributeError):
    _IS_EXTRA_SCRIPT = False


def _extra_script_post_action(target, source, env):
    """SCons post-action 回调: firmware.bin 构建完成后触发"""
    project_dir = Path(env.subst("$PROJECT_DIR"))
    build_dir = Path(env.subst("$BUILD_DIR"))
    release_dir = project_dir / "release"

    _log("=" * 60)
    _log("ESP32-S3 全量固件 Release 打包 (post-build)")
    _log("=" * 60)

    do_package(project_dir, build_dir, release_dir)


if _IS_EXTRA_SCRIPT:
    PIOENV = env.subst("$PIOENV")  # type: ignore[name-defined]
    if PIOENV == "release":
        _log(f"已注册 post-build 钩子 (PIOENV={PIOENV})")
        env.AddPostAction(  # type: ignore[name-defined]
            "$BUILD_DIR/${PROGNAME}.bin",
            _extra_script_post_action
        )

if __name__ == "__main__":
    project_root = Path(__file__).resolve().parent.parent
    build_dir = project_root / ".pio" / "build" / "release"
    release_dir = project_root / "release"

    print("=" * 60)
    print("ESP32-S3 全量固件 Release 打包")
    print("=" * 60)
    print("\n[1/2] 编译 release 构建 ...")
    subprocess.run(["pio", "run", "-e", "release"], cwd=str(project_root), check=True)
    print("\n[2/2] 合并全量固件 ...")
    do_package(project_root, build_dir, release_dir)
    print("\n完成")
