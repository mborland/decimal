"""
Bit strips, reviewer's layout.

Rows, top to bottom:
    legend            1 / 0 swatches            (top strip of a pair only)
    field names       coloured per field
    field sizes       coloured per field, "+" in the gaps, "= 32 bits" right
    rule              coloured span per field
    cells             bit value as fill, "= 0.15625" right
    note              optional, right-aligned under the cells

Cell metrics are unchanged from the shipped strips (60 x 89, 2px cell gap,
14px field gap, left margin 30) so the bits line up with the old images if
anything is ever diffed against them. Everything above and below the cells
is new. Vertical layout is computed from whichever rows are present.

Palette
    1        #E8A33D  amber          0        #6B7076  neutral grey
    outline  #F2F4F6  3px, drawn inward so the pitch does not change
    fields   Sign #E8EAED, Comb #D4789B, Exponent #3A8FD0, Significand #C9B89A
"""
import struct
from PIL import Image, ImageDraw, ImageFont

FD = "/usr/share/fonts/truetype/dejavu"
OUT = "/mnt/user-data/outputs/img"

LEFT, CELL_W, CELL_H, CELL_GAP, FIELD_GAP = 30, 60, 89, 2, 14
RIGHT_PAD, RIGHT_OFF = 310, 35
RULE_H, SWATCH = 6, 46
CAP_H, CAP_GAP = 56, 34
OUTLINE_W = 3

TEXT = (242, 244, 246)
OUTLINE = (242, 244, 246)
ONE = (232, 163, 61)
ZERO = (107, 112, 118)
SIGN = (232, 234, 237)
COMB = (212, 120, 155)
EXPO = (58, 143, 208)
SIGD = (201, 184, 154)

F_NAME = ImageFont.truetype(f"{FD}/DejaVuSans-Bold.ttf", 31)
F_SIZE = ImageFont.truetype(f"{FD}/DejaVuSansMono-Bold.ttf", 40)
F_RIGHT = ImageFont.truetype(f"{FD}/DejaVuSansMono.ttf", 46)
F_LEGEND = ImageFont.truetype(f"{FD}/DejaVuSansMono-Bold.ttf", 40)
F_NOTE = ImageFont.truetype(f"{FD}/DejaVuSansMono.ttf", 28)
F_CAP = ImageFont.truetype(f"{FD}/DejaVuSans-Bold.ttf", 38)

FLOAT32 = [(1, "Sign", SIGN), (8, "Exponent", EXPO), (23, "Significand", SIGD)]
DEC32 = [(1, "Sign", SIGN), (2, "Comb", COMB), (6, "Exponent", EXPO),
         (23, "Significand", SIGD)]
DEC32_S11 = [(1, "Sign", SIGN), (2, "Comb", COMB), (8, "Exponent", EXPO),
             (21, "Significand", SIGD)]


def f32(x):
    return struct.unpack(">I", struct.pack(">f", x))[0]


def dec32_fields(word):
    return DEC32_S11 if (word & 0x60000000) == 0x60000000 else DEC32


def spans(fields):
    """(x0, x1, centre) per field, plus the centre of each inter-field gap."""
    out, gaps, x = [], [], LEFT
    for count, _, _ in fields:
        x0 = x
        x += count * (CELL_W + CELL_GAP)
        x1 = x - CELL_GAP - 1
        out.append((x0, x1, (x0 + x1) // 2))
        gaps.append(x1 + FIELD_GAP // 2)
        x += FIELD_GAP - CELL_GAP
    return out, gaps[:-1], out[-1][1]


def draw(fname, word, fields, value, sizes=True, legend=True, note=None):
    bits = format(word, "032b")
    cols, gaps, right = spans(fields)

    y = 0
    y_legend = y
    if legend:
        y += SWATCH + 30
    y_name = y
    y += 48
    y_size = y
    if sizes:
        y += 58
    y_rule = y
    y += RULE_H + 12
    y_cell = y
    y += CELL_H
    y_note = y + 10
    height = (y_note + 34) if note else (y + 14)

    im = Image.new("RGBA", (right + RIGHT_PAD, height), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    if legend:
        lx = LEFT
        for swatch, t in ((ONE, "1"), (ZERO, "0")):
            d.rectangle([lx, y_legend, lx + SWATCH, y_legend + SWATCH],
                        fill=swatch, outline=OUTLINE, width=OUTLINE_W)
            d.text((lx + SWATCH + 16, y_legend + SWATCH // 2), f"= {t}",
                   font=F_LEGEND, fill=TEXT, anchor="lm")
            lx += 190

    for (count, label, colour), (x0, x1, cx) in zip(fields, cols):
        d.text((cx, y_name), label, font=F_NAME, fill=colour, anchor="ma")
        if sizes:
            d.text((cx, y_size), str(count), font=F_SIZE, fill=colour, anchor="ma")
        d.rectangle([x0, y_rule, x1, y_rule + RULE_H - 1], fill=colour)

    # cells
    x, i = LEFT, 0
    for count, _, _ in fields:
        for _ in range(count):
            d.rectangle([x, y_cell, x + CELL_W - 1, y_cell + CELL_H - 1],
                        fill=ONE if bits[i] == "1" else ZERO,
                        outline=OUTLINE, width=OUTLINE_W)
            x += CELL_W + CELL_GAP
            i += 1
        x += FIELD_GAP - CELL_GAP

    if sizes:
        for gx in gaps:
            d.text((gx, y_size), "+", font=F_SIZE, fill=TEXT, anchor="ma")
        total = sum(c for c, _, _ in fields)
        d.text((right + RIGHT_OFF, y_size + 21), f"= {total} bits",
               font=F_RIGHT, fill=TEXT, anchor="lm")

    d.text((right + RIGHT_OFF, y_cell + CELL_H // 2), value,
           font=F_RIGHT, fill=TEXT, anchor="lm")

    if note:
        d.text((right, y_note), note, font=F_NOTE, fill=TEXT, anchor="ra")

    im.save(f"{OUT}/{fname}", "PNG")
    print(f"  {fname}  0x{word:08X}  {im.width}x{im.height}")


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
    draw("dec32_0p15625.png", w, dec32_fields(w), "= 0.15625", legend=False,
         note="0b11110100001001 = 15625   x 10^-5")

    draw("float_0p1.png", f32(0.1), FLOAT32, "~ 0.1", sizes=False,
         note="0.00011001100110011...  repeats forever")
    w = 0x32000001
    draw("dec32_0p1.png", w, dec32_fields(w), "= 0.1", sizes=False,
         legend=False, note="0b1 = 1   x 10^-1")

    print("stacking:")
    stack("pair_0p15625.png", [("float", "float_0p15625.png"),
                               ("decimal32_t", "dec32_0p15625.png")])
    stack("pair_0p1.png", [("float", "float_0p1.png"),
                           ("decimal32_t", "dec32_0p1.png")])
