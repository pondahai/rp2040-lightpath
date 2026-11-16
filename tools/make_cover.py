"""產生 lightpath 的 96x96 封面：一台米色雷射機射出紅光，打在支架上的鏡子後轉向上方。

依 rp2040-retro-handheld docs/ICON-STYLE.md：具體物件、淺色背景、填滿畫面、
硬邊明暗（受光左上）、深褐描邊、一個暖色重點（紅色光束）。
輸出 assets/cover.png（檢視用）與 assets/LIGHTPATH.RAW（RGB565 big-endian）。

    python tools/make_cover.py      （需要 Pillow）
"""
import os
import struct
from PIL import Image, ImageDraw

W = H = 96
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets")

BG      = (246, 244, 238)
OUTLINE = (58, 48, 44)
SHELL_L = (222, 214, 198)
SHELL   = (196, 186, 166)   # #C4BAA6
SHELL_D = (150, 140, 122)
PANEL   = (92, 84, 78)
PANEL_D = (64, 56, 52)
LED     = (120, 220, 110)
METAL_L = (226, 228, 232)
METAL   = (178, 182, 190)
METAL_D = (120, 124, 132)
GLASS_L = (236, 246, 252)
GLASS   = (176, 206, 226)
GLASS_D = (112, 146, 172)
BEAM_O  = (196, 40, 30)
BEAM    = (236, 70, 48)
BEAM_HI = (255, 196, 170)

img = Image.new("RGB", (W, H), BG)
d = ImageDraw.Draw(img)


def box(x0, y0, x1, y1, fill, light, dark):
    d.rectangle([x0, y0, x1, y1], fill=fill)
    d.line([x0, y0, x1, y0], fill=light)
    d.line([x0, y0, x0, y1], fill=light)
    d.line([x0, y1, x1, y1], fill=dark)
    d.line([x1, y0, x1, y1], fill=dark)


def outline(x0, y0, x1, y1):
    d.rectangle([x0 - 1, y0 - 1, x1 + 1, y1 + 1], outline=OUTLINE)


def beam(points):
    """紅色光束：外框暗紅、本體紅、中間亮芯，寬 5 px"""
    d.line(points, fill=BEAM_O, width=7)
    d.line(points, fill=BEAM, width=5)
    d.line(points, fill=BEAM_HI, width=1)


# ---- 光束先畫（被機器和鏡子蓋住的部分自然被擋掉）----
# 雷射口 (40, 63) 往右到鏡子中心 (73, 63)，再往上出畫面
beam([(40, 63), (73, 63), (73, 2)])

# ---- 雷射機（左）：米色機身 + 深色面板 + 前端鏡筒 ----
outline(4, 46, 38, 82)
box(4, 46, 38, 82, SHELL, SHELL_L, SHELL_D)
d.rectangle([4, 46, 38, 49], fill=SHELL_L)            # 頂面亮帶
d.line([4, 50, 38, 50], fill=SHELL_D)
outline(9, 55, 27, 70)
box(9, 55, 27, 70, PANEL, PANEL, PANEL_D)              # 面板
for i, y in enumerate((58, 62, 66)):                   # 面板上的刻度
    d.line([12, y, 20 + 2 * i, y], fill=SHELL_D)
d.rectangle([23, 58, 25, 60], fill=LED)                # 電源燈
d.rectangle([10, 74, 32, 77], fill=SHELL_D)            # 散熱槽
for x in range(11, 32, 3):
    d.line([x, 74, x, 77], fill=PANEL_D)
# 鏡筒
outline(39, 58, 46, 68)
box(39, 58, 46, 68, METAL, METAL_L, METAL_D)
d.rectangle([44, 61, 46, 65], fill=OUTLINE)            # 出光口
# 腳
outline(7, 83, 14, 88)
box(7, 83, 14, 88, SHELL_D, SHELL, PANEL_D)
outline(28, 83, 35, 88)
box(28, 83, 35, 88, SHELL_D, SHELL, PANEL_D)

# ---- 鏡子（右）：支架 + 斜放的圓鏡 ----
# 底座與立柱
outline(60, 86, 88, 91)
box(60, 86, 88, 91, METAL, METAL_L, METAL_D)
outline(72, 72, 76, 85)
box(72, 72, 76, 85, METAL, METAL_L, METAL_D)
# 鏡框：45° 的長條，從左下 (60,76) 到右上 (86,50)
frame = [(58, 74), (62, 78), (88, 52), (84, 48)]
d.polygon(frame, fill=METAL_D, outline=OUTLINE)
glass = [(61, 74), (63, 76), (86, 53), (84, 51)]
d.polygon(glass, fill=GLASS)
d.line([(62, 74), (84, 52)], fill=GLASS_L)             # 受光左上的亮邊
d.line([(64, 76), (86, 54)], fill=GLASS_D)
# 光打在鏡面的亮點
d.ellipse([70, 60, 76, 66], fill=BEAM_HI, outline=BEAM)

os.makedirs(OUT, exist_ok=True)
img.save(os.path.join(OUT, "cover.png"))

with open(os.path.join(OUT, "LIGHTPATH.RAW"), "wb") as f:
    px = img.tobytes()
    for i in range(0, len(px), 3):
        r, g, b = px[i], px[i + 1], px[i + 2]
        f.write(struct.pack(">H", ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)))

print("colors:", len(img.getcolors(9999)))
print(os.path.join(OUT, "LIGHTPATH.RAW"))
