# Learnings - OTA 进度覆盖显示

## 2026-05-11

### `hos_bold_big` 类型注意事项
- `lof_power_system_gen.h` 中声明为 `extern lv_font_t * hos_bold_big;` —— 本身即指针
- `lv_obj_set_style_text_font()` 接受 `const lv_font_t*`
- 传递时应直接传 `hos_bold_big`，不要 `&hos_bold_big`（后者变成 `lv_font_t**`）
- 参考：`home_gen.c` 中用法为 `lv_obj_set_style_text_font(lv_label_6, hos_bold_big, 0);`

### PlatformIO + LSP
- `lvgl/lvgl.h` not found 是 PlatformIO 项目的预存问题，不影响实际编译
- 构建时由 `platformio.ini` 中的 `build_flags` 和 `lib_deps` 提供 include 路径
