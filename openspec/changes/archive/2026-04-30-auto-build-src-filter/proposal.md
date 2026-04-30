## Why

当前 `platformio.ini` 中 `build_src_filter` 完全手工维护——每次新增 `.cpp`/`.c` 源文件，或 LVGL Editor 导出新页面，都必须手动编辑 `platformio.ini` 的每个 `[env:*]` 段。native 环境尤其严重：14+ 个源文件逐个列举，新增模块极易遗漏，导致"编译通过但实际没有链接新代码"的隐蔽 bug。此外 `ui/file_list_gen.cmake` 已是 UI 源文件的权威清单，却未被 PlatformIO 构建流程利用，形成了两套需要独立维护的文件列表。

## What Changes

- **新增** `scripts/auto_src_filter.py`：PlatformIO `extra_scripts` 钩子脚本，在构建前自动扫描 `src/` 和 `ui/` 目录，动态生成 `build_src_filter` 条目
- **新增** `scripts/src_filter_config.yaml`（可选）：约定配置文件，定义扫描规则（目录 → 平台映射、排除模式）
- **修改** `platformio.ini`：所有 `[env:*]` 段用 `extra_scripts` 引用新脚本，大幅简化手写 `build_src_filter` 内容
- **约定建立**：
  - `src/` 下所有 `.c`/`.cpp` 自动纳入 `esp32s3` 构建（`+<*>` 已覆盖，无需改动）
  - `*_native.cpp` / `*_native.c` 文件仅在 `native` 平台编译
  - `ui/` 下所有 `*_gen.c`、`*_data.c`、`lof_power_system.c` 自动纳入构建
  - `ui/_legacy/` 和 `src/ui/_legacy/` 自动排除

## Capabilities

### New Capabilities
- `auto-src-filter`: Python 脚本自动发现源文件并注入 `build_src_filter`，开发者新增文件后无需手动编辑 `platformio.ini`
- `build-script-hook`: PlatformIO `extra_scripts` 集成机制，脚本在构建前透明运行，不影响现有开发工作流

### Modified Capabilities
<!-- 无现有 spec 需要修改 -->

## Impact

- **Affected code**: `platformio.ini`（三处 env 段简化）、`scripts/`（新增 2 文件）
- **Dependencies**: 无新增外部依赖（仅 Python 标准库，PlatformIO 自带 Python 运行时）
- **Breaking changes**: 无——现有 `build_src_filter` 的行为完全保留，脚本仅补充而非替换
- **Risks**: 文件命名需遵循约定（如 native 替身用 `*_native.*` 后缀），否则可能被错误平台编译
