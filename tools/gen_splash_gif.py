#!/usr/bin/env python3
"""
gen_splash_gif.py - 生成 LOF Power System 开机闪电动画 GIF + 转 C 数组

输出：
  ui/images/splash.gif       - 预览 GIF
  ui/images/splash_gif.c     - LVGL lv_gif 控件可用的 C 数组

用法：
  python tools/gen_splash_gif.py
  python tools/gen_splash_gif.py --out ui/images

动画设计：
  240×280, 30帧, ~1.2s, 256色
  - 帧 0-6  : 黑底 → 闪电从顶部劈下（锯齿折线逐渐延伸）
  - 帧 7-12 : 闪电完全显现 + 金黄光晕脉冲
  - 帧 13-18: MORNSUN 字从透明淡入 + 向上微移
  - 帧 19-29: 定格（闪电+MORNSUN），末帧停留供 lv_gif 循环或切换 home
"""

import argparse
import math
import os
import struct
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

# ---------------------------------------------------------------------------
# 常量
# ---------------------------------------------------------------------------
W, H = 240, 280
N_FRAMES = 30
DURATION_MS = 1200
FRAME_MS = DURATION_MS // N_FRAMES  # 40ms/帧

BG = (0, 0, 0)
BOLT_COLOR = (255, 176, 32)       # 金黄 #FFB020
BOLT_GLOW = (255, 210, 80)        # 光晕
BRAND_COLOR = (255, 255, 255)     # 纯白
SLOGAN_COLOR = (107, 125, 142)    # 灰蓝 #6b7d8e

BOLT_PATH = [                     # 闪电锯齿点 (x, y) 从顶到底
    (120, 0), (110, 35), (130, 50), (105, 90),
    (135, 100), (100, 150), (140, 160), (108, 210),
]
BRAND_Y = 200                     # MORNSUN 字 y 基线
SLOGAN_Y = 230


def lerp(a: float, b: float, t: float) -> float:
    return a + (b - a) * max(0.0, min(1.0, t))


def color_alpha(base: tuple, alpha: float) -> tuple:
    """返回与黑色背景混合后的 RGB"""
    return tuple(int(c * alpha) for c in base)


def draw_bolt(draw: ImageDraw.ImageDraw, progress: float, glow: bool):
    """画闪电。progress: 0→1 闪电延伸比例；glow: 是否画光晕"""
    n = len(BOLT_PATH)
    visible = max(2, int(n * progress))

    if glow:
        for i in range(visible - 1):
            x1, y1 = BOLT_PATH[i]
            x2, y2 = BOLT_PATH[i + 1]
            draw.line([(x1, y1), (x2, y2)], fill=BOLT_GLOW, width=8)

    for i in range(visible - 1):
        x1, y1 = BOLT_PATH[i]
        x2, y2 = BOLT_PATH[i + 1]
        if i == visible - 2:
            # 最后一段按 progress 插值
            seg_t = (progress * n) - int(progress * n)
            if seg_t > 0:
                x2 = int(lerp(x1, x2, seg_t))
                y2 = int(lerp(y1, y2, seg_t))
        draw.line([(x1, y1), (x2, y2)], fill=BOLT_COLOR, width=4)


def draw_brand(img: Image.Image, alpha: float, y_offset: int):
    """画 MORNSUN 字（PIL 不支持原生透明文字，需 RGBA 合成）"""
    if alpha <= 0.01:
        return
    overlay = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)

    # 尝试找系统字体；fallback 到默认
    font = None
    for name in ["arialbd.ttf", "Arial Bold", "segoeui.ttf", "arial.ttf"]:
        try:
            font = ImageFont.truetype(name, 28)
            break
        except (OSError, IOError):
            continue
    if font is None:
        font = ImageFont.load_default()

    text = "MORNSUN"
    bbox = draw.textbbox((0, 0), text, font=font)
    tw = bbox[2] - bbox[0]
    x = (W - tw) // 2
    y = BRAND_Y + y_offset
    a = int(255 * alpha)
    draw.text((x, y), text, fill=(255, 255, 255, a), font=font)
    img.paste(Image.alpha_composite(Image.new("RGBA", (W, H), (0, 0, 0, 0)), overlay), (0, 0),
              mask=overlay.split()[3])


def draw_slogan(img: Image.Image, alpha: float):
    """画 Power System Inside"""
    if alpha <= 0.01:
        return
    overlay = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    font = None
    for name in ["arial.ttf", "segoeui.ttf", "Arial"]:
        try:
            font = ImageFont.truetype(name, 14)
            break
        except (OSError, IOError):
            continue
    if font is None:
        font = ImageFont.load_default()
    text = "Power System Inside"
    bbox = draw.textbbox((0, 0), text, font=font)
    tw = bbox[2] - bbox[0]
    x = (W - tw) // 2
    a = int(255 * alpha)
    draw.text((x, SLOGAN_Y), text, fill=(*SLOGAN_COLOR, a), font=font)
    img.paste(Image.alpha_composite(Image.new("RGBA", (W, H), (0, 0, 0, 0)), overlay), (0, 0),
              mask=overlay.split()[3])


