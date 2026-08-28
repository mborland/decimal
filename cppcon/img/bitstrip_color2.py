"""
Redraw float_0p1.png with bit VALUE encoded as cell colour instead of a
digit glyph, plus a legend.

Geometry is measured from the existing img/float_0p1.png so the new strip
drops into pair_0p1.png without reflowing anything:

    left margin      30      cell 60 x 89, 2px gap
    field gap        14      bar top 210 (was h=9)
    cell top        108      canvas 2505 x 294
"""
import struct
from PIL import Image, ImageDraw, ImageFont

FD = "/usr/share/fonts/truetype/dejavu"
OUT = "/mnt/user-data/outputs/img"

# --- measured geometry -----------------------------------------------------
W, H = 2505, 294
LEFT, CELL_W, CELL_H, CELL_GAP, FIELD_GAP = 30, 60, 89, 2, 14
CELL_TOP = 108
BAR_TOP, BAR_H = 210, 14          # bar thickened 9 -> 14, it is now the only
LABEL_TOP = 232                   # per-field cue left in the strip
ANNOT_BASE = 67
LEGEND_TOP = 62

# --- palette ---------------------------------------------------------------
TEXT = (242, 244, 246)            # #F2F4F6, unchanged
F_SIGN = (232, 234, 237)          # field bar colours, unchanged
F_EXPO = (58, 143, 208)
F_SIGD = (201, 184, 154)

OFF_G  = (107, 112, 118)          # #6B7076  neutral grey, off the background hue
OFF_GL = (115, 119, 128)          # #737780  one step lighter
ON_A = (232, 163, 61)             # #E8A33D  amber, as shipped
ON_C = (242, 174, 74)             # #F2AE4A  amber, one step brighter

FONTS = {
    "label": ImageFont.truetype(f"{FD}/DejaVuSans-Bold.ttf", 40),
    "mono": ImageFont.truetype(f"{FD}/DejaVuSansMono.ttf", 28),
    "value": ImageFont.truetype(f"{FD}/DejaVuSansMono.ttf", 42),
    "legend": ImageFont.truetype(f"{FD}/DejaVuSansMono-Bold.ttf", 30),
}


def f32_bits(x):
    w = struct.unpack(">I", struct.pack(">f", x))[0]
    return format(w, "032b"), w


def draw(fname, value, on, off, note, tail):
    bits, word = f32_bits(value)
    fields = [(1, F_SIGN, "Sign"), (8, F_EXPO, "Exponent"), (23, F_SIGD, "Significand")]

    im = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    x, i = LEFT, 0
    for n, bar, label in fields:
        x0 = x
        for _ in range(n):
            d.rectangle([x, CELL_TOP, x + CELL_W - 1, CELL_TOP + CELL_H - 1],
                        fill=on if bits[i] == "1" else off)
            x += CELL_W + CELL_GAP
            i += 1
        x1 = x - CELL_GAP - 1
        d.rectangle([x0, BAR_TOP, x1, BAR_TOP + BAR_H - 1], fill=bar)
        d.text(((x0 + x1) // 2, LABEL_TOP), label, font=FONTS["label"],
               fill=TEXT, anchor="ma")
        x += FIELD_GAP - CELL_GAP
    strip_right = x - FIELD_GAP

    # legend, top-left over the sign/exponent fields
    lx = LEFT
    for swatch, text in ((on, "1"), (off, "0")):
        d.rectangle([lx, LEGEND_TOP, lx + 33, LEGEND_TOP + 33], fill=swatch)
        d.text((lx + 44, LEGEND_TOP + 1), f"= {text}", font=FONTS["legend"], fill=TEXT)
        lx += 150

    # repeating-expansion note, right-aligned to the end of the significand
    d.text((strip_right, ANNOT_BASE), note, font=FONTS["mono"], fill=TEXT, anchor="ra")

    # the value, to the right of the strip
    d.text((strip_right + 35, CELL_TOP + CELL_H // 2), tail,
           font=FONTS["value"], fill=TEXT, anchor="lm")

    im.save(f"{OUT}/{fname}", "PNG")
    print(f"  {fname}  0x{word:08X}  {im.width}x{im.height}")


if __name__ == "__main__":
    import os
    os.makedirs(OUT, exist_ok=True)
    NOTE = "0.00011001100110011...  repeats forever"
    print("generating:")
    draw("float_0p1_grey.png",  0.1, ON_A, OFF_G,  NOTE, "~ 0.1")
    draw("float_0p1_grey2.png", 0.1, ON_C, OFF_GL, NOTE, "~ 0.1")
