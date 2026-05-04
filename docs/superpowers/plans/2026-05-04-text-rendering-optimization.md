# 文字显示高清优化实现计划

> **面向 AI 代理的工作者：** 必需子技能：使用 superpowers:subagent-driven-development（推荐）或 superpowers:executing-plans 逐任务实现此计划。步骤使用复选框（`- [ ]`）语法来跟踪进度。

**目标：** 解决 ESP32-S3 真机环境下 LVGL 文字显示不够高清的问题，提升小字号文本清晰度

**架构：** 通过优化字体资产（调整字号、导出参数）和 LVGL 渲染配置，在不改变硬件和屏幕分辨率的前提下提升文字渲染质量

**技术栈：** LVGL 9.x, HarmonyOS Sans SC 字体, ST7789 240×280 RGB565

---

## 问题分析

### 根本原因
1. **字体资产问题**：`hos_14` 字体实际是 12px，名字有误导性，在 240×280 小屏上显示过小
2. **字体使用过广**：12px 小字被大量用于状态栏、通道标签、数值显示等关键信息区域
3. **渲染配置保守**：LVGL 配置偏保守，未启用高级渲染优化

### 受影响范围
- 状态栏文字（系统状态、风扇转速、温度）
- 通道标签（CH1, CH2, CH3）
- 通道数值（电压、电流）
- 运行时间、累计电量等信息文字

---

## 文件结构

### 需要修改的文件
- `ui/globals.xml` - 字体定义（主要修改点）
- `ui/screens/home.xml` - 字体使用调整
- `include/lv_conf.h` - LVGL 渲染配置优化

### 生成文件（需要重新生成）
- `ui/fonts/hos_14.c` - 字体数据文件
- `ui/fonts/hos_regular.c` - 字体数据文件
- `ui/screens/home_gen.c` - 屏幕生成代码

---

## 任务分解

### 任务 1：字体资产优化

**文件：**
- 修改：`ui/globals.xml:77-85`
- 重新生成：`ui/fonts/hos_14.c`

- [ ] **步骤 1：检查当前字体定义**

```xml
<!-- 当前 hos_14 定义（实际 12px） -->
<bin
    name="hos_14"
    as_file="false"
    src_path="fonts/HarmonyOS_Sans_SC_Regular.ttf"
    size="12"
    bpp="8"
    range="0x20-0x7F"
    symbols="电源功耗运行面板系统温度电压电流功率就绪运行中停止通道℃：当前占比时间总能效输出风扇转故障"
/>
```

- [ ] **步骤 2：修改字体大小为 14px**

```xml
<!-- 优化后 hos_14 定义（真正的 14px） -->
<bin
    name="hos_14"
    as_file="false"
    src_path="fonts/HarmonyOS_Sans_SC_Regular.ttf"
    size="14"
    bpp="8"
    range="0x20-0x7F"
    symbols="电源功耗运行面板系统温度电压电流功率就绪运行中停止通道℃：当前占比时间总能效输出风扇转故障"
/>
```

- [ ] **步骤 3：重新生成字体文件**

运行 LVGL Editor 或字体生成工具，重新生成 `hos_14.c`

- [ ] **步骤 4：验证字体文件更新**

检查 `ui/fonts/hos_14.c` 文件大小和修改时间是否更新

- [ ] **步骤 5：Commit 字体资产变更**

```bash
git add ui/globals.xml ui/fonts/hos_14.c
git commit -m "fix: correct hos_14 font size from 12px to 14px for better readability"
```

---

### 任务 2：字体使用优化

**文件：**
- 修改：`ui/screens/home.xml`
- 重新生成：`ui/screens/home_gen.c`

- [ ] **步骤 1：识别关键小字区域**

分析 `home.xml` 中使用 `hos_14` 的位置：
- 状态栏文字（第 77, 92, 93, 108 行）
- 通道标签（第 250, 295, 340 行）
- 通道数值（第 266, 267, 311, 312, 356, 357 行）
- 运行时间、电量（第 188, 207 行）

- [ ] **步骤 2：为关键信息区域升级字体**

将以下关键信息区域的字体从 `hos_14` 升级为 `hos_regular`：

```xml
<!-- 状态栏文字 -->
<lv_label bind_text="system_state" style_text_color="0x00e68a" style_text_font="hos_regular" />
<lv_label bind_text="fan_percent" style_text_color="0x3d9eff" style_text_font="hos_regular" />
<lv_label bind_text="fan_rpm_txt" style_text_color="0x6b7d8e" style_text_font="hos_regular" />
<lv_label bind_text="device_temp" style_text_color="0xff6b35" style_text_font="hos_regular" />

<!-- 通道数值 -->
<lv_label bind_text="ch1_voltage" style_text_color="0xFFFFFF" style_text_font="hos_regular" />
<lv_label bind_text="ch1_current" style_text_color="0xFFFFFF" style_text_font="hos_regular" />
<!-- 其他通道类似 -->
```