def generate_frames() -> list[Image.Image]:
    """生成所有帧"""
    frames: list[Image.Image] = []

    for f in range(N_FRAMES):
        t = f / (N_FRAMES - 1)  # 0..1

        img = Image.new("RGB", (W, H), BG)
        draw = ImageDraw.Draw(img)

        # --- 闪电阶段 ---
        bolt_start, bolt_end = 0.0, 0.4
        if t < bolt_end:
            progress = (t - bolt_start) / (bolt_end - bolt_start)
        else:
            progress = 1.0
        glow = t > 0.25  # 光晕在闪电延伸后出现
        draw_bolt(draw, progress, glow)

        # --- MORNSUN 字阶段 ---
        brand_start, brand_end = 0.35, 0.6
        if t < brand_start:
            brand_alpha = 0.0
        elif t < brand_end:
            brand_alpha = (t - brand_start) / (brand_end - brand_start)
        else:
            brand_alpha = 1.0
        brand_y_off = int(10 * (1.0 - brand_alpha))  # 向上微移
        draw_brand(img, brand_alpha, brand_y_off)

        # --- Slogan 阶段 ---
        slogan_start, slogan_end = 0.55, 0.8
        if t < slogan_start:
            slogan_alpha = 0.0
        elif t < slogan_end:
            slogan_alpha = (t - slogan_start) / (slogan_end - slogan_start)
        else:
            slogan_alpha = 1.0
        draw_slogan(img, slogan_alpha)

        frames.append(img)

    return frames


def gif_to_c_array(gif_path: str, out_path: str, var_name: str = "splash_gif"):
    """将 GIF 文件转为 LVGL lv_gif 控件可用的 C 数组（lv_image_dsc_t 格式）"""
    with open(gif_path, "rb") as f:
        data = f.read()

    size = len(data)
    lines = [
        '/* Auto-generated by tools/gen_splash_gif.py -- DO NOT EDIT */',
        '#ifdef __has_include',
        '#  if __has_include("lvgl/lvgl.h")',
        '#    include "lvgl/lvgl.h"',
        '#  elif __has_include("lvgl.h")',
        '#    include "lvgl.h"',
        '#  else',
        '#    error "Cannot find lvgl.h"',
        '#  endif',
        '#else',
        '#  include "lvgl/lvgl.h"',
        '#endif',
        '',
        f'#define {var_name.upper()}_SIZE {size}',
        '',
        f'static const uint8_t {var_name}_map[{size}] __attribute__((aligned(4))) = {{',
    ]

    for i in range(0, size, 16):
        chunk = data[i:i + 16]
        hex_vals = ", ".join(f"0x{b:02x}" for b in chunk)
        comma = "," if i + 16 < size else ""
        lines.append(f"    {hex_vals}{comma}")

    lines.append("};")
    lines.append("")
    lines.append("const lv_image_dsc_t splash_gif_dsc = {")
    lines.append("    .header = {")
    lines.append("        .magic = LV_IMAGE_HEADER_MAGIC,")
    lines.append("        .cf = LV_COLOR_FORMAT_RAW,")
    lines.append("        .flags = 0,")
    lines.append(f"        .w = {W},")
    lines.append(f"        .h = {H},")
    lines.append("        .stride = 0,")
    lines.append("        .reserved_2 = 0,")
    lines.append("    },")
    lines.append(f"    .data_size = {size},")
    lines.append(f"    .data = {var_name}_map,")
    lines.append("    .reserved = NULL,")
    lines.append("    .reserved_2 = NULL,")
    lines.append("};")
    lines.append("")

    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))

    return size


def main():
    parser = argparse.ArgumentParser(description="生成 LOF Power System 开机闪电动画 GIF")
    parser.add_argument("--out", default="ui/images", help="输出目录 (default: ui/images)")
    parser.add_argument("--frames", type=int, default=N_FRAMES, help="帧数")
    parser.add_argument("--duration", type=int, default=DURATION_MS, help="总时长 ms")
    args = parser.parse_args()

    out_dir = Path(args.out)
    gif_path = out_dir / "splash.gif"
    c_path = out_dir / "splash_gif.c"

    print(f"[gen_splash_gif] 生成 {W}×{H} {args.frames}帧 GIF...")
    frames = generate_frames()
    print(f"[gen_splash_gif] 保存 GIF → {gif_path}")
    frames[0].save(
        str(gif_path),
        save_all=True,
        append_images=frames[1:],
        duration=args.duration // args.frames,
        loop=0,           # 无限循环
        optimize=False,   # 保持颜色
    )

    gif_size = gif_path.stat().st_size
    print(f"[gen_splash_gif] GIF 大小: {gif_size} bytes ({gif_size/1024:.1f} KB)")

    print(f"[gen_splash_gif] 转 C 数组 → {c_path}")
    c_size = gif_to_c_array(str(gif_path), str(c_path))
    print(f"[gen_splash_gif] C 数组大小: {c_size} bytes ({c_size/1024:.1f} KB)")
    print("[gen_splash_gif] 完成!")


if __name__ == "__main__":
    main()
