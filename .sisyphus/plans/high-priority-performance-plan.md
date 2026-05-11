# 高优先级性能优化开发计划

## TL;DR
> **Summary**：围绕 `power_history`、`chart_view`、`data_bridge`、`settings_ui`、`config_manager` 五个热点，先锁定回归行为，再移除全量复制/全量刷新/整页重建/重复持久化路径，保证现有 UI 语义不变。
> **Deliverables**：
> - 采样式功率历史接口与回归测试
> - 固定点数图表刷新链路
> - 基于值变化与节拍门控的 `data_bridge` 刷新策略
> - 单次提交单次写盘的配置保存语义
> - 预创建复用的设置页行控件
> **Effort**：Medium
> **Parallel**：YES - 3 waves
> **Critical Path**：1 → 2 → 3 → 6 → F1-F4

## Context
### Original Request
根据 `docs/superpowers/plans/2026-05-10-high-priority-performance-plan.md` 创建一个开发计划，并进行高精度审核。

### Interview Summary
- 用户未要求扩展范围，保持在原文定义的高优先级性能项内。
- 仓库探索确认目标文件真实存在于 `src/ui_bridge/` 与 `src/app/`，测试入口为 `src/test_main.cpp`。
- `pio test -e test` 与 `pio run -e esp32s3` 在当前仓库有效，适合作为统一验证命令。
- 无额外偏好输入，因此采用仓库内最小侵入方案：采样替代线性副本、固定图表点数、差异化刷新、setter 仅更新内存、设置页固定行复用。

### Metis Review (gaps addressed)
- 已显式加入防范围蔓延护栏：禁止扩展到中低优先级性能项、禁止改动 UI 生成文件、禁止改动网络/风扇/电源逻辑。
- 已显式加入边界条件：空历史、跨环形边界、窗口内无数据、图表页未激活、页面 item 数小于 `MAX_PAGE_ITEMS`、主题切换即时生效、无 Preferences 后端时 `save_to_nvs()` 仍保持可调用。
- 已显式加入验收条件：每个任务都绑定到 `pio test -e test`、`pio run -e esp32s3` 或源码结构断言，不留口头验收。
- 已显式加入实现禁区：不得修改 `ui/screens/*_gen.*`、`ui/lof_power_system_gen.*`、`ui/fonts/*_data.c`，不得引入新的周期性重刷新。

## Work Objectives
### Core Objective
在不改变现有显示语义、按键交互语义、主题切换语义和配置读取语义的前提下，降低 LVGL 任务中的全量扫描、全量拷贝、全量重建和重复 NVS 写入开销。

### Deliverables
- `power_history` 从“查询即线性展开 3000 点”改为“按窗口 + 点数上限采样输出”，并在图表调用方完成迁移后移除旧的 `power_history_get_range()` 接口与调用点。
- `chart_view` 每次刷新最多处理固定数量图表点，并继续保留 CH1/CH2/CH3 颜色与 Y 轴策略。
- `data_bridge` 将高频字段改为“仅值变化时更新”，将 `uptime` / `Wh` 改为低频刷新，并限制图表刷新节拍。
- `config_manager` 的 setter 不再立即落盘，由显式 `save_to_nvs()` 负责持久化。
- `settings_ui` 改为首屏预创建固定数量 row，切页时仅绑定内容与样式，不再 `lv_obj_clean()` 整页重建。

### Definition of Done (verifiable conditions with commands)
- `pio test -e test` 通过，且新增的 power history / config manager 回归测试已执行。
- `pio run -e esp32s3` 通过，无接口签名错误或构建失败。
- 源码结构满足以下约束：
  - `src/ui_bridge/power_history.cpp` 不再保留 `s_linear_temp`。
  - `src/ui_bridge/chart_view.cpp` 存在单一图表点数常量，且不再以 `3000` 作为显示点数上限。
  - `src/ui_bridge/power_history.h` / `src/ui_bridge/power_history.cpp` 在整体验证时不再包含 `power_history_get_range()`。
  - `src/ui_bridge/settings_ui.cpp` 的页面重建路径不再调用 `lv_obj_clean(g_content_area)`。
  - `src/app/config_manager.cpp` 的 setter 不再直接调用 `save_to_nvs_locked()`。

### Must Have
- 保持 `power_history` 的时间顺序语义。
- 保持图表页颜色、窗口切换、固定 Y 轴模式和空图语义。
- 保持 `data_bridge` 当前 subject 字段集不减少。
- 保持设置页 K1/K3/K2 的焦点、编辑、切页语义。
- 保持 `save_to_nvs()` 对所有调用方仍然可用。

### Must NOT Have (guardrails, AI slop patterns, scope boundaries)
- 不得修改任何 LVGL Editor 生成文件。
- 不得新增新的全量历史副本缓存。
- 不得把 `data_bridge` 刷新周期从 200 ms 改成更高频。
- 不得把设置页改成“每页单独建一套控件树”。
- 不得把配置保存逻辑改成“永不持久化”；必须保留显式保存入口。
- 不得顺手改动中低优先级项：按键轮询、主题递归、`power_task` 周期、网络/OTA 行为。

## Verification Strategy
> ZERO HUMAN INTERVENTION - all verification is agent-executed.
- Test decision: tests-after + Unity（`src/test_main.cpp`）
- QA policy: 每个任务都包含 Happy path + Failure/edge case 场景
- Evidence: `.sisyphus/evidence/task-{N}-{slug}.{ext}`
- Primary commands:
  - `pio test -e test`
  - `pio run -e esp32s3`
  - `python -c "..."` 用于源码结构断言

