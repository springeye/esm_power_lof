## Context

当前项目使用LVGL 9.x作为UI引擎，屏幕分辨率为240x280像素，像素间距0.11655mm。LVGL的默认字体配置为`LV_FONT_DEFAULT &lv_font_montserrat_14`，但Montserrat字体没有抗锯齿效果，在小尺寸高分辨率屏幕上显示效果不佳。

项目已经引入了HarmonyOS Sans（hos）字体系列，包括多个权重（Regular、Bold、Thin、Light、Medium、Black），并已生成对应的LVGL字体数据文件（hos_*_data.c）。这些字体支持抗锯齿（bpp=8），显示效果更清晰。

**当前状态**：
- `include/lv_conf.h`：LV_FONT_DEFAULT指向Montserrat 14
- `ui/globals.xml`：定义了hos字体映射（hos_14、hos_regular、hos_bold_big等）
- `ui/fonts/`：包含hos字体的TTF文件和生成的C数据文件
- `ui/screens/*.xml`：使用hos字体引用（hos_14、hos_regular等）

**约束条件**：
- 屏幕尺寸：240x280像素
- 像素间距：0.11655mm
- 不改变现有UI布局和显示效果
- 保持FontAwesome图标字体不变

## Goals / Non-Goals

**Goals:**
- 将LVGL默认字体从Montserrat替换为hos_14（HarmonyOS Sans Regular 12px）
- 关闭未使用的Montserrat字体宏，减小固件体积
- 确保所有UI文本使用抗锯齿字体，提升显示质量
- 保持现有UI布局和字体大小不变

**Non-Goals:**
- 不修改FontAwesome图标字体
- 不改变现有UI布局或字体大小
- 不添加新的字体权重或样式
- 不修改LVGL Editor生成的代码（*_gen.c/h）

## Decisions

### Decision 1: 默认字体选择

**选择**: `hos_14`（HarmonyOS Sans SC Regular 12px）

**理由**:
- 项目中已存在hos_14字体数据（ui/fonts/hos_14_data.c）
- 12px字体大小适合240x280屏幕的常规文本显示
- 支持抗锯齿（bpp=8），显示效果清晰
- 已在XML文件中广泛使用，兼容性好

**替代方案**:
- `hos_regular`（16px）：字体较大，可能影响布局
- `hos_bold`（12px Bold）：字重较大，不适合所有场景

### Decision 2: lv_conf.h修改策略

**选择**: 修改LV_FONT_DEFAULT宏，并关闭未使用的Montserrat字体

**理由**:
- 直接修改LV_FONT_DEFAULT宏是最简单、最直接的方法
- 关闭未使用的Montserrat字体（14/16/20/28）可减小固件体积
- 保持其他LVGL字体配置不变（如LV_FONT_FMT_TXT_LARGE等）

**实现**:
```c
// 修改前
#define LV_FONT_DEFAULT &lv_font_montserrat_14

// 修改后
#define LV_FONT_DEFAULT hos_14
```

**替代方案**:
- 使用`&hos_14_data`：直接引用字体数据，但需要确保符号可见性
- 保留Montserrat字体：不推荐，因为用户明确要求替换

### Decision 3: 字体大小验证

**选择**: 保持现有字体大小不变，仅替换字体族

**理由**:
- 用户要求"不改变显示效果（大小和布局）"
- hos_14（12px）与montserrat_14（14px）大小接近，显示效果差异主要来自字体设计而非尺寸
- 如需微调，可在XML中调整字体size参数

**验证方法**:
- 编译并运行native模拟器，对比替换前后的显示效果
- 检查UI布局是否有溢出或错位

## Risks / Trade-offs

### Risk 1: 字体大小差异
- **风险**: hos_14（12px）与montserrat_14（14px）大小略有差异，可能导致布局变化
- **缓解**: 编译后在模拟器中验证显示效果，必要时调整XML中的字体size参数

### Risk 2: 字符集覆盖
- **风险**: hos字体可能不包含某些特殊字符（如FontAwesome图标）
- **缓解**: FontAwesome图标使用独立字体（font_awesome_14/48），不受影响

### Risk 3: 固件体积变化
- **风险**: 关闭Montserrat字体可能影响某些未发现的引用
- **缓解**: 全局搜索确认无其他Montserrat引用后再关闭

### Risk 4: 生成代码兼容性
- **风险**: LVGL Editor生成的代码可能依赖默认字体
- **缓解**: 生成代码显式引用hos_*字体指针，不依赖LV_FONT_DEFAULT

## Migration Plan

### 部署步骤
1. 修改`include/lv_conf.h`中的LV_FONT_DEFAULT宏
2. 关闭未使用的Montserrat字体宏
3. 编译验证（pio run -e native）
4. 在模拟器中验证显示效果
5. 如有布局问题，调整XML字体参数
6. 最终编译esp32s3固件

### 回滚策略
- 保留原始lv_conf.h备份
- 如有问题，恢复LV_FONT_DEFAULT为&lv_font_montserrat_14
- 重新启用Montserrat字体宏

## Open Questions

1. **字体大小微调**: 替换后是否需要调整XML中的字体size参数以保持布局？
2. **其他字体引用**: 是否有其他代码或配置文件引用了Montserrat字体？
3. **性能影响**: hos字体（bpp=8）与Montserrat（bpp=4）的渲染性能差异？