- [ ] **步骤 3：保持次要信息使用 hos_14**

以下区域保持使用 `hos_14`（升级后的 14px）：
- 通道标签（CH1, CH2, CH3）
- 百分比文字
- 图标文字

- [ ] **步骤 4：重新生成屏幕代码**

运行 LVGL Editor 重新生成 `home_gen.c`

- [ ] **步骤 5：Commit 字体使用优化**

```bash
git add ui/screens/home.xml ui/screens/home_gen.c
git commit -m "refactor: upgrade critical text areas from hos_14 to hos_regular for clarity"
```

---

### 任务 3：LVGL 渲染配置优化

**文件：**
- 修改：`include/lv_conf.h`

- [ ] **步骤 1：优化 DPI 设置**

```c
// 当前设置
#define LV_DPI_DEF 130  // [px/inch]

// 优化为更适合 240×280 屏幕的值
#define LV_DPI_DEF 120  // [px/inch] 降低 DPI 使默认尺寸更合适
```

- [ ] **步骤 2：启用字体缓存（如果内存允许）**

```c
// 在 FONT USAGE 部分添加
#define LV_FONT_CACHE_DEF_SIZE 32  // 缓存 32 个字形
```

- [ ] **步骤 3：优化绘制引擎**

```c
// 当前设置
#define LV_DRAW_COMPLEX 1

// 保持启用，但优化圆形缓存
#define LV_CIRCLE_CACHE_SIZE 8  // 从 4 增加到 8
```

- [ ] **步骤 4：Commit 渲染配置优化**

```bash
git add include/lv_conf.h
git commit -m "perf: optimize LVGL rendering config for better text clarity"
```

---

### 任务 4：构建和测试验证

**文件：**
- 无文件修改，仅验证

- [ ] **步骤 1：编译固件**

```bash
pio run -e esp32s3
```

预期：编译成功，无错误

- [ ] **步骤 2：检查固件大小**

```bash
pio run -e esp32s3 -t size
```

预期：固件大小在可接受范围内（考虑字体文件增大）

- [ ] **步骤 3：静态分析**

```bash
pio check -e esp32s3 --skip-packages
```

预期：无新增警告或错误

- [ ] **步骤 4：Commit 测试验证**

```bash
git add -A
git commit -m "test: verify text rendering optimization builds successfully"
```

---

### 任务 5：文档更新

**文件：**
- 修改：`README.md`

- [ ] **步骤 1：更新字体说明**

在 README.md 的"技术栈"部分添加字体优化说明：

```markdown
## 字体优化

- `hos_14`：14px 常规字体，用于状态栏、通道标签等关键信息
- `hos_regular`：16px 常规字体，用于通道数值等重要数据
- `hos_bold_big`：44px 粗体，用于主功率数字显示
- 所有字体使用 8bpp 抗锯齿，确保在 240×280 屏幕上的清晰度
```

- [ ] **步骤 2：Commit 文档更新**

```bash
git add README.md
git commit -m "docs: add font optimization notes to README"
```

---

## 验证清单

### 编译验证
- [ ] `pio run -e esp32s3` 编译成功
- [ ] 固件大小在可接受范围内
- [ ] 静态分析无新增警告

### 功能验证（需要硬件测试）
- [ ] 状态栏文字清晰可读
- [ ] 通道数值清晰可读
- [ ] 大功率数字显示正常
- [ ] 图标显示正常
- [ ] 整体布局无错位

### 性能验证
- [ ] 刷新率保持在 30fps 以上
- [ ] 内存使用在可接受范围内

---

## 风险评估

### 低风险
- 字体大小调整：仅修改导出参数，不影响功能
- LVGL 配置优化：使用官方推荐参数

### 中风险
- 字体文件增大：14px 字体比 12px 占用更多 Flash 空间
- 布局微调：字体变大可能需要微调布局

### 缓解措施
- 在修改前备份当前字体文件
- 逐步验证每个变更
- 准备回滚方案

---

## 后续优化方向

如果当前优化效果仍不理想，可考虑：

1. **字体字重调整**：测试 Medium 字重是否比 Regular 更清晰
2. **bpp 调整**：测试 4bpp 是否在清晰度和大小间取得更好平衡
3. **字体更换**：测试其他字体（如思源黑体）在小字号下的表现
4. **布局重构**：重新设计 UI 布局，为关键信息分配更多空间

---

## 执行方式

**计划已完成并保存到 `docs/superpowers/plans/2026-05-04-text-rendering-optimization.md`。两种执行方式：**

**1. 子代理驱动（推荐）** - 每个任务调度一个新的子代理，任务间进行审查，快速迭代

**2. 内联执行** - 在当前会话中使用 executing-plans 执行任务，批量执行并设有检查点

**选哪种方式？**