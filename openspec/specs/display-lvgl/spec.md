## ADDED Requirements

### Requirement: ST7789P3 显示驱动接入

The system SHALL 通过 TFT_eSPI 库驱动 ST7789P3 240×280 4-SPI 屏幕，正确处理 240×280 模组的列/行偏移，画面方向、颜色不出现偏移、错位或撕裂。

#### Scenario: 上电初始化
- **WHEN** 设备上电
- **THEN** 屏幕在 500ms 内点亮，背景色填充无偏移，且分辨率为 240×280

#### Scenario: 全屏纯色填充
- **WHEN** UI 调用全屏填充红/绿/蓝
- **THEN** 整屏被对应纯色覆盖，无残影、无错位、无颜色翻转

### Requirement: SPI 时序与背光

The system SHALL 使用 VSPI（SCK=GPIO18，MOSI=GPIO23，CS=GPIO5，DC=GPIO2，RES=GPIO4），SPI 速率不低于 40MHz，背光通过 GPIO16 控制并支持 PWM 调光。

#### Scenario: SPI 频率
- **WHEN** 检查 TFT_eSPI 编译配置
- **THEN** `SPI_FREQUENCY` ≥ 40000000

#### Scenario: 背光开关
- **WHEN** 软件调用背光关闭
- **THEN** GPIO16 输出 0，屏幕熄灭；调用打开后屏幕恢复显示

### Requirement: LVGL 9.x 移植

The system SHALL 集成 LVGL v9.2.x，使用双缓冲（每缓冲 240×28 像素，RGB565）、PARTIAL 渲染模式，由 `esp_timer` 1ms 周期推进 `lv_tick_inc`，并由专用 FreeRTOS 任务以 5ms 周期调用 `lv_timer_handler`。

#### Scenario: 缓冲区配置
- **WHEN** 检查 LVGL display 注册代码
- **THEN** 注册的两个 draw buffer 大小均为 `240*28*sizeof(lv_color_t)`，渲染模式为 `LV_DISPLAY_RENDER_MODE_PARTIAL`

#### Scenario: 刷新 60 FPS 目标
- **WHEN** UI 任务正常运行
- **THEN** `lv_timer_handler` 调用周期不超过 10ms，且 LVGL benchmark 的"empty screen"测试 FPS ≥ 30

### Requirement: 抗锯齿英文字体

The system SHALL 启用 LVGL 内置 Montserrat 16 与 Montserrat 24 字体（4bpp 抗锯齿），不启用任何中文字体，`LV_COLOR_DEPTH` 为 16 且 `LV_COLOR_16_SWAP` 为 1。

#### Scenario: 字体配置
- **WHEN** 检查 `include/lv_conf.h`
- **THEN** `LV_FONT_MONTSERRAT_16` 与 `LV_FONT_MONTSERRAT_24` 均为 1，所有 `LV_FONT_*_CHINESE*` 与额外中文字模均为 0

#### Scenario: 文字渲染清晰
- **WHEN** 在 UI 上显示英文/数字字符串（例如 "CPU 42.5°C"）
- **THEN** 边缘有抗锯齿过渡，无明显锯齿与色边
