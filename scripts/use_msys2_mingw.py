"""
PlatformIO extra_script: 为 native 环境注入 MSYS2 MinGW64 工具链。

背景:
  Windows 系统未将 MSYS2 加入全局 PATH, 但 C:\\msys64\\mingw64\\bin 已安装 g++ 15.2.0。
  本脚本仅在构建时把该路径前置到子进程 PATH, 并显式指定 CC/CXX/AR/RANLIB,
  避免污染系统环境, 也避免依赖开发者本机的 PATH 配置。

使用:
  在 platformio.ini 的 [env:native] 中加入:
    extra_scripts = pre:scripts/use_msys2_mingw.py
"""
import os
from SCons.Script import Import  # type: ignore

Import("env")

MINGW_BIN = r"C:\msys64\mingw64\bin"

if not os.path.isdir(MINGW_BIN):
    print(f"[use_msys2_mingw] 警告: 未找到 {MINGW_BIN}, 跳过工具链注入")
else:
    # 前置到当前构建子进程的 PATH (不影响系统环境)
    current_path = env["ENV"].get("PATH", "")
    if MINGW_BIN.lower() not in current_path.lower():
        env["ENV"]["PATH"] = MINGW_BIN + os.pathsep + current_path

    # 显式指定工具链可执行文件, 防止 SCons 回退到系统 PATH
    env.Replace(
        CC=os.path.join(MINGW_BIN, "gcc.exe"),
        CXX=os.path.join(MINGW_BIN, "g++.exe"),
        AR=os.path.join(MINGW_BIN, "ar.exe"),
        RANLIB=os.path.join(MINGW_BIN, "ranlib.exe"),
        LINK=os.path.join(MINGW_BIN, "g++.exe"),
    )
    print(f"[use_msys2_mingw] 已注入 MinGW64 工具链: {MINGW_BIN}")

# 测试构建时注入 PIO_UNIT_TESTING 宏, 让 src/native_main.cpp 跳过自身 main(),
# 避免和 Unity 测试入口的 main() 重定义冲突。
from SCons.Script import COMMAND_LINE_TARGETS  # type: ignore
if any("test" in t.lower() for t in COMMAND_LINE_TARGETS):
    env.Append(CPPDEFINES=["PIO_UNIT_TESTING"])
    print("[use_msys2_mingw] 检测到测试构建, 已定义 PIO_UNIT_TESTING")
