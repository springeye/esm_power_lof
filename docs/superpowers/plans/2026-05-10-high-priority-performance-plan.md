# 高优先级性能优化实现计划

> **面向 AI 代理的工作者：** 必需子技能：使用 superpowers:subagent-driven-development（推荐）或 superpowers:executing-plans 逐任务实现此计划。步骤使用复选框（`- [ ]`）语法来跟踪进度。

**目标：** 降低 LVGL 线程中的全量刷新和重建开销，减少功率历史数据的 RAM/CPU 消耗，并消除设置页保存路径中的重复持久化写入。

**架构：** 本计划只覆盖前期评估中“高优先级”的四个热点：图表刷新链路、功率历史缓冲、设置页重建、数据桥无差别刷新。整体策略是先用测试锁定行为，再拆掉全量扫描/拷贝/重建路径，最后做 UI 刷新去重与持久化收敛，确保显示逻辑保持现有功能不变。

**技术栈：** C++17、Arduino、FreeRTOS、LVGL 9.x、Unity（`src/test_main.cpp`）、PlatformIO

---

## 目标文件结构

### 核心修改文件
- `src/ui_bridge/power_history.h`
  - 定义新的历史查询接口，支持不拷贝或低拷贝读取。
- `src/ui_bridge/power_history.cpp`
  - 移除双份缓存或将其改为轻量窗口视图；保留现有环形缓冲外部语义。
- `src/ui_bridge/chart_view.cpp`
  - 将图表更新从“全量重灌”改为“增量/限量刷新”。
- `src/ui_bridge/data_bridge.cpp`
  - 将 200ms 定时器改成“值变化才更新”的 UI 刷新策略，并降低 uptime 等低频字段的刷新频率。
- `src/ui_bridge/settings_ui.cpp`
  - 将设置页从 `lv_obj_clean + 全量重建` 改为预创建固定行并复用。
- `src/app/config_manager.cpp`
  - 消除 setter 内立即落盘与 UI commit 再次落盘的重复写入路径。
- `src/app/config_manager.h`
  - 如有必要，增加显式“仅更新内存/延迟保存”的接口。

### 测试文件
- `src/test_main.cpp`
  - 扩展 power history、config manager、view/chart 行为相关测试。

### 验证入口
- `pio test -e test`
- `pio run -e esp32s3`
- （如实现过程中增加了额外测试 helper，也要纳入 `src/test_main.cpp`）

---

## 实施顺序

1. **先锁定数据层行为**：重构 `power_history` 前补测试，避免图表侧改动时行为漂移。
2. **再改图表刷新链路**：让 `chart_view` 不再每次从 3000 点历史中全量重建图表。
3. **再改 `data_bridge` 刷新策略**：压缩 UI 字符串格式化和 subject 更新频率。
4. **最后处理设置页重建与重复持久化**：这是 UI 结构调整，和前面链路解耦，单独验证更安全。

---

## 任务 1：锁定功率历史层行为并重构查询接口

**文件：**
- 修改：`src/ui_bridge/power_history.h`
- 修改：`src/ui_bridge/power_history.cpp`
- 修改：`src/test_main.cpp`

- [ ] **步骤 1：为现有历史查询行为补足回归测试**

在 `src/test_main.cpp` 增加以下测试场景：
- 多通道互不干扰（已存在，保留）
- 时间窗口只返回最近窗口数据（已存在，补上首尾值断言）
- 环形溢出后仍保持时间顺序
- 连续多次查询不会修改历史内容

建议新增测试名称：
```cpp
void test_power_history_time_window_bounds(void);
void test_power_history_query_is_stable(void);
void test_power_history_overflow_keeps_order(void);
```

- [ ] **步骤 2：运行测试验证基线通过**

运行：`pio test -e test`
预期：现有测试全部通过；新增测试如果只补断言、不依赖新接口，也应通过。

- [ ] **步骤 3：重构 `power_history` 返回方式，消除全量线性副本**

目标：去掉 `s_linear_temp[CHANNEL_COUNT][BUFFER_SIZE]` 这份 72KB 临时副本。

建议方向（二选一，优先 A）：

**A. 窗口采样输出接口（推荐）**
- 保留环形缓冲 `s_buffer`
- 新增一个“按目标点数提取窗口点”的接口，例如：
```cpp
struct PowerHistorySample {
    uint32_t timestamp_ms;
    float power_w;
};

uint32_t power_history_sample_window(
    uint8_t ch,
    uint32_t window_ms,
    uint16_t max_points,
    PowerHistorySample* out_points,
    uint16_t out_capacity);
```
- 由 `chart_view` 传入固定容量缓冲（例如 300 点），`power_history` 负责在一次遍历中完成窗口过滤 + 降采样。

**B. 双段只读视图接口**
- 如果坚持保留原始点数据，可把环形缓冲暴露为两段 slice，而不是线性拷贝。
- 但这会让 `chart_view` 复杂化，因此不推荐优先采用。

