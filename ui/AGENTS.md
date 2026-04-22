# PROJECT KNOWLEDGE BASE

**Generated:** 2026-04-23 00:18:36 +08:00
**Commit:** cb6bef9
**Branch:** master

## OVERVIEW
该目录是 LVGL Editor 导出的 UI 子工程，核心是 C/CMake + Emscripten 预览产物。
这里不是主固件源码目录；主固件与测试在上层仓库路径。

## STRUCTURE
```text
ui/
├── CMakeLists.txt                 # UI 预览构建入口
├── file_list_gen.cmake            # 生成：编译源清单
├── component_lib_list_gen.cmake   # 生成：组件库清单（可为空）
├── lof_power_system.c/.h          # 手写扩展入口
├── lof_power_system_gen.c/.h      # 生成初始化与聚合头
├── screens/                       # screen XML 与 *_gen.c/h
├── preview-build/                 # 生成：CMake 中间产物
└── preview-bin/                   # 生成：预览 runtime.js/wasm
```

## WHERE TO LOOK
| Task | Location | Notes |
|---|---|---|
| UI 构建入口 | `CMakeLists.txt` | include 生成清单并创建 `lib-ui` |
| 编译源来源 | `file_list_gen.cmake` | `LV_EDITOR_PROJECT_SOURCES` 单一来源 |
| 手写初始化钩子 | `lof_power_system.c` | `lof_power_system_init()` 调用 `*_init_gen()` |
| 生成初始化逻辑 | `lof_power_system_gen.c` | 资源/注册逻辑在此 |
| 屏幕源码描述 | `screens/*.xml` | 人工可编辑输入 |
| 生成屏幕代码 | `screens/*_gen.c/h` | 由工具生成，不应长期手改 |
| 预览导出符号 | `preview-bin/build.log` | 含 `_lof_power_system_init` |

## CODE MAP
| Symbol | Type | Location | Role |
|---|---|---|---|
| `lof_power_system_init` | function | `lof_power_system.c` | 手写入口，桥接生成初始化 |
| `lof_power_system_init_gen` | function | `lof_power_system_gen.c` | 生成初始化主体 |
| `home_create` | function | `screens/home_gen.c` | Home 屏幕生成函数 |
| `splash_create` | function | `screens/splash_gen.c` | Splash 屏幕生成函数 |
| `LV_EDITOR_PROJECT_SOURCES` | CMake list | `file_list_gen.cmake` | 编译单元汇总 |

## CONVENTIONS
- 优先修改 `screens/*.xml` 与手写入口文件；再通过工具重新生成 `*_gen.*`。
- `file_list_gen.cmake` 与 `component_lib_list_gen.cmake` 视为生成文件。
- `preview-build/` 与 `preview-bin/` 视为构建/预览产物目录。
- `lof_power_system.c` 是手写扩展点；生成逻辑在 `*_gen.c`。

## ANTI-PATTERNS (THIS PROJECT)
- 直接长期手改 `*_gen.c` / `*_gen.h`。
- 手改 `preview-build/**` 中的 `.o/.d/.make/.rsp/link.txt`。
- 把 `preview-bin/lved-runtime.js` 当作业务源码维护。
- 假设本目录存在 npm/ts 测试与构建入口。

## UNIQUE STYLES
- 生成文件普遍采用 `*_gen` 命名，手写入口不带 `_gen`。
- 生成屏幕函数以 `*_create()` 暴露，命名与 XML 屏幕对应。
- 样式对象常见 `static ... + style_inited` 的一次性初始化模式。

## COMMANDS
```powershell
# 查看 UI 预览构建入口与源清单
Get-Content CMakeLists.txt
Get-Content file_list_gen.cmake

# 在已配置 emsdk/cmake 的环境里构建预览工程
cmake --build preview-build

# 检查预览导出符号
Get-Content preview-bin/build.log
```

## NOTES
- `.gitignore` 当前仅忽略 `preview-build`；`preview-bin` 是否忽略由仓库策略决定。
- 本目录未发现独立测试配置；测试主入口在上层仓库 `test/native`。
- 修改生成边界规则时，优先更新导出/生成流程，而非补丁式改生成物。
