"""
PlatformIO extra_script: 自动发现源文件并注入 build_src_filter。

消除手工维护 platformio.ini 中 +<...> 条目的痛点:
- 新增 src/ 模块 → 自动编译，无需手改 platformio.ini
- LVGL Editor 导出新屏幕/字体 → 从 file_list_gen.cmake 自动发现

使用方式:
  [env:esp32s3]
  extra_scripts = pre:scripts/auto_src_filter.py
"""
import os
import re
import traceback
from pathlib import Path
from SCons.Script import Import  # type: ignore

Import("env")  # type: ignore[name-defined]  # SCons injected

PROJECT_DIR = Path(env.subst("$PROJECT_DIR"))  # type: ignore[name-defined]
SRC_DIR = PROJECT_DIR / "src"
UI_DIR = PROJECT_DIR / "ui"
FILE_LIST_CMAKE = UI_DIR / "file_list_gen.cmake"

_verbose_cache = None


def _is_verbose():
    global _verbose_cache
    if _verbose_cache is None:
        try:
            cppdefines = env.get("CPPDEFINES", [])  # type: ignore[name-defined]
            _verbose_cache = "AUTO_SRC_VERBOSE" in str(cppdefines)
        except Exception:
            _verbose_cache = False
    return _verbose_cache


def _log(msg):
    print(f"[auto_src_filter] {msg}")


def _warn(msg):
    print(f"[auto_src_filter] WARNING: {msg}")


def _vlog(msg):
    if _is_verbose():
        _log(f"  {msg}")


def _parse_file_list_cmake():
    if not FILE_LIST_CMAKE.is_file():
        _warn(f"未找到 {FILE_LIST_CMAKE}，回退到目录扫描")
        return _fallback_scan_ui()

    content = FILE_LIST_CMAKE.read_text(encoding="utf-8")
    pattern = r'\$\{CMAKE_CURRENT_LIST_DIR\}/([^\s)]+\.c)\b'
    matches = re.findall(pattern, content)

    if not matches:
        _warn("file_list_gen.cmake 中未找到 .c 文件，回退到目录扫描")
        return _fallback_scan_ui()

    return matches


def _fallback_scan_ui():
    files = []
    for dirpath, _, filenames in os.walk(UI_DIR):
        for f in filenames:
            if f.endswith(".c"):
                full = os.path.join(dirpath, f)
                rel = os.path.relpath(full, UI_DIR).replace("\\", "/")
                if "legacy" not in rel.split("/"):
                    files.append(rel)
    return files


def _ui_discovery():
    ui_files = _parse_file_list_cmake()
    entries = [f"+<../ui/{f}>" for f in ui_files]
    entries.append("-<ui/_legacy/>")
    return ui_files, entries


def _is_legacy(rel_path):
    return "_legacy" in rel_path.replace("\\", "/").split("/")


def _src_discovery():
    excluded = []

    for dirpath, _, filenames in os.walk(SRC_DIR):
        for f in filenames:
            if not f.endswith((".cpp", ".c")):
                continue
            rel = os.path.relpath(os.path.join(dirpath, f), SRC_DIR).replace("\\", "/")

            if _is_legacy(rel):
                excluded.append(rel)
                continue

    return excluded


def main():
    new_filters = []
    total_discovered = 0

    ui_files, ui_filters = _ui_discovery()
    new_filters.extend(ui_filters)
    total_discovered += len(ui_files)
    _log(f"UI: 发现 {len(ui_files)} 个文件 (来源: file_list_gen.cmake)")
    for f in ui_files:
        _vlog(f"  ui/{f}")

    excluded = _src_discovery()

    new_filters.append("+<../src/compat/lvgl_v8_shim.cpp>")
    _log(f"ESP32: +<*> 已覆盖 src/ 文件, "
         f"排除 {len(excluded)} 个 (legacy)")

    if new_filters:
        existing = env.get("SRC_FILTER", [])  # type: ignore[name-defined]
        if isinstance(existing, list):
            existing.extend(new_filters)
        elif existing:
            try:
                merged = list(existing) + new_filters
                env["SRC_FILTER"] = merged  # type: ignore[name-defined]
            except Exception:
                env["SRC_FILTER"] = new_filters  # type: ignore[name-defined]
        else:
            env["SRC_FILTER"] = new_filters  # type: ignore[name-defined]

        _log(f"总计注入 {len(new_filters)} 条自动发现过滤规则")
    else:
        _log("无新的自动发现条目")

    _log("完成")


try:
    main()
except Exception as e:
    _warn(f"脚本异常: {e}")
    _warn("回退到 platformio.ini 手写 build_src_filter 规则")
    traceback.print_exc()