## Execution Strategy
### Parallel Execution Waves
> Target: 5-8 tasks per wave. <3 per wave (except final) = under-splitting.
> Extract shared dependencies as Wave-1 tasks for max parallelism.

Wave 1: Task 1（power_history 采样接口） + Task 4（config_manager 持久化语义）
Wave 2: Task 2（chart_view 限量刷新） + Task 5（settings_ui 行复用）
Wave 3: Task 3（data_bridge 差异化刷新） + Task 6（整体验证与结果汇总）
Final Wave: F1-F4 审核并行执行

### Dependency Matrix (full, all tasks)
| Task | Depends On | Enables |
|---|---|---|
| 1. power_history 采样接口 | 无 | 2, 3, 6 |
| 2. chart_view 限量刷新 | 1 | 3, 6 |
| 3. data_bridge 差异化刷新 | 1, 2 | 6 |
| 4. config_manager 单次提交单次写盘 | 无 | 5, 6 |
| 5. settings_ui 行复用 | 4 | 6 |
| 6. 整体验证与性能结果汇总 | 1, 2, 3, 4, 5 | F1-F4 |

### Agent Dispatch Summary (wave → task count → categories)
- Wave 1 → 2 tasks → `deep`, `unspecified-high`
- Wave 2 → 2 tasks → `unspecified-high`, `deep`
- Wave 3 → 2 tasks → `unspecified-high`, `quick`
- Final Wave → 4 tasks → `subagent_type=oracle`, `category=unspecified-high`, `category=unspecified-high`, `category=deep`

## TODOs
> Implementation + Test = ONE task. Never separate.
> EVERY task MUST have: Agent Profile + Parallelization + QA Scenarios.

- [ ] 1. 重构 `power_history` 为采样式窗口查询

  **What to do**：
  1. 在 `src/ui_bridge/power_history.h` 新增 `PowerHistorySample` 与 `power_history_sample_window(...)` 接口，签名固定为：
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
  2. 保留 `PowerPoint`、`power_history_init()`、`power_history_push()`，并暂时保留 `power_history_get_range()` 仅供当前 `chart_view.cpp` 与旧测试过渡使用；旧接口必须在 Task 2 中随图表调用方迁移一起删除。
  3. 在 `src/ui_bridge/power_history.cpp` 删除 `s_linear_temp`，不再维护 72 KB 线性副本。
  4. `power_history_sample_window()` 必须在一次持锁遍历中完成：
     - 根据最新点时间戳计算 `start_time`
     - 仅保留 `timestamp_ms >= start_time` 的点
     - 若窗口内点数 `<= max_points`，按时间顺序完整输出
     - 若窗口内点数 `> max_points`，采用等间距抽样，始终保留窗口内最旧点与最新点
     - 返回值为实际输出点数，且绝不超过 `min(max_points, out_capacity)`
  5. 若 `max_points == 0`、`out_capacity == 0`、`out_points == nullptr`、无数据、`ch` 越界，统一返回 `0`。
  6. 同步修改 `src/test_main.cpp` 现有 `power_history_*` 测试，优先改用 `power_history_sample_window()`；若为了保持 Wave 1 可独立提交而临时保留 1-2 条旧接口测试，也必须在 Task 2 删除旧接口时一并改完，Task 6 前仓库中不得再出现 `power_history_get_range()`。
  7. 在 `src/test_main.cpp` 增加以下测试并纳入 `RUN_TEST(...)`：
     ```cpp
     void test_power_history_time_window_bounds(void);
     void test_power_history_query_is_stable(void);
     void test_power_history_overflow_keeps_order(void);
     void test_power_history_sample_window_caps_points(void);
     void test_power_history_sample_window_prefers_recent_range(void);
     void test_power_history_sample_window_zero_capacity(void);
     ```

  **Must NOT do**：
  - 不得新增第二份完整环形缓冲镜像。
  - 不得把采样逻辑下沉到 `chart_view.cpp`。
  - 不得改变通道编号语义（0/1/2）。

  **Recommended Agent Profile**：
  - Category: `deep` - Reason: 涉及公共接口、采样策略、回归测试与后续任务依赖。
  - Skills: [`test-driven-development`, `verification-before-completion`] - 先锁定行为再改接口，并用完整验证收尾。
  - Omitted: [`frontend-design`] - 与 UI 样式无关。

  **Parallelization**：Can Parallel: YES | Wave 1 | Blocks: 2, 3, 6 | Blocked By: none

  **References** (executor has NO interview context - be exhaustive)：
  - Pattern: `src/ui_bridge/power_history.h:8-35` - 当前公共 API 与 `PowerPoint` 定义；Wave 1 结束时允许旧接口暂存，Task 2 结束后必须只保留 `PowerHistorySample` + `power_history_sample_window()`。
  - Pattern: `src/ui_bridge/power_history.cpp:7-18` - 当前 `BUFFER_SIZE=3000`、`CHANNEL_COUNT=3`、`s_linear_temp` 内存热点。
  - Pattern: `src/ui_bridge/power_history.cpp:44-76` - 当前时间窗口过滤与线性展开路径。
  - Test: `src/test_main.cpp:9-88` - 已有 power history 测试风格与断言模式。
  - Build/Test: `platformio.ini:64-119` - `env:test` 的真实入口。

  **Acceptance Criteria** (agent-executable only)：
  - [ ] `pio test -e test` 通过，且新增 power history 测试全部 PASS。
  - [ ] `python -c "from pathlib import Path; s=Path('src/ui_bridge/power_history.cpp').read_text(encoding='utf-8'); assert 's_linear_temp' not in s"` 返回 exit 0。
  - [ ] `python -c "from pathlib import Path; s=Path('src/ui_bridge/power_history.h').read_text(encoding='utf-8'); assert 'power_history_sample_window' in s and 'PowerHistorySample' in s"` 返回 exit 0。

  **QA Scenarios** (MANDATORY - task incomplete without these)：
  ```
  Scenario: 采样接口在大窗口内限制输出点数
    Tool: Bash
    Steps:
      1. 在 src/test_main.cpp 中加入 3500 点输入、300 点输出上限的测试 test_power_history_sample_window_caps_points。
      2. 运行命令：pio test -e test
    Expected: 新测试通过，返回点数 <= 300，且时间戳严格递增。
    Evidence: .sisyphus/evidence/task-1-power-history.txt

  Scenario: 边界参数与空窗口安全返回
    Tool: Bash
    Steps:
      1. 在 src/test_main.cpp 中加入 test_power_history_sample_window_zero_capacity 与空窗口断言。
      2. 运行命令：pio test -e test
    Expected: 对 0 容量、nullptr、无数据三类输入均返回 0，不崩溃、不越界。
    Evidence: .sisyphus/evidence/task-1-power-history-error.txt
  ```

  **Commit**：YES | Message: `refactor(power-history): replace linear buffer with sampled window reads` | Files: `src/ui_bridge/power_history.h`, `src/ui_bridge/power_history.cpp`, `src/test_main.cpp`

