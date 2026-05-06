# settings.xml 内容区添加预览 item

> **面向 AI 代理的工作者：** 使用 `/start-work` 进入执行模式。

**目标：** 在 `ui/screens/settings.xml` 的内容区中添加 4 个预览用设置项行，方便在 LVGL Editor 中预览页面效果。运行时 `rebuild_page()` 中的 `lv_obj_clean(g_content_area)` 会清空所有子控件，不影响实际功能。

**技术栈：** LVGL Editor XML

---

## 修改计划

### 任务 1：在 settings.xml content area 内添加预览 item

**文件：** `ui/screens/settings.xml` 第 68-82 行

**改动说明：** 在内容区容器内、`<style name="style_no_pad" />` 之后插入 4 个预览行（风扇设置页的 4 个温度阈值），用清晰注释标记为预览专用。

**替换：** 将第 68-82 行替换为以下内容：

```xml
		<!-- 内容区（由 settings_ui.cpp 动态填充） -->
		<lv_obj
			width="100%"
			height="0%"
			style_flex_grow="1"
			style_layout="flex"
			flex_flow="column"
			style_bg_opa="0"
			style_border_width="0"
			style_pad_all="0"
			style_pad_row="4"
			style_pad_column="0"
		>
			<style name="style_no_pad" />

			<!-- ▼▼▼ 预览 item（仅 LVGL Editor 预览用，运行时 rebuild_page() 中 lv_obj_clean 清空）▼▼▼ -->

			<lv_obj
				width="100%" height="30"
				style_layout="flex" flex_flow="row"
				style_flex_main_place="space_between" style_flex_cross_place="center"
				style_bg_color="0x111820" style_bg_opa="255"
				style_border_color="0x1a2533" style_border_width="1"
				style_radius="3"
				style_pad_left="8" style_pad_right="8"
				style_pad_top="4" style_pad_bottom="4"
			>
				<lv_label text="低温阈值" style_text_font="hos_14" style_text_color="0xFFFFFF" />
				<lv_label text="35.0°C" style_text_font="hos_14" style_text_color="0xFFFFFF" />
			</lv_obj>

			<lv_obj
				width="100%" height="30"
				style_layout="flex" flex_flow="row"
				style_flex_main_place="space_between" style_flex_cross_place="center"
				style_bg_color="0x111820" style_bg_opa="255"
				style_border_color="0x1a2533" style_border_width="1"
				style_radius="3"
				style_pad_left="8" style_pad_right="8"
				style_pad_top="4" style_pad_bottom="4"
			>
				<lv_label text="中温阈值" style_text_font="hos_14" style_text_color="0xFFFFFF" />
				<lv_label text="45.0°C" style_text_font="hos_14" style_text_color="0xFFFFFF" />
			</lv_obj>

			<lv_obj
				width="100%" height="30"
				style_layout="flex" flex_flow="row"
				style_flex_main_place="space_between" style_flex_cross_place="center"
				style_bg_color="0x111820" style_bg_opa="255"
				style_border_color="0x1a2533" style_border_width="1"
				style_radius="3"
				style_pad_left="8" style_pad_right="8"
				style_pad_top="4" style_pad_bottom="4"
			>
				<lv_label text="高温阈值" style_text_font="hos_14" style_text_color="0xFFFFFF" />
				<lv_label text="55.0°C" style_text_font="hos_14" style_text_color="0xFFFFFF" />
			</lv_obj>

			<lv_obj
				width="100%" height="30"
				style_layout="flex" flex_flow="row"
				style_flex_main_place="space_between" style_flex_cross_place="center"
				style_bg_color="0x111820" style_bg_opa="255"
				style_border_color="0x1a2533" style_border_width="1"
				style_radius="3"
				style_pad_left="8" style_pad_right="8"
				style_pad_top="4" style_pad_bottom="4"
			>
				<lv_label text="强制阈值" style_text_font="hos_14" style_text_color="0xFFFFFF" />
				<lv_label text="60.0°C" style_text_font="hos_14" style_text_color="0xFFFFFF" />
			</lv_obj>

			<!-- ▲▲▲ 预览 item 结束 ▲▲▲ -->

		</lv_obj>
```

- [x] **步骤 1**：修改 `ui/screens/settings.xml` 入场区，添加 4 个预览行

- [ ] **步骤 2**：在 LVGL Editor 中重新打开并导出 `settings_gen.c/h`（如不需要预览可跳过此步）

- [x] **步骤 3**：编译验证 `pio run -e esp32s3`，确保新增 XML 内容不影响编译

---

## 不改动的范围

- ❌ 不修改 `settings_ui.cpp`
- ❌ 不修改 `settings_gen.c/h`（用户自行决定是否重新导出）

## 原理

运行时 `rebuild_page()` 第一行执行 `lv_obj_clean(g_content_area)`，清空 content area 的所有子控件（包括 XML 中写的预览 item），然后重新动态创建当前页面的设置项行。预览 item 仅在 LVGL Editor 中可见。
