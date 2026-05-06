"""
ESP32-S3 全量固件发布打包脚本

用法:
    python scripts/release_package.py

流程:
    1. 执行 pio run -e release 编译 release 构建
    2. 合并 bootloader + partitions + firmware + 空 SPIFFS → 单个 8MB .bin
    3. 输出到 release/ 目录，文件名含版本号/日期

依赖:
    - PlatformIO (pio 命令可用)
    - esptool.py (PlatformIO 自带的 tool-esptoolpy)
"""

import subprocess
import sys
import os
import tempfile
import shutil
import datetime
from pathlib import Path

if sys.platform == "win32" and hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")


PROJECT_ROOT = Path(__file__).resolve().parent.parent
BUILD_DIR = PROJECT_ROOT / ".pio" / "build" / "release"
RELEASE_DIR = PROJECT_ROOT / "release"

# 与 partitions/default_8MB.csv 保持一致
FLASH_SIZE_MB = 8
BOOTLOADER_OFFSET = 0x0000
PARTITIONS_OFFSET = 0x8000
APP_OFFSET = 0x10000
SPIFFS_OFFSET = 0x690000
SPIFFS_SIZE = 0x160000
# coredump 分区 (0x7F0000-0x800000) 由运行时自行管理，不预填充
FIRMWARE_SIZE = SPIFFS_OFFSET + SPIFFS_SIZE  # 0x7F0000 = 8323072 bytes


def run(cmd, **kwargs):
    """运行命令，失败时退出"""
    print(f"  -> {' '.join(str(c) for c in cmd)}")
    result = subprocess.run(cmd, cwd=str(PROJECT_ROOT), **kwargs)
    if result.returncode != 0:
        print(f"[FAIL] 命令失败 (exit_code={result.returncode})")
        sys.exit(result.returncode)
    return result


def find_esptool():
    """在 PlatformIO 环境中查找 esptool.py"""
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

    print("[WARN] 未找到 PlatformIO 的 esptool, 尝试系统 esptool ...")
    return [sys.executable, "-m", "esptool"]


def get_version():
    """从 git 获取版本号，失败则用日期"""
    try:
        result = subprocess.run(
            ["git", "describe", "--tags", "--always", "--dirty"],
            cwd=str(PROJECT_ROOT),
            capture_output=True, text=True
        )
        if result.returncode == 0:
            return result.stdout.strip()
    except Exception:
        pass

    try:
        result = subprocess.run(
            ["git", "rev-parse", "--short", "HEAD"],
            cwd=str(PROJECT_ROOT),
            capture_output=True, text=True
        )
        if result.returncode == 0:
            return result.stdout.strip()
    except Exception:
        pass

    return datetime.datetime.now().strftime("%Y%m%d")


def verify_binaries():
    """检查构建产物是否存在"""
    required = {
        "bootloader.bin": BOOTLOADER_OFFSET,
        "partitions.bin": PARTITIONS_OFFSET,
        "firmware.bin":   APP_OFFSET,
    }
    for name, offset in required.items():
        path = BUILD_DIR / name
        if not path.exists():
            print(f"[FAIL] 缺少构建产物: {path}")
            print(f"  请先运行: pio run -e release")
            sys.exit(1)
        size = path.stat().st_size
        print(f"  [OK] {name}  offset=0x{offset:06X}  size={size} bytes ({size/1024:.1f} KB)")


def create_empty_spiffs(tmp_dir):
    """创建全 0xFF 填充的空 SPIFFS 镜像"""
    path = tmp_dir / "spiffs_empty.bin"
    with open(path, "wb") as f:
        f.write(b'\xff' * SPIFFS_SIZE)
    print(f"  [OK] 创建空 SPIFFS 镜像  offset=0x{SPIFFS_OFFSET:06X}  size={SPIFFS_SIZE} bytes ({SPIFFS_SIZE/1024:.1f} KB)")
    return path


def merge_firmware(esptool_cmd, spiffs_path, output_path):
    """使用 esptool merge_bin 合并完整固件"""
    cmd = esptool_cmd + [
        "--chip", "esp32s3",
        "merge_bin",
        "--flash_mode", "dio",
        "--flash_size", f"{FLASH_SIZE_MB}MB",
        "--flash_freq", "80m",
        "-o", str(output_path),
        hex(BOOTLOADER_OFFSET), str(BUILD_DIR / "bootloader.bin"),
        hex(PARTITIONS_OFFSET), str(BUILD_DIR / "partitions.bin"),
        hex(APP_OFFSET),       str(BUILD_DIR / "firmware.bin"),
        hex(SPIFFS_OFFSET),    str(spiffs_path),
    ]

    print(f"\n正在合并全量固件 ...")
    run(cmd, check=True)

    size = output_path.stat().st_size
    if size == FIRMWARE_SIZE:
        print(f"[OK] 全量固件已生成: {output_path} ({size} bytes = {size/(1024*1024):.1f}MB)")
    else:
        print(f"[WARN] 固件大小异常: {size} bytes (预期 {FIRMWARE_SIZE} bytes)")


def main():
    print("=" * 60)
    print("ESP32-S3 全量固件 Release 打包")
    print("=" * 60)

    print("\n[1/4] 编译 release 构建 ...")
    run(["pio", "run", "-e", "release"])

    print("\n[2/4] 验证构建产物 ...")
    verify_binaries()

    print("\n[3/4] 合并全量固件 ...")
    esptool_cmd = find_esptool()
    print(f"  使用 esptool: {' '.join(esptool_cmd)}")

    RELEASE_DIR.mkdir(parents=True, exist_ok=True)

    version = get_version()
    date_str = datetime.datetime.now().strftime("%Y%m%d")
    filename = f"esm_power_firmware_{version}_{date_str}.bin"
    output_path = RELEASE_DIR / filename

    with tempfile.TemporaryDirectory() as tmp_dir:
        tmp_path = Path(tmp_dir)
        spiffs_path = create_empty_spiffs(tmp_path)
        merge_firmware(esptool_cmd, spiffs_path, output_path)

    print("\n[4/4] 完成")
    print(f"""
{'=' * 60}
固件文件:  {output_path}
烧录命令:
  esptool.py --chip esp32s3 --port <COM口> --baud 921600 \\
    write_flash 0x0 {filename}

  或使用 PlatformIO:
  pio run -e release -t upload
{'=' * 60}
""")


if __name__ == "__main__":
    main()