- [ ] 2. 将 `chart_view` 切换到固定点数采样刷新

  **What to do**：
  1. 在 `src/ui_bridge/chart_view.cpp` 定义唯一显示点数常量：
     ```cpp
     static constexpr uint16_t CHART_POINT_COUNT = 300;
     ```
  2. 保留窗口常量 `WINDOW_1MIN_MS` / `WINDOW_5MIN_MS` / `WINDOW_10MIN_MS` 与颜色表 `CH_COLORS`。
  3. 在 `update_chart_data(uint8_t ch)` 中改为调用 `power_history_sample_window(...)`，栈上使用固定数组：
     ```cpp
     PowerHistorySample points[CHART_POINT_COUNT];
     const uint32_t count = power_history_sample_window(
         ch,
         window_ms,
         CHART_POINT_COUNT,
         points,
         CHART_POINT_COUNT);
     ```
  4. `count == 0` 时继续调用 `lv_chart_set_all_values(..., LV_CHART_POINT_NONE)`，保持空图语义。
  5. `count > 0` 时调用 `lv_chart_set_point_count(g_charts[ch].chart, count)`，然后仅写入 `count` 个点。
  6. 严禁再把 `3000` 作为图表显示点数上限；`3000` 仅允许留在历史存储层。
  7. 保持 `chart_yaxis_mode == 1` 时继续使用 `config_manager::get_design_power_w()` 作为固定 Y 轴上限。
  8. 在 `src/ui_bridge/chart_view.cpp` 完成迁移后，删除 `power_history_get_range()` 的唯一剩余调用点，并同步从 `src/ui_bridge/power_history.h/.cpp` 删除旧接口声明与实现。
  9. 在 `src/test_main.cpp` 至少增加一条“采样代理测试”或直接复用任务 1 中的采样测试，确保 3000+ 输入时图表上游最多只拿到 300 点。

  **Must NOT do**：
  - 不得修改 CH1/CH2/CH3 颜色。
  - 不得改变 `chart_view_cycle_window()` 和 `chart_view_get_window_ms()` 语义。
  - 不得在 `chart_view.cpp` 中重新实现窗口过滤。

  **Recommended Agent Profile**：
  - Category: `unspecified-high` - Reason: 单文件为主，但依赖新接口并要求保持 LVGL 语义不变。
  - Skills: [`verification-before-completion`] - 需要用编译和源码断言收尾。
  - Omitted: [`test-driven-development`] - 直接 UI 控件层没有完整 Unity 测试面，测试重心在采样代理层。

  **Parallelization**：Can Parallel: NO | Wave 2 | Blocks: 3, 6 | Blocked By: 1

  **References** (executor has NO interview context - be exhaustive)：
  - Pattern: `src/ui_bridge/chart_view.cpp:14-24` - 窗口与颜色常量。
  - Pattern: `src/ui_bridge/chart_view.cpp:81-98` - 现有 chart 初始化与 `lv_chart_set_point_count(..., 300)`。
  - Pattern: `src/ui_bridge/chart_view.cpp:137-165` - 当前全量取数、逐点写入、刷新链路。
  - Pattern: `src/ui_bridge/chart_view.cpp:171-209` - 当前懒创建与窗口切换入口。
  - API/Type: `src/ui_bridge/power_history.h:8-35` - 历史层现有公共接口位置；Task 2 必须完成调用方迁移并删除旧接口。
  - Test: `src/test_main.cpp:9-88` - power history 测试承载位置。

  **Acceptance Criteria** (agent-executable only)：
  - [ ] `pio run -e esp32s3` 通过。
  - [ ] `pio test -e test` 通过。
  - [ ] `python -c "from pathlib import Path; s=Path('src/ui_bridge/chart_view.cpp').read_text(encoding='utf-8'); assert 'CHART_POINT_COUNT' in s and 'power_history_sample_window' in s and 'power_history_get_range' not in s and '(count > 3000)' not in s"` 返回 exit 0。
  - [ ] `python -c "from pathlib import Path; h=Path('src/ui_bridge/power_history.h').read_text(encoding='utf-8'); c=Path('src/ui_bridge/power_history.cpp').read_text(encoding='utf-8'); assert 'power_history_get_range' not in h and 'power_history_get_range' not in c"` 返回 exit 0。

  **QA Scenarios** (MANDATORY - task incomplete without these)：
  ```
  Scenario: 图表刷新仅处理固定上限点数
    Tool: Bash
    Steps:
      1. 完成 chart_view.cpp 对 power_history_sample_window 的接入。
      2. 运行命令：pio run -e esp32s3
      3. 运行命令：pio test -e test
    Expected: 构建通过，采样相关测试仍通过，chart_view.cpp 中只保留 CHART_POINT_COUNT 一个显示上限常量。
    Evidence: .sisyphus/evidence/task-2-chart-view.txt

  Scenario: 无数据窗口继续显示空图
    Tool: Bash
    Steps:
      1. 保留 count==0 时的空图分支。
      2. 运行命令：python -c "from pathlib import Path; s=Path('src/ui_bridge/chart_view.cpp').read_text(encoding='utf-8'); assert 'LV_CHART_POINT_NONE' in s"
      3. 运行命令：pio run -e esp32s3
    Expected: 源码仍包含空图分支，且编译通过。
    Evidence: .sisyphus/evidence/task-2-chart-view-error.txt
  ```

  **Commit**：YES | Message: `perf(chart-view): cap chart refresh to sampled history points` | Files: `src/ui_bridge/chart_view.cpp`, `src/ui_bridge/power_history.h`, `src/ui_bridge/power_history.cpp`, `src/test_main.cpp`