- [ ] **步骤 4：更新测试以覆盖新接口**

如果采用方案 A，在 `src/test_main.cpp` 新增：
```cpp
void test_power_history_sample_window_caps_points(void);
void test_power_history_sample_window_prefers_recent_range(void);
```

关键断言：
- 返回点数不超过 `max_points`
- 数据按时间递增
- 最近 60s 窗口只返回窗口内数据
- 无数据时返回 0

- [ ] **步骤 5：运行测试验证数据层通过**

运行：`pio test -e test`
预期：Power history 相关测试全部 PASS。

- [ ] **步骤 6：提交阶段性改动**

```bash
git add src/ui_bridge/power_history.h src/ui_bridge/power_history.cpp src/test_main.cpp
git commit -m "refactor: reduce power history copy overhead"
```

---

## 任务 2：将图表更新从全量重灌改为限量采样刷新

**文件：**
- 修改：`src/ui_bridge/chart_view.cpp`
- 修改：`src/ui_bridge/power_history.h`
- 修改：`src/ui_bridge/power_history.cpp`
- 测试：`src/test_main.cpp`

- [ ] **步骤 1：为图表数据规模建立明确上限**

在 `chart_view.cpp` 中引入一个显式常量，例如：
```cpp
static constexpr uint16_t CHART_POINT_COUNT = 300;
```

并统一替换当前硬编码的 `300` / `3000` 语义混用，确保：
- 屏幕图表最大显示点数固定
- 历史原始点数与显示点数分离

- [ ] **步骤 2：将 `update_chart_data()` 改为使用采样接口**

当前热点代码：
- `power_history_get_range(...)`
- `for` 循环逐点 `lv_chart_set_series_value_by_id(...)`
- `lv_chart_refresh(...)`

目标改法：
- 从 `power_history` 取最多 `CHART_POINT_COUNT` 个采样点
- `lv_chart_set_point_count()` 固定到采样点数
- 严禁再以 3000 原始点为输入做每次全量筛选

可接受实现：
```cpp
PowerHistorySample points[CHART_POINT_COUNT];
uint32_t count = power_history_sample_window(ch, window_ms, CHART_POINT_COUNT, points, CHART_POINT_COUNT);
```

- [ ] **步骤 3：保留现有视觉语义**

以下行为不得改变：
- CH1/CH2/CH3 颜色保持不变
- `chart_yaxis_mode == 1` 时仍使用 `design_power_w` 固定 Y 轴
- 无数据时仍显示空图

- [ ] **步骤 4：补一条最小回归测试说明**

由于 `chart_view` 依赖 LVGL 对象，当前 Unity 不适合直接做完整控件测试；因此在 `src/test_main.cpp` 中至少增加一条针对采样层的代理测试，证明：
- 3000 个点输入时，输出点数被限制在目标点数内
- 输出仍按时间递增

- [ ] **步骤 5：编译验证**

运行：`pio run -e esp32s3`
预期：编译通过，无接口签名错误。

- [ ] **步骤 6：提交阶段性改动**

```bash
git add src/ui_bridge/chart_view.cpp src/ui_bridge/power_history.h src/ui_bridge/power_history.cpp src/test_main.cpp
git commit -m "perf: limit chart refresh to sampled history points"
```

---

## 任务 3：压缩 `data_bridge` 的无差别 UI 刷新

**文件：**
- 修改：`src/ui_bridge/data_bridge.cpp`
- 如需要修改：`src/app/app_state.h`
- 测试：`src/test_main.cpp`（仅补数据层辅助测试，不强求 LVGL 控件测试）

- [ ] **步骤 1：把字段按刷新类型分组**

在 `data_bridge.cpp` 中把当前字段拆成三组：
- **高频但仅变化时更新**：温度、电压、电流、功率、RPM、fan percent
- **低频更新**：uptime、Wh
- **图表相关**：仅在图表视图激活时触发

- [ ] **步骤 2：引入上次显示值缓存**

在匿名命名空间中新增缓存结构，例如：
```cpp
struct UiSnapshot {
    float temp_c;
    uint16_t ch_mv[3];
    float ch_a[3];
    float total_w;
    uint32_t rpm;
    int32_t power_percent;
};
```

要求：
- 初始化为“不可能值”或单独 `bool initialized`
- 只有值变化时才调用 `lv_subject_copy_string` / `lv_subject_set_int`

- [ ] **步骤 3：降低 uptime 和 Wh 的刷新频率**

当前这两个字段每 200ms 都会 `snprintf`。
目标：
- uptime 改为每 1000ms 更新一次
- Wh 可与 uptime 同频或 1000ms 更新一次
- 不允许继续每 200ms 无条件刷新这两个字段

- [ ] **步骤 4：限制图表刷新触发条件**

当前代码只要在图表页，每 200ms 就 `chart_view_update(...)`。
目标：
- 仅当对应通道功率有变化，或到达一个单独的图表刷新节拍时才刷新
- 初始建议：图表刷新节拍单独限制为 500ms

