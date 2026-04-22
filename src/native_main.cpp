#if !defined(ARDUINO) && !defined(PIO_UNIT_TESTING)
// Native 仿真入口：在 PC 上跑一遍 NTC→fan_curve→psu_fsm→keys 数据流，便于不上板调试。
// 用法示例：
//   program.exe                       使用默认参数 (adc=2048, pwok=1, keys="S")
//   program.exe --adc 1500            指定 NTC ADC 原始值 (0..4095)
//   program.exe --adc 1500 --pwok 0   同时指定 PWOK 引脚电平
//   program.exe --keys "S,L,S"        模拟按键序列 (S=短按, L=长按, _=空闲)
//
// 设计目标：纯函数式串一遍核心算法，输出每步中间结果，方便对照单元测试和实机现象。

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "sensors/ntc/ntc.h"
#include "fan/fan_curve.h"
#include "power/psu_fsm.h"
#include "input/keys.h"

namespace {

const char *psu_state_name(PsuState s) {
    switch (s) {
        case PSU_OFF:      return "OFF";
        case PSU_STANDBY:  return "STANDBY";
        case PSU_STARTING: return "STARTING";
        case PSU_ON:       return "ON";
        case PSU_STOPPING: return "STOPPING";
        case PSU_FAULT:    return "FAULT";
        default:           return "?";
    }
}

const char *key_event_name(KeyEvent e) {
    switch (e) {
        case KEY_IDLE:  return "IDLE";
        case KEY_SHORT: return "SHORT";
        case KEY_LONG:  return "LONG";
        default:        return "?";
    }
}

struct CliArgs {
    uint16_t adc = 2048;
    bool pwok = true;
    std::string keys = "S";  // 逗号分隔: S=短按事件, L=长按事件, _=空闲一拍
};

bool parse_args(int argc, char **argv, CliArgs &out) {
    for (int i = 1; i < argc; ++i) {
        const char *a = argv[i];
        if ((!std::strcmp(a, "--adc") || !std::strcmp(a, "-a")) && i + 1 < argc) {
            int v = std::atoi(argv[++i]);
            if (v < 0) v = 0;
            if (v > 4095) v = 4095;
            out.adc = static_cast<uint16_t>(v);
        } else if ((!std::strcmp(a, "--pwok") || !std::strcmp(a, "-p")) && i + 1 < argc) {
            out.pwok = std::atoi(argv[++i]) != 0;
        } else if ((!std::strcmp(a, "--keys") || !std::strcmp(a, "-k")) && i + 1 < argc) {
            out.keys = argv[++i];
        } else if (!std::strcmp(a, "--help") || !std::strcmp(a, "-h")) {
            return false;
        } else {
            std::fprintf(stderr, "未知参数: %s\n", a);
            return false;
        }
    }
    return true;
}

void print_usage() {
    std::puts("用法: program [--adc 0..4095] [--pwok 0|1] [--keys S,L,_,...]");
    std::puts("  --adc, -a    NTC ADC 原始值 (默认 2048)");
    std::puts("  --pwok, -p   PWOK 引脚电平 (默认 1)");
    std::puts("  --keys, -k   按键事件序列, 逗号分隔: S=短按 L=长按 _=空闲 (默认 S)");
}

std::vector<KeyEvent> parse_key_seq(const std::string &s) {
    std::vector<KeyEvent> seq;
    std::string token;
    for (size_t i = 0; i <= s.size(); ++i) {
        char c = (i < s.size()) ? s[i] : ',';
        if (c == ',' || c == ' ') {
            if (!token.empty()) {
                if (token == "S" || token == "s") seq.push_back(KEY_SHORT);
                else if (token == "L" || token == "l") seq.push_back(KEY_LONG);
                else seq.push_back(KEY_IDLE);
                token.clear();
            }
        } else {
            token.push_back(c);
        }
    }
    return seq;
}

}

int main(int argc, char **argv) {
    CliArgs args;
    if (!parse_args(argc, argv, args)) {
        print_usage();
        return 1;
    }

    std::puts("=== ESM Power LOF 原生仿真 ===");
    std::printf("输入: adc=%u  pwok=%d  keys=\"%s\"\n\n",
                args.adc, args.pwok ? 1 : 0, args.keys.c_str());

    // 1) NTC ADC -> 温度
    float temp_c = ntc_adc_to_temp(args.adc);
    std::printf("[NTC]      ADC %4u -> %.2f °C\n", args.adc, temp_c);

    // 2) 温度 -> 风扇 PWM (10-bit)
    uint16_t pwm = fan_temp_to_pwm(temp_c);
    float duty = (pwm * 100.0f) / 1023.0f;
    std::printf("[FAN]      %.2f °C -> PWM %u/1023 (%.1f%%)\n", temp_c, pwm, duty);

    // 3) PSU 状态机：BOOT 起步, 然后按 PWOK 派发事件, 再喂按键事件序列
    PsuState st = PSU_OFF;
    st = psu_fsm_transition(st, EVT_BOOT);
    std::printf("[PSU]      EVT_BOOT          -> %s\n", psu_state_name(st));

    PsuFsmEvent pwok_evt = args.pwok ? EVT_PWOK_HIGH : EVT_PWOK_LOW;
    st = psu_fsm_transition(st, pwok_evt);
    std::printf("[PSU]      %-17s -> %s\n",
                args.pwok ? "EVT_PWOK_HIGH" : "EVT_PWOK_LOW",
                psu_state_name(st));

    // 4) 按键序列：每个 token 直接当作 FSM 事件喂入 (跳过 KEY_IDLE)
    auto seq = parse_key_seq(args.keys);
    for (size_t i = 0; i < seq.size(); ++i) {
        KeyEvent ke = seq[i];
        std::printf("[KEY %zu]    %s", i, key_event_name(ke));
        if (ke == KEY_SHORT) {
            st = psu_fsm_transition(st, EVT_KEY_SHORT);
            std::printf(" -> EVT_KEY_SHORT -> PSU %s", psu_state_name(st));
        } else if (ke == KEY_LONG) {
            st = psu_fsm_transition(st, EVT_KEY_LONG);
            std::printf(" -> EVT_KEY_LONG  -> PSU %s", psu_state_name(st));
        }
        std::putchar('\n');
    }

    std::printf("\n[最终]     PSU=%s  PWM=%u  Temp=%.2f°C\n",
                psu_state_name(st), pwm, temp_c);
    return 0;
}

#endif  // !ARDUINO
