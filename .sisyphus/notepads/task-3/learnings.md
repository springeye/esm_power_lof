# learnings

- `src/fan/fan_curve.cpp` 应使用相对当前目录的头文件引用 `"fan_curve.h"`，而不是 `"fan/fan_curve.h"`。
- 风扇曲线参数已迁移到 `config_manager` 运行时 getter 后，`fan_temp_to_pwm()` 应在函数内读取最新配置，避免依赖编译期宏。
- 10-bit PWM 换算统一按 `percent * 1023 / 100` 计算，`mid_duty` 不再硬编码。
- 堵转保护阈值也应并入 `config_manager::TempProtectionConfig`，避免 `fault_guard.cpp` 混用编译期宏和运行时配置。
- `platformio.ini` 现阶段仅保留 `esp32s3`/`esp32dev` 环境，删除 native env 后需要同步修正文档和后续脚本预期。