- [ ] 3. 对 `data_bridge` 实施差异化与节拍化刷新

  **What to do**：
  1. 在 `src/ui_bridge/data_bridge.cpp` 的匿名命名空间内新增缓存结构：
     ```cpp
     struct UiSnapshot {
         bool initialized;
         float temp_c;
         uint16_t ch_mv[3];
         float ch_a[3];
         float ch_w[3];
         float total_w;
         uint32_t rpm;
         int32_t power_percent;
         uint16_t design_power_w;
         uint8_t psu_state;
         bool fault;
     };
     ```
  2. 新增节拍状态：
     - `static uint32_t g_last_low_freq_ms = 0;`
     - `static uint32_t g_last_chart_refresh_ms = 0;`
     - `static constexpr uint32_t LOW_FREQ_INTERVAL_MS = 1000;`
     - `static constexpr uint32_t CHART_REFRESH_INTERVAL_MS = 500;`
  3. 保持 `lv_timer_create(refresh_cb, 200, nullptr)` 不变，但在 `refresh_cb()` 内按三组处理：
     - 高频字段：温度、电压、电流、功率、总功率、功率百分比、风扇百分比、RPM，仅当数值变化时才更新对应 subject
     - 低频字段：`uptime`、`wh` 每 1000 ms 更新一次
     - 图表字段：仅当当前视图在 `VIEW_CHART_CH1..VIEW_CHART_CH3` 且（通道功率变化或达到 500 ms 刷新节拍）时调用 `chart_view_update()`
  4. `power_history_push(i, lv_tick_get(), ch_w)` 保留，但每轮只写一次，不允许在图表刷新分支再重复 push。
  5. `device_power_percent` 与 `device_power_percent_txt` 必须同步门控，避免一个更新一个不更新。
  6. `system_state` 的门控条件应基于 `fault` 与 `psu_state` 组合，而不是单字段。
  7. 对 float 比较采用稳定阈值：电压、电流、功率比较使用 `fabsf(new-old) >= 0.001f`；温度使用 `0.1f`；总功率使用 `0.01f`。
  8. 若 `UiSnapshot.initialized == false`，首轮必须强制刷新全部字段并初始化缓存。

  **Must NOT do**：
  - 不得删除 `power_history_push()` 采集行为。
  - 不得把 `uptime` / `wh` 继续保持在每 200 ms 无条件刷新。
  - 不得在非图表视图下继续定期调用 `chart_view_update()`。

  **Recommended Agent Profile**：
  - Category: `unspecified-high` - Reason: 单文件逻辑重排较重，涉及多字段一致性与节拍判断。
  - Skills: [`verification-before-completion`] - 该任务难以完全单元测试，必须依靠结构断言与完整构建验证。
  - Omitted: [`frontend-design`] - 无视觉样式设计工作。

  **Parallelization**：Can Parallel: NO | Wave 3 | Blocks: 6 | Blocked By: 1, 2

  **References** (executor has NO interview context - be exhaustive)：
  - Pattern: `src/ui_bridge/data_bridge.cpp:18-19` - 当前仅记录 `g_start_ms`。
  - Pattern: `src/ui_bridge/data_bridge.cpp:21-52` - 现有格式化函数。
  - Pattern: `src/ui_bridge/data_bridge.cpp:54-161` - 当前 200 ms 全字段无差别刷新路径。
  - Pattern: `src/ui_bridge/data_bridge.cpp:119-122` - 当前图表页每轮刷新入口。
  - API/Type: `src/ui_bridge/chart_view.cpp:171-209` - 图表刷新与窗口接口。
  - API/Type: `src/app/config_manager.h:86-109` - 设计功率、主题与显示配置 getter/setter。

  **Acceptance Criteria** (agent-executable only)：
  - [ ] `pio run -e esp32s3` 通过。
  - [ ] `pio test -e test` 通过。
  - [ ] `python -c "from pathlib import Path; s=Path('src/ui_bridge/data_bridge.cpp').read_text(encoding='utf-8'); assert 'UiSnapshot' in s and 'LOW_FREQ_INTERVAL_MS' in s and 'CHART_REFRESH_INTERVAL_MS' in s and 'lv_timer_create(refresh_cb, 200, nullptr);' in s"` 返回 exit 0。

  **QA Scenarios** (MANDATORY - task incomplete without these)：
  ```
  Scenario: 高频字段仅在变化时更新，低频字段 1 秒节拍更新
    Tool: Bash
    Steps:
      1. 完成 UiSnapshot 与节拍门控实现。
      2. 运行命令：python -c "from pathlib import Path; s=Path('src/ui_bridge/data_bridge.cpp').read_text(encoding='utf-8'); assert 'LOW_FREQ_INTERVAL_MS = 1000' in s or 'LOW_FREQ_INTERVAL_MS = 1000u' in s"
      3. 运行命令：pio run -e esp32s3
    Expected: 源码存在低频节拍常量，且构建通过。
    Evidence: .sisyphus/evidence/task-3-data-bridge.txt

  Scenario: 图表页外不触发无条件 chart_view_update
    Tool: Bash
    Steps:
      1. 运行命令：python -c "from pathlib import Path; s=Path('src/ui_bridge/data_bridge.cpp').read_text(encoding='utf-8'); assert 'VIEW_CHART_CH1' in s and 'CHART_REFRESH_INTERVAL_MS' in s"
      2. 运行命令：pio test -e test
    Expected: 图表刷新条件同时受当前视图和节拍/值变化控制，现有测试无回归。
    Evidence: .sisyphus/evidence/task-3-data-bridge-error.txt
  ```

  **Commit**：YES | Message: `perf(data-bridge): avoid redundant subject and chart updates` | Files: `src/ui_bridge/data_bridge.cpp`, `src/test_main.cpp`

