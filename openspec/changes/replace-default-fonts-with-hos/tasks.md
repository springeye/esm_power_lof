## 1. 修改LVGL默认字体配置

- [ ] 1.1 修改`include/lv_conf.h`中的`LV_FONT_DEFAULT`宏，从`&lv_font_montserrat_14`改为`hos_14`
- [ ] 1.2 关闭未使用的Montserrat字体宏：设置`LV_FONT_MONTSERRAT_14`、`LV_FONT_MONTSERRAT_16`、`LV_FONT_MONTSERRAT_20`、`LV_FONT_MONTSERRAT_28`为0

## 2. 验证字体引用

- [ ] 2.1 全局搜索项目中是否有其他代码引用Montserrat字体（grep搜索"montserrat"）
- [ ] 2.2 检查`ui/globals.xml`中的字体映射是否正确指向hos字体
- [ ] 2.3 验证`ui/screens/*.xml`中的字体引用使用hos字体而非默认字体

## 3. 编译验证

- [ ] 3.1 运行`pio run -e native`编译native环境，确保无编译错误
- [ ] 3.2 运行`pio run -e esp32s3`编译ESP32-S3环境，确保无编译错误
- [ ] 3.3 运行`pio test -e native`执行单元测试，确保所有测试通过

## 4. 显示效果验证

- [ ] 4.1 运行native模拟器，检查UI文本是否使用hos字体渲染
- [ ] 4.2 验证UI布局是否保持不变，无文本溢出或截断
- [ ] 4.3 检查FontAwesome图标是否正常显示

## 5. 固件体积验证

- [ ] 5.1 运行`pio run -e esp32s3 -t size`查看固件大小
- [ ] 5.2 与修改前的固件大小对比，确认体积减小

## 6. 文档更新

- [ ] 6.1 更新`include/lv_conf.h`中的注释，说明默认字体已更改为hos_14
- [ ] 6.2 在`AGENTS.md`中记录字体配置变更
