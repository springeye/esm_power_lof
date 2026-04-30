## Context

`platformio.ini` 中三处 `[env:*]` 段的 `build_src_filter` 均由开发者手工维护。`[env:native]` 最严重——14+ 条目逐一列举源文件（`+<sensors/ntc/ntc.cpp>`、`+<fan/fan_curve.cpp>` 等）。每次新增模块或 LVGL Editor 导出新屏幕，都必须手动编辑 `platformio.ini`，极易遗漏。

现有资产可复用：
- `ui/file_list_gen.cmake`：LVGL Editor 生成的 UI 源文件权威清单（`LV_EDITOR_PROJECT_SOURCES`），包含所有字体、屏幕和入口文件
- `scripts/use_msys2_mingw.py`：已有的 `extra_scripts` 钩子模板，展示了如何通过 `Import("env")` 操作 SCons 构建环境

约束条件：
- 不能破坏 `esp32s3` 的 `+<*>` 全量包含策略
- 必须保留 `ui/_legacy/` 和 `src/ui/_legacy/` 的排除
- native 平台需要区分硬件相关代码（ESP32 专用）和可移植代码
- 脚本只能依赖 Python 标准库（PlatformIO 自带 CPython）

## Goals / Non-Goals

**Goals:**
- 开发者新增 `.cpp`/`.c` 源文件后，所有 `[env:*]` 构建**自动**包含该文件，无需手动编辑 `platformio.ini`
- LVGL Editor 导出新屏幕/字体后，构建自动识别新文件（利用 `file_list_gen.cmake`）
- `platformio.ini` 中手写 `build_src_filter` 条目大幅减少，仅保留必要的排除规则
- 支持通过目录级约定区分平台专属文件（`*_native.*` 后缀、`hal/` 目录排除等）

**Non-Goals:**
- 不修改 `platformio.ini` 文件本身（脚本在构建时动态注入过滤规则，不写回 INI）
- 不移除 `platformio.ini` 中现有的所有 `build_src_filter`（保留 `+<*>` 等基础规则作为兜底）
- 不支持运行时动态加载/插件化源文件（仅解决编译期源文件发现）
- 不改变现有测试构建流程（`native-smoke` 保持不变）

## Decisions

### 决策 1: 使用 PlatformIO `extra_scripts` 钩子（而非独立脚本）

**选择**: `scripts/auto_src_filter.py` 作为 `pre:extra_scripts`，在 SCons 读取 `platformio.ini` 后、开始编译前运行。

**替代方案**:
- ❌ 独立脚本修改 `platformio.ini`：需要处理 INI 格式解析，容易引入格式错误，且每次需要手动运行
- ❌ SCons `AddPostAction`：时序太晚，此时源文件列表已确定
- ✅ `extra_scripts`：PlatformIO 原生支持，可访问 `env` 对象直接注入 `SRC_FILTER`

### 决策 2: 以 `ui/file_list_gen.cmake` 为 UI 文件唯一来源

**选择**: 脚本解析 `file_list_gen.cmake` 中的 `LV_EDITOR_PROJECT_SOURCES` 列表，转换为 `+<../ui/...>` 格式注入构建。

**替代方案**:
- ❌ 直接扫描 `ui/**/*.c`：会包含不参与构建的辅助文件
- ❌ 在 `platformio.ini` 中写 glob：PlatformIO 不支持跨 `+<*>` 和 `-<...>` 的复杂 glob 组合
- ✅ 解析权威清单：与 CMake 预览构建保持一致，单一数据源

**解析策略**: 用正则提取 `${CMAKE_CURRENT_LIST_DIR}/...` 后的相对路径，而非依赖 CMake 解释器。

### 决策 3: 平台区分使用约定而非显式配置

**选择**: 通过文件命名约定和目录规则自动判断文件归属平台。

| 规则 | esp32s3 | native | 说明 |
|------|:-------:|:------:|------|
| `src/**/*.cpp` (不含 hal/ 和特定例外) | ✅ (via `+<*>`) | ✅ (glob) | 通用业务逻辑 |
| `src/hal/**/*.cpp` | ✅ | ❌ | 硬件抽象层，仅 ESP32 |
| `src/display/tft_driver.cpp` | ✅ | ❌ | 有 native 替身 |
| `src/display/lvgl_port.cpp` | ✅ | ❌ | 有 native 替身 |
| `src/display/*_native.cpp` | ❌ | ✅ | Native 替身文件 |
| `src/native/**/*.cpp` | ❌ | ✅ | Native 专属入口 |
| `ui/` 下所有源文件 | ✅ | ✅ | UI 层，跨平台 |

**替代方案**:
- ❌ 显式配置文件（YAML/TOML）：增加维护负担，且大部分规则已隐含在目录命名中
- ❌ 全部文件两平台编译：导致链接错误（Arduino API 在 native 不存在）

### 决策 4: 通过 `env.Append(SRC_FILTER=...)` 注入规则

**选择**: 使用 SCons 环境变量的追加操作，而非替换 `build_src_filter`。

**理由**: `platformio.ini` 中的手写规则作为基线保留，脚本仅追加自动发现的文件。这样即使脚本不运行，构建也不会完全失败——手写规则提供兜底。

### 决策 5: 脚本无外部依赖

**选择**: 仅使用 Python 标准库（`os`, `re`, `glob`, `pathlib`），不引入 PyYAML 等第三方包。

**理由**: PlatformIO 自带 CPython 运行时，但不保证第三方包可用。`use_msys2_mingw.py` 已证明此模式可行。

## Risks / Trade-offs

| 风险 | 缓解措施 |
|------|---------|
| **命名约定被违反**: 开发者在 `hal/` 下创建可移植代码，或在非 native 目录下创建 `*_native.cpp` | 脚本输出 WARNING 日志；代码审查中检查 |
| **`file_list_gen.cmake` 格式变化**: LVGL Editor 更新后生成格式改变，正则解析失败 | 脚本解析失败时回退到目录扫描兜底；输出 ERROR 日志提示手动更新 |
| **SCons 缓存失效**: 新增文件后增量构建可能不重新评估 `SRC_FILTER` | 脚本打印当前注入的文件列表供调试；必要时 `pio run -t clean` |
| **脚本性能**: 每次构建都扫描目录和解析 cmake | 扫描操作 < 100ms，可忽略不计 |
| **native 遗漏**: 新模块目录未被规则覆盖 | 使用 `sensors/**/*` 等递归 glob，新子目录自动覆盖 |

## Open Questions

1. **是否需要配置文件**？当前设计基于约定（无配置文件）。如果未来平台差异变复杂（3+ 平台、更细粒度的排除），可引入 `src_filter_config.yaml`。建议 MVP 先走纯约定。
2. **`native-smoke` 是否需要自动发现**？当前仅包含 `test_main.cpp`，建议保持不变（smoke 测试应最小化）。