- [ ] 4. 将 `config_manager` 改为显式持久化模型

  **What to do**：
  1. 保持 `init()`、`load_from_nvs()`、`save_to_nvs()` 的对外 API 不变。
  2. 在 `src/app/config_manager.cpp` 中移除各 setter 内部的 `save_to_nvs_locked()` 调用；setter 只负责：
     - 持锁
     - `ensure_initialized_locked()`
     - clamp / 校验
     - 更新 `s_config`
  3. `set_design_power_w(uint16_t v)` 仍然只接受 `350/450/550/750`，非法值保持忽略。
  4. `set_wifi_ssid()` / `set_wifi_password()` 保留截断与 `\0` 终止逻辑，但不再立即落盘。
  5. 在 `src/test_main.cpp` 新增并注册以下测试：
     ```cpp
     void test_config_theme_mode_set_get_without_autosave(void);
     void test_config_design_power_set_get_without_autosave(void);
     void test_config_save_to_nvs_callable_after_setters(void);
     ```
  6. 这些测试至少验证：
     - setter 后 getter 立即可读
     - `save_to_nvs()` 在 setter 之后仍可单独调用
     - 非法设计功率值不会污染当前配置
  7. 不新增“批处理模式”或“dirty flag”公开接口；当前范围只消除重复写盘，不做更大范围配置系统重构。

  **Must NOT do**：
  - 不得删除 `save_to_nvs_locked()` / `save_to_nvs()`。
  - 不得改变现有 clamp 范围。
  - 不得引入额外线程或异步保存机制。

  **Recommended Agent Profile**：
  - Category: `deep` - Reason: 影响所有配置 setter 的统一语义，需配套回归测试。
  - Skills: [`test-driven-development`, `verification-before-completion`] - 先补回归测试，再统一移除 autosave。
  - Omitted: [`systematic-debugging`] - 当前是语义重构，不是已知故障定位。

  **Parallelization**：Can Parallel: YES | Wave 1 | Blocks: 5, 6 | Blocked By: none

  **References** (executor has NO interview context - be exhaustive)：
  - Pattern: `src/app/config_manager.h:55-109` - 当前公共 API 范围。
  - Pattern: `src/app/config_manager.cpp:133-162` - 当前 `save_to_nvs_locked()` 实现，必须保留。
  - Pattern: `src/app/config_manager.cpp:261-265` - 当前 `save_to_nvs()` 公共入口。
  - Pattern: `src/app/config_manager.cpp:273-541` - 当前 setter 全部立即落盘，需统一消除。
  - Pattern: `src/app/config_manager.cpp:94-96` - 设计功率合法值约束。
  - Test: `src/test_main.cpp:90-110` - 已有 config manager 测试风格。

  **Acceptance Criteria** (agent-executable only)：
  - [ ] `pio test -e test` 通过，且新增 config manager 回归测试全部 PASS。
  - [ ] `pio run -e esp32s3` 通过。
  - [ ] `python -c "from pathlib import Path; s=Path('src/app/config_manager.cpp').read_text(encoding='utf-8'); setters=s[s.index('void set_fan_temp_low'):]; assert 'save_to_nvs_locked();' not in setters"` 返回 exit 0。

  **QA Scenarios** (MANDATORY - task incomplete without these)：
  ```
  Scenario: setter 只改内存，save_to_nvs 仍可单独调用
    Tool: Bash
    Steps:
      1. 在 src/test_main.cpp 增加 config manager 回归测试并注册。
      2. 运行命令：pio test -e test
    Expected: 新测试通过，setter 后 getter 立即返回新值，save_to_nvs() 可调用。
    Evidence: .sisyphus/evidence/task-4-config-manager.txt

  Scenario: 非法设计功率值不会污染配置
    Tool: Bash
    Steps:
      1. 在 test_config_design_power_set_get_without_autosave 中先设置合法值，再尝试非法值。
      2. 运行命令：pio test -e test
    Expected: 非法值被忽略，getter 保持最后一个合法值。
    Evidence: .sisyphus/evidence/task-4-config-manager-error.txt
  ```

  **Commit**：YES | Message: `perf(config-manager): defer persistence until explicit save` | Files: `src/app/config_manager.h`, `src/app/config_manager.cpp`, `src/test_main.cpp`

