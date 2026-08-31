"""
Bit strips for the Boost.Decimal deck.

Rows, top to bottom:
    legend        1 / 0 swatches                    (top strip of a stack only)
    field names   coloured per field
    field sizes   coloured per field, "+" in the gaps, "= 32 bits" right
    rule          coloured span per field
    cells         bit value as fill, the value right

Cells are flat fill, no outline. The ONLY outlined cells in the deck are the
implied 0b100 bits on the steering-11 strip: outline means "this bit position
exists but is not stored", and nothing else uses that vocabulary.

No mathematics is drawn into these images. Every equation lives on the slide
as LaTeX so it can be coloured to match the fields and read at any size.

Palette
    1        #E8A33D  amber          0        #6B7076  neutral grey
    fields   Sign #E8EAED, Comb #D4789B, Exponent #3A8FD0, Significand #C9B89A
"""
import struct
from PIL import Image, ImageDraw, ImageFont

FD = "/usr/share/fonts/truetype/dejavu"
OUT = "/mnt/user-data/outputs/img"

MARGIN, GUTTER = 30, 215
LEFT = MARGIN + GUTTER
CELL_W, CELL_H, CELL_GAP, FIELD_GAP = 60, 89, 2, 14
RIGHT_PAD, RIGHT_OFF = 310, 35
RULE_H, GHOST_W = 6, 5
SWATCH, SWATCH_GAP = 64, 20
CAP_H, CAP_GAP = 56, 34

TEXT = (242, 244, 246)
ONE = (232, 163, 61)
ZERO = (107, 112, 118)
SIGN = (232, 234, 237)
COMB = (212, 120, 155)
EXPO = (58, 143, 208)
SIGD = (201, 184, 154)

F_NAME = ImageFont.truetype(f"{FD}/DejaVuSans-Bold.ttf", 31)
F_GHOST = ImageFont.truetype(f"{FD}/DejaVuSans-Bold.ttf", 26)
F_SIZE = ImageFont.truetype(f"{FD}/DejaVuSansMono-Bold.ttf", 40)
F_RIGHT = ImageFont.truetype(f"{FD}/DejaVuSansMono.ttf", 46)
F_LEGEND = ImageFont.truetype(f"{FD}/DejaVuSansMono-Bold.ttf", 46)
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


def draw(fname, word, fields, value, sizes=True, legend=True, implied=None):
    """implied: bit string drawn as outline-only cells before the last field."""
    bits = format(word, "032b")
    ghost = implied or ""

    y = 0
    y_name = y
    y += 48
    y_size = y
    if sizes:
        y += 58
    y_rule = y
    y += RULE_H + 12
    y_cell = y
    # the legend is a vertical key in the left gutter, centred on the cells
    key_h = 2 * SWATCH + SWATCH_GAP
    y_key = y_cell + (CELL_H - key_h) // 2
    overhang = max(0, y_key + key_h - (y_cell + CELL_H))
    height = y_cell + CELL_H + (overhang + 14 if legend else 14)

    # first pass: x extents, inserting the ghost run ahead of the last field
    cols, gaps, x, ghost_x = [], [], LEFT, None
    for idx, (count, _, _) in enumerate(fields):
        if ghost and idx == len(fields) - 1:
            ghost_x = x
            x += len(ghost) * (CELL_W + CELL_GAP)
        x0 = x
        x += count * (CELL_W + CELL_GAP)
        cols.append((x0, x - CELL_GAP - 1, (x0 + x - CELL_GAP - 1) // 2))
        gaps.append(x - CELL_GAP - 1 + FIELD_GAP // 2)
        x += FIELD_GAP - CELL_GAP
    right = cols[-1][1]

    im = Image.new("RGBA", (right + RIGHT_PAD, height), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    if legend:
        ky = y_key
        for swatch, t in ((ONE, "1"), (ZERO, "0")):
            d.rectangle([MARGIN, ky, MARGIN + SWATCH, ky + SWATCH], fill=swatch)
            d.text((MARGIN + SWATCH + 18, ky + SWATCH // 2), f"= {t}",
                   font=F_LEGEND, fill=TEXT, anchor="lm")
            ky += SWATCH + SWATCH_GAP

    for (count, label, colour), (x0, x1, cx) in zip(fields, cols):
        d.text((cx, y_name), label, font=F_NAME, fill=colour, anchor="ma")
        if sizes:
            d.text((cx, y_size), str(count), font=F_SIZE, fill=colour,
                   anchor="ma")
        d.rectangle([x0, y_rule, x1, y_rule + RULE_H - 1], fill=colour)

    if ghost:
        gx0 = ghost_x
        gx1 = ghost_x + len(ghost) * (CELL_W + CELL_GAP) - CELL_GAP - 1
        d.text(((gx0 + gx1) // 2, y_name + 4), "implied", font=F_GHOST,
               fill=SIGD, anchor="ma")
        x = ghost_x
        for b in ghost:
            d.rectangle([x, y_cell, x + CELL_W - 1, y_cell + CELL_H - 1],
                        outline=ONE if b == "1" else ZERO, width=GHOST_W)
            x += CELL_W + CELL_GAP

    x, i = LEFT, 0
    for idx, (count, _, _) in enumerate(fields):
        if ghost and idx == len(fields) - 1:
            x += len(ghost) * (CELL_W + CELL_GAP)
        for _ in range(count):
            d.rectangle([x, y_cell, x + CELL_W - 1, y_cell + CELL_H - 1],
                        fill=ONE if bits[i] == "1" else ZERO)
            x += CELL_W + CELL_GAP
            i += 1
        x += FIELD_GAP - CELL_GAP

    if sizes:
        for gx in gaps[:-1]:
            d.text((gx, y_size), "+", font=F_SIZE, fill=TEXT, anchor="ma")
        total = sum(c for c, _, _ in fields)
        d.text((right + RIGHT_OFF, y_size + 20), f"= {total} bits",
               font=F_RIGHT, fill=TEXT, anchor="lm")

    d.text((right + RIGHT_OFF, y_cell + CELL_H // 2), value, font=F_RIGHT,
           fill=TEXT, anchor="lm")

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
        d.text((MARGIN, y + 8), cap, font=F_CAP, fill=TEXT)
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
    draw("dec32_0p15625.png", w, dec32_fields(w), "= 0.15625", legend=False)

    draw("float_0p1.png", f32(0.1), FLOAT32, "~ 0.1", sizes=False)
    w = 0x32000001
    draw("dec32_0p1.png", w, dec32_fields(w), "= 0.1", sizes=False,
         legend=False)

    w = 0x32FFFFFF
    draw("dec32_8388607.png", w, dec32_fields(w), "= 8388607")
    w = 0x6CA00000
    draw("dec32_8388608.png", w, dec32_fields(w), "= 8388608", legend=False,
         implied="100")

    print("stacking:")
    stack("pair_0p15625.png", [("float", "float_0p15625.png"),
                               ("decimal32_t", "dec32_0p15625.png")])
    stack("pair_0p1.png", [("float", "float_0p1.png"),
                           ("decimal32_t", "dec32_0p1.png")])
    stack("boundary.png", [("decimal32_t{8388607}   steering != 11",
                            "dec32_8388607.png"),
                           ("decimal32_t{8388608}   steering == 11",
                            "dec32_8388608.png")])
