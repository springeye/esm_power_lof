## Context

ESP32 智能风扇控制器的 Home 屏幕当前使用 `home.xml` 定义布局，由 LVGL Editor 生成 `home_gen.c/h`。现有布局存在以下问题：

1. 缺少状态指示灯和运行时间显示区域
2. 缺少设计功率标签（`⚡750W`）与功率占比进度条区域
3. 统计行只有两列（uptime + Wh），效果图设计为三列（运行时间 + 总电能 + 效率）
4. 三个通道卡片的 `height="0%"` + `flex_grow` 在 LVGL 中高度计算不稳定
5. CH3 通道的绑定错误引用了 `ch2_*` 变量
6. 通道标签使用中文"通道N"，效果图为"CH1/CH2/CH3"

参考效果图 `ui/power_monitor_mockup.html` 定义了目标视觉设计：
- 顶部状态栏（状态点 + 文本 | 运行时间 + 温度）
- 功率数字主区（48px 大字 + 设计功率徽标）
- 功率占比进度条
- 三列统计行
- 分隔线
- 三列通道卡片（CH 标签 + 电压/电流行 + 功率大字）

屏幕分辨率 240×280，LVGL 9.x flex 布局。

Affected files:
- `ui/screens/home.xml` (主要修改)
- `ui/screens/home_gen.c/h` (生成文件，需重新生成)
- `ui/lof_power_system.c` (可能与新增绑定交互)

## Goals / Non-Goals

**Goals:**
- 重构 `home.xml` 布局使其与效果图一致
- 修复 CH3 绑定错误
- 确保三列通道区域在 LVGL flex 布局中稳定填满剩余高度
- 保持所有现有数据绑定变量名不变（`device_current_power`, `device_power_percent`, `device_power_percent_txt`, `device_temp`, `system_name`, `system_state`, `uptime`, `wh`, `device_power`, `ch1_voltage`, `ch1_current`, `ch1_pwer`, `ch2_*`, `ch3_*`）

**Non-Goals:**
- 不修改 data_bridge 层或新增绑定变量（本次仅布局重构）
- 不引入新的自定义字体或图片资源
- 不修改 LVGL Editor 生成的代码结构
- 不处理状态色彩联动逻辑（运行中/停止），该逻辑在 C 代码层实现

## Decisions

### D1: 布局策略 — flex column 百分比 + grow 混合

**选择**: 使用 flex column 布局，顶部区域用固定百分比高度，通道区域用 `flex_grow=1` 填满剩余空间。

**替代方案**: 全部用固定像素高度（LVGL不支持px单位在XML中直接使用）或 全部用百分比。

**理由**: 效果图的 flex 布局在 CSS 中 `flex:1` 自然填满，LVGL 的 `style_flex_grow` 行为一致。顶部信息区高度固定（百分比或内容自适应），通道区弹性占剩余空间。与效果图的 CSS `flex: 1` 策略对应。

### D2: 进度条实现 — 复用 lv_bar + label 组合

**选择**: 保留现有 `lv_bar` 组件绑定 `device_power_percent`，外层 flex row 包含 bar 和百分比文本。

**理由**: 现有 `home.xml` 已有 `lv_bar` + `device_power_percent` 绑定，只需调整外层布局和增加标签文字。无需新增组件。

### D3: 通道卡片结构 — 标签头 + 数据行 + 功率大字

**选择**: 每个通道卡片内部结构为：顶部 CH 标签（带底边框色区分）→ 电压行（左右对齐）→ 电流行（左右对齐）→ 功率大字（居中/右对齐，变色）。

**理由**: 与效果图 `.channel-card` 结构一致。标签底边框通过 `style_border_side` + `style_border_color` 在 XML 中实现色区分。功率值大字可通过字体大小和颜色差异化。

### D4: 统计行扩展 — 三列替代两列

**选择**: 将 uptime + wh 两列改为三列：运行时间 | 总电能 | 效率，使用 `width="32%"` 近似三等分。

**理由**: 效果图设计要求三列统计数据。效率百分比是新增展示项，绑定变量需确认（可能复用现有 `device_power_percent_txt` 或在 data_bridge 新增 `efficiency` 变量——但本次不改动 data_bridge，用 `device_power_percent_txt` 暂时占位并在注释中标注）。

### D5: 状态栏实现

**选择**: 使用 `system_state` 绑定显示运行状态文本，状态指示点用 `lv_label` 的 Unicode 字符 `◉` 模拟（与效果图一致），不引入 LED 控件。

**理由**: LVGL XML 不支持动态样式切换（运行时变色），状态点和颜色联动需在 C 代码层实现。XML 仅定布局和静态样式，动态样式由 `lof_power_system.c` 的初始化回调处理。

## Risks / Trade-offs

- **[LVGL flex 高度稳定性]** → `flex_grow=1` 在 LVGL 9.x 中对嵌套 flex 容器的表现可能与 CSS 不同，需实际测试验证。Mitigation: 如 grow 不生效，改为显式百分比高度分配。
- **[数据绑定缺失]** → 效率百分比、设计功率等新展示项可能缺少对应绑定变量。Mitigation: 使用已有变量占位，在 XML 注释中标注需要后续 data_bridge 扩展。
- **[CH3 数据错误]** → 现有 home.xml 中 CH3 绑定 `ch2_*` 已确认是 bug，本次一并修复为 `ch3_*`。