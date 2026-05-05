# decisions

- `fan_temp_to_pwm()` 保持签名不变，仅替换内部阈值与 PWM 取值来源。
- `max_duty` 直接使用 `1023u`，不再通过配置 getter 间接获取。
- 堵转保护三个阈值（RPM / duty / timeout）统一收口到 `config_manager`，`fault_guard.cpp` 只负责判定逻辑，不再保留本地硬编码常量。
- `config_manager` 对堵转配置做范围钳制：RPM 0-5000、duty 0-1023、timeout 100-60000ms，默认值分别为 100 / 307 / 3000。
