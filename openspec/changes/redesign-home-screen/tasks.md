## 1. XML 布局重构

- [ ] 1.1 重构顶部状态栏：左侧状态点+状态文本行，右侧运行时间+温度，替换原有 `system_name` / `F` / `CAL` 布局
- [ ] 1.2 保留功率主区（`device_current_power` 大字），增加"当前功率"标签行和设计功率徽标占位
- [ ] 1.3 新增功率占比进度条区域：左标签+右百分比+`lv_bar`，替换原有功率条行
- [ ] 1.4 重构统计行：从两列（uptime + wh）改为三列等宽（运行时间 / 总电能 / 效率）
- [ ] 1.5 修复 CH3 通道绑定：将 `ch2_voltage` / `ch2_current` / `ch2_pwer` 改为 `ch3_voltage` / `ch3_current` / `ch3_pwer`

## 2. 通道卡片重构

- [ ] 2.1 将三个通道容器从 `height="0%"` + `flex_grow` 改为 `style_flex_grow="1"` + 确定性高度分配
- [ ] 2.2 每个通道卡片内部重构为：标签头（CH1/CH2/CH3 替代"通道N"）+ 底边框色区分 + 电压/电流双列行 + 功率大字
- [ ] 2.3 CH1 标签底边框设为绿色 `0x00e68a`，CH2 为琥珀色 `0xffb020`，CH3 为蓝色 `0x3d9eff`
- [ ] 2.4 电压/电流行改为左右布局：左标签（`电压`/`电流`）+ 右数值（`ch*_voltage`/`ch*_current`）

## 3. 验证与生成

- [ ] 3.1 在 LVGL Editor 中重新生成 `home_gen.c` / `home_gen.h`
- [ ] 3.2 验证生成的 `home_create()` 函数签名不变
- [ ] 3.3 确认 `lof_power_system.c` 中 `home_create()` 调用无需修改