- [ ] 5. 将 `settings_ui` 改为固定行预创建与绑定复用

  **What to do**：
  1. 保留 `MAX_PAGE_ITEMS = 8`、页面定义 `PAGES[]`、按键分发与 `lv_async_call()` 入口不变。
  2. 在 `src/ui_bridge/settings_ui.cpp` 新增并使用以下 helper：
     ```cpp
     void ensure_rows_created();
     void bind_page_to_rows();
     void hide_unused_rows(size_t used_count);
     ```
  3. `ensure_rows_created()` 在 `g_content_area` 下仅首次创建 `MAX_PAGE_ITEMS` 行，每行都保存：
     - `row`
     - label 控件引用（新增到 `RowRefs`）
     - value_label 控件引用
  4. 将 `RowRefs` 改为：
     ```cpp
     struct RowRefs {
         lv_obj_t* row;
         lv_obj_t* label;
         lv_obj_t* value_label;
     };
     ```
  5. `rebuild_page()` 重写为：
     - `stop_blink()`
     - `g_editing = false`
     - `ensure_rows_created()`
     - `g_row_count = current_page().item_count`
     - `bind_page_to_rows()`
     - `hide_unused_rows(g_row_count)`
     - 修正 `g_focus_index`
     - `update_header()` + `refresh_all_values()`
  6. `bind_page_to_rows()` 只负责把当前页的 `item.label` 写入已有 label，把当前值写入已有 `value_label`，并显示当前需要的 row。
  7. `hide_unused_rows()` 必须对 `used_count..MAX_PAGE_ITEMS-1` 的 row 调用 `lv_obj_add_flag(row, LV_OBJ_FLAG_HIDDEN)`，而不是销毁对象。
  8. 保持以下语义不变：
     - K1/K3 短按移动焦点；编辑态下调整值
     - K2 短按进入/确认编辑
     - K1/K3 长按切页
     - K2 长按退出设置页
     - 主题模式提交后立即 `theme_manager::theme_apply_to_active_screen()`
     - 切页后 `g_focus_index = 0`
  9. `commit_edit_mode()` 保留显式 `config_manager::save_to_nvs()`；该调用依赖任务 4 已消除 setter autosave。

  **Must NOT do**：
  - 不得继续在 `rebuild_page()` 中调用 `lv_obj_clean(g_content_area)`。
  - 不得把每次切页改成重新创建 `g_screen`。
  - 不得破坏闪烁编辑逻辑与 `stop_blink()` 清理流程。

  **Recommended Agent Profile**：
  - Category: `deep` - Reason: 单文件较大，涉及状态、焦点、编辑、切页、主题联动的行为保持。
  - Skills: [`verification-before-completion`] - 主要依靠源码结构断言与构建验证。
  - Omitted: [`frontend-design`] - 不是视觉重设计，只是控件生命周期优化。

  **Parallelization**：Can Parallel: NO | Wave 2 | Blocks: 6 | Blocked By: 4

  **References** (executor has NO interview context - be exhaustive)：
  - Pattern: `src/ui_bridge/settings_ui.cpp:27-28` - `BLINK_PERIOD_MS` 与 `MAX_PAGE_ITEMS` 现有常量。
  - Pattern: `src/ui_bridge/settings_ui.cpp:76-80` - 当前 `RowRefs` 仅保存 row 与 value_label，需要扩充。
  - Pattern: `src/ui_bridge/settings_ui.cpp:171-183` - 当前全局 UI 状态。
  - Pattern: `src/ui_bridge/settings_ui.cpp:401-421` - 当前值刷新与样式刷新路径，应复用。
  - Pattern: `src/ui_bridge/settings_ui.cpp:423-473` - 当前 `lv_obj_clean + 全量重建` 热点。
  - Pattern: `src/ui_bridge/settings_ui.cpp:555-569` - 编辑提交与显式 `save_to_nvs()` 入口。
  - Pattern: `src/ui_bridge/settings_ui.cpp:587-606` - 切页与 show 时的重建入口。
  - Pattern: `src/ui_bridge/settings_ui.cpp:631-705` - 按键分发语义，必须保持。

  **Acceptance Criteria** (agent-executable only)：
  - [ ] `pio run -e esp32s3` 通过。
  - [ ] `pio test -e test` 通过。
  - [ ] `python -c "from pathlib import Path; s=Path('src/ui_bridge/settings_ui.cpp').read_text(encoding='utf-8'); assert 'ensure_rows_created' in s and 'bind_page_to_rows' in s and 'hide_unused_rows' in s and 'lv_obj_clean(g_content_area)' not in s"` 返回 exit 0。

  **QA Scenarios** (MANDATORY - task incomplete without these)：
  ```
  Scenario: 切页复用既有 row，不再整页销毁重建
    Tool: Bash
    Steps:
      1. 完成 RowRefs 扩展与 3 个 helper 接入。
      2. 运行命令：python -c "from pathlib import Path; s=Path('src/ui_bridge/settings_ui.cpp').read_text(encoding='utf-8'); assert 'lv_obj_clean(g_content_area)' not in s"
      3. 运行命令：pio run -e esp32s3
    Expected: 页面重建路径已移除整页 clean，构建通过。
    Evidence: .sisyphus/evidence/task-5-settings-ui.txt

  Scenario: 切页后焦点重置且主题提交仍即时应用
    Tool: Bash
    Steps:
      1. 运行命令：python -c "from pathlib import Path; s=Path('src/ui_bridge/settings_ui.cpp').read_text(encoding='utf-8'); assert 'g_focus_index = 0;' in s and 'theme_manager::theme_apply_to_active_screen();' in s and 'config_manager::save_to_nvs();' in s"
      2. 运行命令：pio test -e test
    Expected: 代码仍保留焦点重置、主题即时应用与显式保存路径，现有测试无回归。
    Evidence: .sisyphus/evidence/task-5-settings-ui-error.txt
  ```

  **Commit**：YES | Message: `perf(settings-ui): reuse fixed rows and avoid full rebuilds` | Files: `src/ui_bridge/settings_ui.cpp`, `src/app/config_manager.h`, `src/app/config_manager.cpp`, `src/test_main.cpp`