- [ ] **步骤 5：编译和回归验证**

运行：
- `pio run -e esp32s3`
- `pio test -e test`

预期：编译通过，现有 power history / config / view manager 测试不回归。

- [ ] **步骤 6：提交阶段性改动**

```bash
git add src/ui_bridge/data_bridge.cpp src/app/app_state.h src/test_main.cpp
git commit -m "perf: avoid redundant UI subject updates"
```

---

## 任务 4：将设置页改为预创建复用，并收敛 NVS 保存路径

**文件：**
- 修改：`src/ui_bridge/settings_ui.cpp`
- 修改：`src/app/config_manager.cpp`
- 修改：`src/app/config_manager.h`
- 测试：`src/test_main.cpp`

- [ ] **步骤 1：先锁定 config manager 的保存语义**

当前已确认：
- `settings_ui::commit_edit_mode()` 会调用 `config_manager::save_to_nvs()`
- 至少部分 setter（如 `set_fan_temp_low`）内部又会 `save_to_nvs_locked()`

目标：统一成“编辑确认时只落盘一次”。

建议接口改法：
- setter 只更新内存，不直接写 NVS
- 显式持久化只通过 `save_to_nvs()` 完成

若担心影响其他调用方，可增加批量模式接口，但不要保留“同一次提交两次写盘”的路径。

- [ ] **步骤 2：为 config manager 补最小回归测试**

在 `src/test_main.cpp` 增加对以下行为的断言：
- `set_chart_yaxis_mode()` / `set_theme_mode()` / `set_design_power_w()` 在内存中立即可读
- `save_to_nvs()` 仍可单独调用
- setter 去掉自动落盘后，不影响 get/set 语义

- [ ] **步骤 3：把设置页改成固定行预创建**

当前热点在：`settings_ui.cpp:423-473`

目标结构：
- 首次显示时，按 `MAX_PAGE_ITEMS` 一次性创建固定数量 row
- `rebuild_page()` 不再 `lv_obj_clean()` 重建整页
- 切页时只：
  - 更新标签文本
  - 更新 value 文本
  - 隐藏超出 `item_count` 的多余 row
  - 更新 focus / editing 样式

建议新增函数：
```cpp
void ensure_rows_created();
void bind_page_to_rows();
void hide_unused_rows(size_t used_count);
```

- [ ] **步骤 4：保留现有交互语义**

以下行为必须保持不变：
- K1/K3 移动焦点
- K2 进入/确认编辑
- 编辑态闪烁逻辑仍可用
- 主题切换后立即应用
- 切页后焦点重置为第 1 项

- [ ] **步骤 5：编译与功能回归验证**

运行：
- `pio run -e esp32s3`
- `pio test -e test`

手动验证建议：
- 进入设置页
- 连续切换 6 个页面
- 观察切页是否仍有明显卡顿
- 修改一个设置项并确认后，重启设备验证值是否保持

- [ ] **步骤 6：提交阶段性改动**

```bash
git add src/ui_bridge/settings_ui.cpp src/app/config_manager.h src/app/config_manager.cpp src/test_main.cpp
git commit -m "perf: reuse settings rows and defer config persistence"
```

---

## 最终整体验证

**文件：**
- 修改：本计划涉及的所有文件

- [ ] **步骤 1：运行单元测试**

运行：`pio test -e test`
预期：全部 PASS。

- [ ] **步骤 2：运行固件编译**

运行：`pio run -e esp32s3`
预期：编译通过，无新增警告导致的构建失败。

- [ ] **步骤 3：进行最小人工冒烟检查**

检查点：
- Home 页面数值仍正常更新
- 图表页面能切换且有曲线显示
- 设置页面切换不卡死
- 修改设置后仍能保存并恢复

- [ ] **步骤 4：汇总性能结果**

在最终说明中记录：
- 是否去掉了 `s_linear_temp`
- 图表单次刷新最大点数
- UI 普通字段的刷新去重策略
- 设置页是否彻底移除 `lv_obj_clean + 全量重建`
- config 持久化是否变成单次提交单次写盘

- [ ] **步骤 5：最终提交**

```bash
git add src/ui_bridge/power_history.h src/ui_bridge/power_history.cpp src/ui_bridge/chart_view.cpp src/ui_bridge/data_bridge.cpp src/ui_bridge/settings_ui.cpp src/app/config_manager.h src/app/config_manager.cpp src/test_main.cpp
git commit -m "perf: optimize high-priority UI refresh paths"
```

---

## 自检结果

- 已覆盖四个高优先级项：图表刷新、历史缓存、设置页重建、无差别刷新
- 未引入“TODO/待定/后续实现”类占位符
- 所有任务都绑定到明确文件与验证命令
- 计划未扩散到中低优先级项（按键轮询、主题递归、power_task 周期），保持范围可控
