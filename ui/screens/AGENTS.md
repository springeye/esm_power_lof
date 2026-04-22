# SCREENS KNOWLEDGE BASE

## OVERVIEW
`screens/` 同时包含可编辑 XML 与由工具生成的 `*_gen.c/h`。

## WHERE TO LOOK
| Task | Location | Notes |
|---|---|---|
| 新增/修改屏幕结构 | `*.xml` | 这是屏幕定义的首选修改点 |
| 查看生成后的 LVGL 代码 | `*_gen.c` | 包含 `*_create()` 与样式/控件构建 |
| 查看导出函数声明 | `*_gen.h` | 与 `*_gen.c` 一一对应 |
| 目录约定说明 | `README.md` | 约定 `<screen>` 根标签 |

## CONVENTIONS
- 先改 XML，再重新生成 `*_gen.c/h`。
- 生成函数名应与屏幕语义一致（如 `home_create`、`splash_create`）。
- 生成文件默认 include `../lof_power_system.h`，保持该边界稳定。
- 与全局构建的关系由上层 `file_list_gen.cmake` 管理。

## ANTI-PATTERNS
- 在 `*_gen.c/h` 上做长期手工改动并期望可保留。
- 在本目录直接引入预览构建中间文件路径（`preview-build/**`）。
- 绕过 XML 直接复制粘贴修改大量生成控件代码。

## QUICK CHECKLIST
- 改动是否发生在 `*.xml`？
- 是否重新生成并同步了对应 `*_gen.c/h`？
- `*_create()` 返回对象与命名是否保持一致？
- 上层 `file_list_gen.cmake` 是否仍包含对应源文件？

## NOTES
- 本目录是 UI 结构变更的主要入口。
- 生成代码过长时，优先拆分 XML 结构而不是在 `*_gen.c` 手修。