- [ ] 6. 运行整体验证并汇总性能结果

  **What to do**：
  1. 在全部代码改动完成后，统一运行：
     - `pio test -e test`
     - `pio run -e esp32s3`
  2. 执行源码结构断言，确认以下事实：
     - `src/ui_bridge/power_history.cpp` 中 `s_linear_temp` 已删除
     - `src/ui_bridge/chart_view.cpp` 中显示点数上限固定为 `CHART_POINT_COUNT = 300`
     - `src/ui_bridge/data_bridge.cpp` 中存在低频与图表刷新节拍常量
     - `src/ui_bridge/settings_ui.cpp` 中不存在 `lv_obj_clean(g_content_area)`
     - `src/app/config_manager.cpp` 的 setter 片段中不存在 `save_to_nvs_locked()`
  3. 生成最终说明，必须记录：
     - 是否完全移除了 `s_linear_temp`
     - 图表单次刷新最大点数
     - `data_bridge` 的高频/低频/图表三组门控策略
     - `settings_ui` 是否改为固定 row 复用
     - `config_manager` 是否变为单次提交单次写盘
  4. 若任一断言失败，回到对应任务修复，不允许带着已知缺口进入 Final Verification Wave。

  **Must NOT do**：
  - 不得只跑单一命令就宣称全部完成。
  - 不得跳过结构断言。
  - 不得在最终说明中使用“应该”“大概”“预计”字样。

  **Recommended Agent Profile**：
  - Category: `quick` - Reason: 已是验证与汇总阶段，操作顺序固定。
  - Skills: [`verification-before-completion`] - 需要完整命令与输出证据。
  - Omitted: [`test-driven-development`] - 此阶段不再新增功能测试设计。

  **Parallelization**：Can Parallel: NO | Wave 3 | Blocks: F1-F4 | Blocked By: 1, 2, 3, 4, 5

  **References** (executor has NO interview context - be exhaustive)：
  - Build/Test: `platformio.ini:4-63` - `env:esp32s3` 构建环境。
  - Build/Test: `platformio.ini:64-119` - `env:test` 测试环境。
  - Test: `src/test_main.cpp:144-161` - `RUN_TEST(...)` 注册区，验证新增测试已接入。
  - Pattern: `src/ui_bridge/power_history.cpp:7-18` - 需要消除的历史线性副本热点。
  - Pattern: `src/ui_bridge/settings_ui.cpp:423-473` - 需要消除的整页重建热点。
  - Pattern: `src/app/config_manager.cpp:273-541` - 需要消除的 setter 自动落盘路径。

  **Acceptance Criteria** (agent-executable only)：
  - [ ] `pio test -e test` 通过。
  - [ ] `pio run -e esp32s3` 通过。
  - [ ] 所有源码结构断言命令均返回 exit 0。

  **QA Scenarios** (MANDATORY - task incomplete without these)：
  ```
  Scenario: 全量回归通过
    Tool: Bash
    Steps:
      1. 运行命令：pio test -e test
      2. 运行命令：pio run -e esp32s3
    Expected: 两条命令均 exit 0。
    Evidence: .sisyphus/evidence/task-6-final-verification.txt

  Scenario: 五项结构断言全部通过
    Tool: Bash
    Steps:
      1. 依次运行对 power_history / chart_view / data_bridge / settings_ui / config_manager 的 python 源码断言命令。
    Expected: 所有断言命令 exit 0，且最终说明完整记录五项性能收敛结果。
    Evidence: .sisyphus/evidence/task-6-final-verification-error.txt
  ```

  **Commit**：YES | Message: `perf(ui-bridge): optimize high-priority refresh and persistence paths` | Files: `src/ui_bridge/power_history.h`, `src/ui_bridge/power_history.cpp`, `src/ui_bridge/chart_view.cpp`, `src/ui_bridge/data_bridge.cpp`, `src/ui_bridge/settings_ui.cpp`, `src/app/config_manager.h`, `src/app/config_manager.cpp`, `src/test_main.cpp`

## Final Verification Wave (MANDATORY — after ALL implementation tasks)
> 4 review agents run in PARALLEL. ALL must APPROVE. Present consolidated results to user and get explicit "okay" before completing.
> **Do NOT auto-proceed after verification. Wait for user's explicit approval before marking work complete.**
> **Never mark F1-F4 as checked before getting user's okay.** Rejection or user feedback -> fix -> re-run -> present again -> wait for okay.

