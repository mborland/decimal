"""
Bit strips with the bit VALUE carried by cell colour and a legend, replacing
the digit glyphs.

Geometry is measured off the shipped PNGs so these drop in without reflow:
    left 30, cell 60 x 89, 2px cell gap, 14px field gap, cells at y=108,
    bar at y=210, labels at y=232, canvas height 294,
    canvas width = end-of-strip + 470.
Stacking matches the old stack(): 56px caption band, 34px gap.

Palette
    1        #E8A33D  amber
    0        #6B7076  neutral grey (deliberately off the background's blue
                      axis -- the slide background is #111920)
    bars     Sign #E8EAED, Comb #D4789B, Exponent #3A8FD0, Significand #C9B89A
             Comb was #F0B23C, which is now 8.2 dE from the "1" cells.
"""
import struct
from PIL import Image, ImageDraw, ImageFont

FD = "/usr/share/fonts/truetype/dejavu"
OUT = "/mnt/user-data/outputs/img"

LEFT, CELL_W, CELL_H, CELL_GAP, FIELD_GAP = 30, 60, 89, 2, 14
CELL_TOP, CANVAS_H, RIGHT_PAD = 108, 294, 470
BAR_TOP, BAR_H, LABEL_TOP = 210, 14, 232
ANNOT_BASE, LEGEND_TOP = 67, 62
CAP_H, CAP_GAP = 56, 34

TEXT = (242, 244, 246)
ONE = (232, 163, 61)
ZERO = (107, 112, 118)
SIGN = (232, 234, 237)
COMB = (212, 120, 155)
EXPO = (58, 143, 208)
SIGD = (201, 184, 154)

F_LABEL = ImageFont.truetype(f"{FD}/DejaVuSans-Bold.ttf", 31)
F_CAP = ImageFont.truetype(f"{FD}/DejaVuSans-Bold.ttf", 38)
F_MONO = ImageFont.truetype(f"{FD}/DejaVuSansMono.ttf", 28)
F_VALUE = ImageFont.truetype(f"{FD}/DejaVuSansMono.ttf", 42)
F_LEGEND = ImageFont.truetype(f"{FD}/DejaVuSansMono-Bold.ttf", 30)

FLOAT32 = [(1, "Sign", SIGN), (8, "Exponent", EXPO), (23, "Significand", SIGD)]
DEC32 = [(1, "Sign", SIGN), (2, "Comb", COMB), (6, "Exponent", EXPO),
         (23, "Significand", SIGD)]
DEC32_S11 = [(1, "Sign", SIGN), (2, "Comb", COMB), (8, "Exponent", EXPO),
             (21, "Significand", SIGD)]


def f32(x):
    return struct.unpack(">I", struct.pack(">f", x))[0]


def dec32_fields(word):
    return DEC32_S11 if (word & 0x60000000) == 0x60000000 else DEC32


def strip_width(fields):
    n = sum(c for c, _, _ in fields)
    return (LEFT + n * (CELL_W + CELL_GAP) - CELL_GAP
            + (len(fields) - 1) * (FIELD_GAP - CELL_GAP)) - 1


def draw(fname, word, fields, tail, note=None, legend=True):
    bits = format(word, "032b")
    right = strip_width(fields)
    im = Image.new("RGBA", (right + RIGHT_PAD, CANVAS_H), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    x, i = LEFT, 0
    for count, label, bar in fields:
        x0 = x
        for _ in range(count):
            d.rectangle([x, CELL_TOP, x + CELL_W - 1, CELL_TOP + CELL_H - 1],
                        fill=ONE if bits[i] == "1" else ZERO)
            x += CELL_W + CELL_GAP
            i += 1
        x1 = x - CELL_GAP - 1
        d.rectangle([x0, BAR_TOP, x1, BAR_TOP + BAR_H - 1], fill=bar)
        d.text(((x0 + x1) // 2, LABEL_TOP), label, font=F_LABEL, fill=TEXT,
               anchor="ma")
        x += FIELD_GAP - CELL_GAP

    if legend:
        lx = LEFT
        for swatch, t in ((ONE, "1"), (ZERO, "0")):
            d.rectangle([lx, LEGEND_TOP, lx + 33, LEGEND_TOP + 33], fill=swatch)
            d.text((lx + 44, LEGEND_TOP + 1), f"= {t}", font=F_LEGEND, fill=TEXT)
            lx += 150

    if note:
        d.text((right, ANNOT_BASE), note, font=F_MONO, fill=TEXT, anchor="ra")
    d.text((right + 35, CELL_TOP + CELL_H // 2), tail, font=F_VALUE, fill=TEXT,
           anchor="lm")

    im.save(f"{OUT}/{fname}", "PNG")
    print(f"  {fname}  0x{word:08X}  {im.width}x{im.height}")
    return bits


def stack(fname, items):
    imgs = [Image.open(f"{OUT}/{f}") for _, f in items]
    W = max(i.width for i in imgs)
    H = sum(i.height + CAP_H for i in imgs) + CAP_GAP * (len(imgs) - 1)
    out = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(out)
    y = 0
    for (cap, _), im in zip(items, imgs):
        d.text((30, y + 8), cap, font=F_CAP, fill=TEXT)
        out.paste(im, (0, y + CAP_H), im)
        y += CAP_H + im.height + CAP_GAP
    out.save(f"{OUT}/{fname}", "PNG")
    print(f"  {fname}  {W}x{H}")


if __name__ == "__main__":
    import os
    os.makedirs(OUT, exist_ok=True)
    print("generating:")

    draw("float_0p15625.png", f32(0.15625), FLOAT32, "= 0.15625")
    w = 0x30003D09
    draw("dec32_0p15625.png", w, dec32_fields(w), "= 0.15625",
         note="0b11110100001001 = 15625   x 10^-5", legend=False)

    draw("float_0p1.png", f32(0.1), FLOAT32, "~ 0.1",
         note="0.00011001100110011...  repeats forever")
    w = 0x32000001
    draw("dec32_0p1.png", w, dec32_fields(w), "= 0.1",
         note="0b1 = 1   x 10^-1", legend=False)

    print("stacking:")
    stack("pair_0p15625.png", [("float", "float_0p15625.png"),
                               ("decimal32_t", "dec32_0p15625.png")])
    stack("pair_0p1.png", [("float", "float_0p1.png"),
                           ("decimal32_t", "dec32_0p1.png")])
