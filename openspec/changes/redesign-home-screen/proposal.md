## Why

Current `home.xml` 屏幕布局与 `power_monitor_mockup.html` 效果图设计脱节：现有布局缺少状态指示灯、设计功率标签、进度条占比区域、三个通道的标签头和分隔线；通道区域的 `height="0%"` + `style_flex_grow="1"` 导致在 LVGL 中高度计算不稳定；通道3的数据绑定错误地引用了 `ch2_*` 而非 `ch3_*`。需要依照效果图重新设计 home 屏幕布局，使其在 240×280 屏幕上的信息密度、视觉层次和交互一致性达到设计预期。

## What Changes

- 重构 `home.xml` 顶部状态栏：左侧状态指示点 + 状态文本，右侧运行时间 + 温度
- 重构功率主区：保留 `device_current_power` 大字号绑定，新增 `system_state` 状态色彩联动设计说明
- 新增进度条区域：功率占比标签 + 百分比数字 + `lv_bar` 进度条（复用已有 `device_power_percent` 绑定）
- 新增三列统计行：运行时间、总电能、效率，替代原有两列（uptime + wh）布局
- 重构三列通道区域：将通道标签头从纯文本 `"通道N"` 改为 `"CH1"/"CH2"/"CH3"` 并加彩色底边框样式；每个通道卡片内数据项改为电压/电流左右对齐 + 功率大字居中
- 修复 CH3 通道绑定 `ch2_*` → `ch3_*` 的错误
- 通道容器从 `height="0%"` + `flex_grow` 改为 `style_flex_grow="1"` 正值 + 明确高度分配

## Capabilities

### New Capabilities
- `home-screen-layout`: Home 屏幕 240×280 布局重构，包含状态栏、功率主区、进度条、统计行、三列通道卡片的完整布局规范

### Modified Capabilities
(无现有 spec 需要修改)

## Impact

- `ui/screens/home.xml` — 主要修改文件，重构整个屏幕布局
- `ui/screens/home_gen.c` / `ui/screens/home_gen.h` — 重新生成，需通过工具流程生成
- `ui/lof_power_system.c` — `home_create()` 签名不变，但新增绑定变量需在 data_bridge 中提供数据源
- `src/ui_bridge/data_bridge.cpp` — 可能需要对接新的绑定变量（如 `device_power_percent_txt`、`system_name` 等已有变量的确认）