- [ ] F1. Plan Compliance Audit

  **Dispatch**：`task(subagent_type="oracle", load_skills=[], run_in_background=true, description="Plan compliance audit", prompt="Audit branch changes against .sisyphus/plans/high-priority-performance-plan.md. Verify every completed task has matching code/test evidence, all acceptance criteria are satisfied, and no required item is missing. Return PASS/FAIL with file-path-based findings only.")`

  **Acceptance Criteria**：
  - [ ] Oracle 返回 PASS，或仅包含已在本轮修复完成的可验证问题。
  - [ ] 审核结果明确覆盖 Task 1-6 与结构断言结果。

  **QA Scenario**：
  ```
  Scenario: 审核实现与计划逐项一致
    Tool: task + Bash
    Steps:
      1. 调度上面的 oracle 审核任务。
      2. 等待完成后读取输出。
      3. 若 FAIL，按输出修复后重新运行 `pio test -e test` 与 `pio run -e esp32s3`，再重新调度 F1。
    Expected: 最终 oracle 输出 PASS，且不再指出遗漏任务、遗漏测试或遗漏结构约束。
    Evidence: .sisyphus/evidence/f1-plan-compliance.txt
  ```

- [ ] F2. Code Quality Review

  **Dispatch**：`task(category="unspecified-high", load_skills=[], run_in_background=true, description="Code quality review", prompt="Review the completed branch changes for code quality in src/ui_bridge and src/app/config_manager.*. Check naming consistency, dead code, hidden regressions, misuse of LVGL APIs, and maintainability issues. Return PASS/FAIL with concrete file/line findings only.")`

  **Acceptance Criteria**：
  - [ ] 审核结果为 PASS。
  - [ ] 没有未处理的高严重度代码质量问题。

  **QA Scenario**：
  ```
  Scenario: 代码质量审查通过
    Tool: task + Bash
    Steps:
      1. 调度上面的 unspecified-high 审核任务。
      2. 读取输出并修复 FAIL 项。
      3. 修复后重新运行 `pio run -e esp32s3`，必要时重跑 `pio test -e test`，再重新调度 F2。
    Expected: 最终输出 PASS，且无新的编译/测试回归。
    Evidence: .sisyphus/evidence/f2-code-quality.txt
  ```

- [ ] F3. Real Manual QA

  **Dispatch**：`task(category="unspecified-high", load_skills=[], run_in_background=true, description="Firmware QA review", prompt="Perform repo-level QA for the completed performance changes. Validate that the documented verification commands and source-structure assertions are sufficient, that settings_ui interaction semantics were preserved in code, and that no verification gap remains. Return PASS/FAIL with concrete evidence requirements only.")`

  **Acceptance Criteria**：
  - [ ] 审核结果为 PASS。
  - [ ] 明确确认验证命令、源码断言、设置页交互语义三类证据都齐全。

  **QA Scenario**：
  ```
  Scenario: QA 证据链完整
    Tool: task + Bash
    Steps:
      1. 调度上面的 unspecified-high QA 审核任务。
      2. 若指出证据缺口，补跑对应命令或补充对应结构断言后重新调度 F3。
    Expected: 最终输出 PASS，并确认 `pio test -e test`、`pio run -e esp32s3`、关键源码断言三类证据完整。
    Evidence: .sisyphus/evidence/f3-qa-review.txt
  ```

- [ ] F4. Scope Fidelity Check

  **Dispatch**：`task(category="deep", load_skills=[], run_in_background=true, description="Scope fidelity check", prompt="Check completed branch changes against .sisyphus/plans/high-priority-performance-plan.md and confirm scope fidelity. Fail if the branch touches unrelated medium/low-priority performance items, generated UI files, or unrelated subsystems such as network, fan, or power logic beyond required call-site updates.")`

  **Acceptance Criteria**：
  - [ ] 审核结果为 PASS。
  - [ ] 没有越界修改中低优先级项、生成文件或无关子系统。

  **QA Scenario**：
  ```
  Scenario: 范围无蔓延
    Tool: task + Bash
    Steps:
      1. 调度上面的 deep 审核任务。
      2. 若 FAIL，撤回越界改动或拆回对应任务范围，重新运行验证命令后再次调度 F4。
    Expected: 最终输出 PASS，且只保留计划内五个热点及其必需调用点变更。
    Evidence: .sisyphus/evidence/f4-scope-fidelity.txt
  ```

## Commit Strategy
- 每个实现任务完成后各自独立提交，避免把 `power_history`、`config_manager`、`settings_ui`、`data_bridge` 的风险混在一个提交中。
- 提交顺序固定：Task 1 → Task 4 → Task 2 → Task 5 → Task 3 → Task 6。
- 若 Task 2 或 Task 3 需要补修 Task 1 的接口，仅在对应任务内顺手包含所需最小补丁，不回退任务边界。
- 最终整合提交仅用于“所有高优先级性能路径已完成并验证”的汇总说明；不要跳过中间提交。

## Success Criteria
- 图表上游读取路径不再依赖 `s_linear_temp` 全量副本，且在 Task 2 完成后旧的 `power_history_get_range()` 已被移除。
- 图表每次刷新最多处理 300 个点，且窗口/颜色/Y 轴语义保持不变。
- `data_bridge` 不再每 200 ms 无条件刷新全部 subject。
- 设置页切换不再通过 `lv_obj_clean + 全量重建` 完成。
- 单次设置提交只发生一次显式 NVS 写盘。
- 全部验证命令与源码结构断言均有证据输出。