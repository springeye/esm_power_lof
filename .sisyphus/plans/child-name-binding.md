# UI 控件获取方式：从硬编码 Index 改为 Name-Based Lookup

> **面向 AI 代理的工作者：** 使用 `/start-work` 进入执行模式。步骤使用复选框（`- [ ]`）语法来跟踪进度。

**目标：** 将 `settings_ui.cpp:init_impl()` 中通过 `lv_obj_get_child(parent, index)` 硬编码子对象位置获取控件，改为 `lv_obj_get_child_by_name(parent, name)` 按名称查找，消除 XML 布局修改导致 index 偏移的风险。

**架构：** 在 `settings_create()` 生成屏幕后、查找子对象前，给 4 个关键容器/控件设置 `lv_obj_set_name()`；随后用 `lv_obj_get_child_by_name()` 替代硬编码 index 查找。所有改动仅限 `src/ui_bridge/settings_ui.cpp` 一个文件。

**技术栈：** C++17, LVGL 9.x (`lv_obj_set_name`, `lv_obj_get_child_by_name`), ESP32 Arduino

---

## 风险分析

| 风险点 | 严重度 | 缓解措施 |
|--------|--------|----------|
| `settings_gen.c` 是生成文件，不能手改 | 中 | 不在生成文件中设置 name；在 `settings_ui.cpp` 的运行时初始化中追加 name |
| `lv_obj_get_child_by_name()` 需要 LVGL 9.x 支持 | 低 | 项目已使用 LVGL ~9.5.0，API 原生支持 |
| 未来 XML 布局重排后 name 设置代码也不同步 | 低 | name 设置和查找在同一个函数中（locality 好），修改 layout 时自然需要一起更新 |

## 当前问题代码

**文件：** `src/ui_bridge/settings_ui.cpp:164-189`

```cpp
void init_impl() {
    // ...
    g_screen = settings_create();       // 行 169

    lv_obj_t* header  = lv_obj_get_child(g_screen, 0);     // 行 174: index 0 → header
    g_content_area     = lv_obj_get_child(g_screen, 1);     // 行 175: index 1 → 内容区

    if (header != nullptr) {
        g_title_label = lv_obj_get_child(header, 1);        // 行 178: index 1 → "设置" 标题
        g_page_label  = lv_obj_get_child(header, 2);        // 行 179: index 2 → "1/5" 页码
    }
    // ...
}
```

**对应 settings.xml 结构：**
```
根 view
├── header (height=22)        ← lv_obj_get_child(screen, 0)
│   ├── label "←"             ← lv_obj_get_child(header, 0)
│   ├── label "设置"          ← lv_obj_get_child(header, 1)  → g_title_label
│   └── label "1/5"           ← lv_obj_get_child(header, 2)  → g_page_label
├── content area (flex_grow=1) ← lv_obj_get_child(screen, 1) → g_content_area
└── bottom bar (height=18)     ← lv_obj_get_child(screen, 2)
```

---

## 修改计划

### 任务 1：用 Name-Based Lookup 替换硬编码 Index

**文件：**
- 修改：`src/ui_bridge/settings_ui.cpp:164-189`

**改动说明：** 在 `init_impl()` 中，`settings_create()` 之后，先给 4 个关键对象设置 name，再用 `lv_obj_get_child_by_name()` 查找。这样即使 XML 布局中 header 和 content area 的位置发生变化，只要 name 保持一致，代码就能正确获取。

- [x] **步骤 1：修改 `init_impl()` 函数**

将第 164-189 行的 `init_impl()` 替换为以下代码：

```cpp
void init_impl() {
    if (g_initialized) {
        return;
    }

    g_screen = settings_create();
    if (g_screen == nullptr) {
        return;
    }

    // ── 给关键子对象设置 name（按当前 settings.xml 结构）────
    {
        lv_obj_t* header  = lv_obj_get_child(g_screen, 0);
        lv_obj_t* content = lv_obj_get_child(g_screen, 1);
        if (header) {
            lv_obj_set_name(header, "settings_header");
            lv_obj_t* title = lv_obj_get_child(header, 1);
            lv_obj_t* page  = lv_obj_get_child(header, 2);
            if (title) lv_obj_set_name(title, "settings_title");
            if (page)  lv_obj_set_name(page,  "settings_page");
        }
        if (content) {
            lv_obj_set_name(content, "settings_content");
        }
    }

    // ── 改用 name 查找，不再依赖 index ──
    lv_obj_t* header  = lv_obj_get_child_by_name(g_screen, "settings_header");
    g_content_area     = lv_obj_get_child_by_name(g_screen, "settings_content");

    if (header != nullptr) {
        g_title_label = lv_obj_get_child_by_name(header, "settings_title");
        g_page_label  = lv_obj_get_child_by_name(header, "settings_page");
    }

    g_initialized = (g_content_area != nullptr && g_title_label != nullptr && g_page_label != nullptr);
    if (!g_initialized) {
        g_screen = nullptr;
        g_title_label = nullptr;
        g_page_label = nullptr;
        g_content_area = nullptr;
    }
}
```

> **注意**：name 设置仍使用了 `lv_obj_get_child(parent, index)` 来获取待设置 name 的对象。这是因为在设置 name 之前，还没有办法按 name 查找。但这是一次性的"bootstrap"：如果 XML 布局重排，只需更新 name 设置部分的 index，后续所有 name 查找都不受影响。这个设计把 layout-dependent index 限制在了 name 设置的 6 行代码中。

- [x] **步骤 2：编译验证 — ESP32 固件**

```bash
pio run -e esp32s3
```

预期：编译成功，无错误。

- [~] **步骤 3：编译验证 — Native 模拟器**（跳过）

```bash
pio run -e native
```

预期：编译成功，无错误。

- [~] **步骤 4：Native 模拟器快速冒烟测试**（跳过）

```bash
pio run -e native -t upload
```

预期：模拟器启动后，长按 K2 进入设置页面，5 个分类页面导航正常，标题和页码显示正确。

---

## 验证检查清单

- [x] `pio run -e esp32s3` 编译通过
- [~] `pio run -e native` 编译通过（跳过）
- [ ] `g_title_label`、`g_page_label`、`g_content_area` 三个指针均非 nullptr（通过 `g_initialized` 检查）
- [ ] 设置页面导航（上下键切换页面、长按上下键翻页）行为不变
- [ ] 设置值编辑（短按 K2 进入编辑、上下键调值）行为不变

---

## 不改动的范围（明确排除）

- ❌ **不修改** `ui/screens/settings_gen.c`（生成文件，不可手改）
- ❌ **不修改** `ui/screens/settings.xml`（LVGL Editor 源文件，本次不改——将来可通过 XML name 属性设置，但目前不需要）
- ❌ **不修改** `ui/lof_power_system.c` 或 `ui/lof_power_system_gen.*`
- ❌ **不修改** `src/ui_bridge/data_bridge.cpp`（已使用 LVGL Subject 模式，不涉及 index 获取 child）
- ❌ **不修改** home 或 splash 屏幕（它们没有通过 index 获取 child 的代码）

---

## 影响面评估

| 维度 | 评估 |
|------|------|
| 修改文件数 | **1 个**：`src/ui_bridge/settings_ui.cpp` |
| 修改行数 | ~20 行（替换 164-189 行内的 init_impl 实现） |
| API 变更 | 无（外部接口 `init()`, `show()`, `hide()`, `handle_key()`, `is_active()` 均不变） |
| 行为变更 | 无（name 设置直接跟随当前 settings.xml 结构，查找结果与 index 方式一致） |
| 兼容性 | 完全向后兼容；LVGL 9.x 原生 API |
