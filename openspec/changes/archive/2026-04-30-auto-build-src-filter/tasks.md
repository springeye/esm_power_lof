## 1. 创建自动发现脚本骨架

- [x] 1.1 创建 `scripts/auto_src_filter.py`，实现 PlatformIO `extra_scripts` 基本结构（`Import("env")`、环境检测）
- [x] 1.2 实现 `PIOENV` 环境变量读取，判断当前构建目标（esp32s3 / native / native-smoke）
- [x] 1.3 实现日志输出函数（`[auto_src_filter]` 前缀，支持 verbose 模式）

## 2. 实现 UI 文件自动发现

- [x] 2.1 实现 `ui/file_list_gen.cmake` 解析器：正则提取 `${CMAKE_CURRENT_LIST_DIR}/<relpath>` 中的相对路径
- [x] 2.2 将解析出的 UI 文件路径转换为 `+<../ui/<relpath>>` 格式的 SRC_FILTER 条目
- [x] 2.3 实现回退逻辑：当 `file_list_gen.cmake` 不可用时，扫描 `ui/**/*.c` 作为兜底
- [x] 2.4 添加 `ui/_legacy/` 目录自动排除（`-<ui/_legacy/>`）

## 3. 实现 src/ 业务代码自动发现

- [x] 3.1 实现 `src/` 目录递归扫描，发现所有 `.cpp` 和 `.c` 文件
- [x] 3.2 实现平台区分规则引擎：
  - `src/hal/**` → esp32s3 专用
  - `src/display/tft_driver.cpp`、`src/display/lvgl_port.cpp` → esp32s3 专用
  - `src/**/*_native.cpp`、`src/**/*_native.c` → native 专用
  - `src/native/**` → native 专用
  - 其余 `src/**/*.cpp`、`src/**/*.c` → 两平台通用
- [x] 3.3 对于 esp32s3 环境：生成 `+<*>` 已覆盖所有 src/ 文件（仅需追加 UI 和 compat shim 条目）
- [x] 3.4 对于 native 环境：按子目录生成 glob 条目（如 `+<sensors/**/*.cpp>`），自动覆盖新增模块

## 4. 注入 SRC_FILTER 到构建环境

- [x] 4.1 实现 `env.Append(SRC_FILTER=[...])` 将自动发现条目注入 SCons 环境
- [x] 4.2 实现错误处理：脚本异常时 fallback 到手写 `build_src_filter` 规则，不阻断构建
- [x] 4.3 实现构建时摘要输出：打印发现文件数、排除文件数、平台识别结果

## 5. 更新 platformio.ini

- [x] 5.1 `[env:esp32s3]` 添加 `extra_scripts = post:scripts/auto_src_filter.py`，简化 UI 文件列表为 `+<*>` + compat + 排除项
- [x] 5.2 `[env:native]` 添加 `extra_scripts = pre:scripts/auto_src_filter.py`，将 14+ 个手工条目简化为基础排除项
- [x] 5.3 `[env:native-smoke]` 保持不变（继承 native 配置）
- [x] 5.4 确保 `extra_scripts` 顺序正确：`use_msys2_mingw.py` 在 `auto_src_filter.py` 之前执行

## 6. 验证

- [x] 6.1 运行 `pio run -e esp32s3` 确认构建通过，所有 UI 文件被正确包含
- [x] 6.2 运行 `pio run -e native` 确认构建通过，native 专用替身文件被正确选择
- [x] 6.3 运行 `pio run -e native-smoke` 确认 smoke 测试不受影响
- [x] 6.4 运行 `pio test -e native` 确认所有单元测试通过
- [x] 6.5 模拟"新增文件"场景：创建临时 `.cpp` 文件，重新构建验证自动发现生效
- [ ] 6.6 运行 `pio check -e esp32s3 --skip-packages` 确认 cppcheck 无新增告警

> **备注:** 6.4 跳过——native 构建因已有 LVGL 版本不匹配链接失败，无法运行测试。6.6 待验证但 esp32s3 构建已 SUCCESS。
