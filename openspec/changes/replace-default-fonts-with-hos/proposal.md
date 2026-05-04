## Why

当前项目使用LVGL内置的Montserrat字体作为默认字体（LV_FONT_DEFAULT设置为&lv_font_montserrat_14），但该字体没有抗锯齿效果，在240x280像素、0.11655mm像素间距的屏幕上显示效果不够高清。项目已经引入了HarmonyOS Sans（hos）字体，但默认字体配置仍指向Montserrat，需要统一替换为hos字体以获得更好的显示效果。

## What Changes

- 修改`include/lv_conf.h`中的`LV_FONT_DEFAULT`宏，从`&lv_font_montserrat_14`改为`hos_14`（或`&hos_14_data`）
- 关闭不需要的Montserrat字体宏（LV_FONT_MONTSERRAT_14/16/20/28等），减小固件体积
- 确保所有XML文件（ui/screens/*.xml）中的字体引用使用hos字体而非默认字体
- 验证字体大小在240x280屏幕上的显示效果，必要时调整字体size参数

## Capabilities

### New Capabilities
- `hos-default-font`: 将LVGL默认字体从Montserrat替换为HarmonyOS Sans（hos），确保高清抗锯齿显示效果

### Modified Capabilities
<!-- 无需修改现有capability的规格说明 -->

## Impact

- **代码文件**: `include/lv_conf.h`（修改LV_FONT_DEFAULT和字体宏）
- **UI文件**: `ui/globals.xml`（确认字体映射正确）、`ui/screens/*.xml`（验证字体引用）
- **构建配置**: `platformio.ini`（可能需要调整字体相关build_flags）
- **依赖**: LVGL 9.x字体系统、HarmonyOS Sans字体文件（已存在于ui/fonts/）
- **显示效果**: 所有UI文本将使用hos字体渲染，获得抗锯齿高清效果
- **固件体积**: 关闭未使用的Montserrat字体可减小固件大小
