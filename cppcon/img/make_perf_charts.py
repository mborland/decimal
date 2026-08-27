"""Generate all benchmark figures for the performance section.

Three figure families, all Option B small-multiple horizontal bars:
  1. vendor_chart   - per width, vs same-width decimalN_t (x86 configs)
  2. boostonly_chart - all six Boost types vs hardware double (ARM configs)
  3. charconv_chart - from_chars/to_chars vs double, float+double included

verify() cross-checks every transcribed runtime against the docs'
published ratio-to-double column before anything is drawn.

Transparent backgrounds, near-white ink -> drops onto the dark Deckset
theme like the existing bit-strip figures. Palette lives in perf_data.py.
"""

import os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

from perf_data import (CONFIGS, CHARCONV, OPS, CC_OPS,
                       INK, AMBER, TAN, BLUE, GRAY)

plt.rcParams.update({
    "figure.facecolor": "none", "axes.facecolor": "none",
    "savefig.transparent": True,
    "text.color": INK, "axes.edgecolor": INK,
    "xtick.color": INK, "ytick.color": INK,
    "font.family": "DejaVu Sans", "font.size": 12,
})
MONO = {"family": "DejaVu Sans Mono"}

# ------------------------------------------------------------------ verify
def verify():
    n = 0
    def check(rt, ratio, where):
        nonlocal n
        for op in rt:
            for k in rt[op]:
                derived = rt[op][k] / rt[op]["double"]
                want = ratio[op][k]
                tol = max(0.002, want * 0.006)
                assert abs(derived - want) <= tol, (
                    f"{where}/{op}/{k}: derived {derived:.3f} "
                    f"vs docs {want:.3f}")
                n += 1
    for name, cfg in CONFIGS.items():
        check(cfg["rt"], cfg["ratio"], name)
    for name, cc in CHARCONV.items():
        check(cc["rt"], cc["ratio"], "charconv/" + name)
    print(f"verify: {n} entries consistent with the docs ratio column")

# ------------------------------------------------------------- helpers
def fmt(v):
    if v < 10:
        return f"{v:.2f}\u00d7"
    if v < 100:
        return f"{v:.1f}\u00d7"
    return f"{v:.0f}\u00d7"

def strip_axes(ax):
    ax.tick_params(axis="both", length=0)
    ax.set_xticks([])
    for s in ax.spines.values():
        s.set_visible(False)

def caption(fig, text):
    fig.text(0.5, 0.015, text, ha="center", fontsize=11, alpha=0.8)

# ------------------------------------------------------- vendor figures
def vendor_chart(cfg, width, path):
    rows = [("i" + width, f"Intel BID_UINT{width}", BLUE)]
    if "g" in cfg["vendors"]:
        rows.append(("g" + width, f"GCC _Decimal{width}", TAN))
    rows += [("d" + width, f"decimal{width}_t", AMBER),
             ("f" + width, f"decimal_fast{width}_t", INK)]
    rt = cfg["rt"]
    base = "d" + width
    fig, axes = plt.subplots(1, len(OPS),
                             figsize=(12.8, 3.5 + 0.4 * len(rows)),
                             sharey=True)
    xmax = max(rt[op][k] / rt[op][base] for op, _ in OPS
               for k, _, _ in rows) * 1.18
    ys = np.arange(len(rows))
    for ax, (op, label) in zip(axes, OPS):
        rel = [rt[op][k] / rt[op][base] for k, _, _ in rows]
        ax.barh(ys, rel, height=0.62, color=[c for _, _, c in rows])
        for y, v in zip(ys, rel):
            ax.text(v + xmax * 0.02, y, f"{v:.2f}\u00d7", va="center",
                    fontsize=10.5, **MONO)
        ax.axvline(1.0, ls="--", lw=1.0, color=INK, alpha=0.45)
        ax.set_title(label, fontsize=13, pad=8)
        ax.set_xlim(0, xmax)
        ax.set_yticks(ys, [n for _, n, _ in rows], fontsize=11.5, **MONO)
        strip_axes(ax)
    caption(fig, f"{cfg['caption']} \u00b7 runtime relative to "
                 f"decimal{width}_t = 1.00\u00d7 (dashed) \u00b7 "
                 f"shorter is faster")
    fig.tight_layout(rect=(0, 0.05, 1, 1))
    fig.savefig(path, dpi=200)
    plt.close(fig)

# ---------------------------------------------------- Boost-only figures
BOOST_ROWS = [("d128", "decimal128_t", AMBER),
              ("f128", "decimal_fast128_t", INK),
              ("d64", "decimal64_t", AMBER),
              ("f64", "decimal_fast64_t", INK),
              ("d32", "decimal32_t", AMBER),
              ("f32", "decimal_fast32_t", INK)]

def boostonly_chart(cfg, path):
    ratio = cfg["ratio"]
    fig, axes = plt.subplots(1, len(OPS), figsize=(12.8, 4.7), sharey=True)
    ys = np.arange(len(BOOST_ROWS))
    for ax, (op, label) in zip(axes, OPS):
        vals = [ratio[op][k] for k, _, _ in BOOST_ROWS]
        xmax = max(vals) * 1.30
        ax.barh(ys, vals, height=0.62, color=[c for _, _, c in BOOST_ROWS])
        for y, v in zip(ys, vals):
            ax.text(v + xmax * 0.02, y, fmt(v), va="center",
                    fontsize=10, **MONO)
        for sep in (1.5, 3.5):
            ax.axhline(sep, color=INK, lw=0.8, alpha=0.22)
        ax.set_title(label, fontsize=13, pad=8)
        ax.set_xlim(0, xmax)
        ax.set_yticks(ys, [n for _, n, _ in BOOST_ROWS],
                      fontsize=11.5, **MONO)
        strip_axes(ax)
    caption(fig, f"{cfg['caption']} \u00b7 runtime relative to hardware "
                 f"double = 1.00\u00d7 \u00b7 shorter is faster \u00b7 "
                 f"panels scaled independently")
    fig.tight_layout(rect=(0, 0.05, 1, 1))
    fig.savefig(path, dpi=200)
    plt.close(fig)

# ------------------------------------------------------ charconv figures
CC_ROWS = [("d128", "decimal128_t", AMBER),
           ("f128", "decimal_fast128_t", INK),
           ("d64", "decimal64_t", AMBER),
           ("f64", "decimal_fast64_t", INK),
           ("d32", "decimal32_t", AMBER),
           ("f32", "decimal_fast32_t", INK),
           ("double", "double", GRAY),
           ("float", "float", GRAY)]

def charconv_chart(cc, path):
    ratio = cc["ratio"]
    fig, axes = plt.subplots(1, len(CC_OPS), figsize=(12.8, 5.3), sharey=True)
    xmax = max(ratio[op][k] for op, _ in CC_OPS
               for k, _, _ in CC_ROWS) * 1.24
    ys = np.arange(len(CC_ROWS))
    for ax, (op, label) in zip(axes, CC_OPS):
        vals = [ratio[op][k] for k, _, _ in CC_ROWS]
        ax.axvspan(0, 1.0, color=INK, alpha=0.05, lw=0)
        ax.barh(ys, vals, height=0.62, color=[c for _, _, c in CC_ROWS])
        for y, v in zip(ys, vals):
            ax.text(v + xmax * 0.02, y, f"{v:.2f}\u00d7", va="center",
                    fontsize=9.5, **MONO)
        ax.axvline(1.0, ls="--", lw=1.0, color=INK, alpha=0.45)
        ax.axhline(5.5, color=INK, lw=0.8, alpha=0.45)   # software | hardware
        for sep in (1.5, 3.5):
            ax.axhline(sep, color=INK, lw=0.8, alpha=0.22)
        ax.set_title(label, fontsize=11.5, pad=8)
        ax.set_xlim(0, xmax)
        ax.set_yticks(ys, [n for _, n, _ in CC_ROWS], fontsize=11.5, **MONO)
        strip_axes(ax)
    caption(fig, f"{cc['caption']} \u00b7 runtime relative to double = "
                 f"1.00\u00d7 (dashed) \u00b7 left of the line beats the "
                 f"hardware type")
    fig.tight_layout(rect=(0, 0.045, 1, 1))
    fig.savefig(path, dpi=200)
    plt.close(fig)

# ---------------------------------------------------------------- driver
def ondark(src, dst):
    from PIL import Image
    im = Image.open(src).convert("RGBA")
    bg = Image.new("RGBA", im.size, (20, 18, 15, 255))
    bg.alpha_composite(im)
    bg.convert("RGB").save(dst)

if __name__ == "__main__":
    verify()
    os.makedirs("img", exist_ok=True)
    os.makedirs("spot", exist_ok=True)

    for name in ("x64linux_gcc", "x64linux_intel", "x32linux_gcc",
                 "x64win_msvc"):
        for width in ("32", "64", "128"):
            vendor_chart(CONFIGS[name], width, f"img/perf_{name}_{width}.png")
    for name in ("arm64win_msvc", "m4mac_clang"):
        boostonly_chart(CONFIGS[name], f"img/perf_{name}.png")
    for name in ("x64linux", "x64win", "m4mac"):
        charconv_chart(CHARCONV[name], f"img/perf_charconv_{name}.png")

    for spot in ("perf_x64linux_gcc_64", "perf_arm64win_msvc",
                 "perf_charconv_x64linux"):
        ondark(f"img/{spot}.png", f"spot/{spot}_ondark.png")
    print("generated", len(os.listdir("img")), "images")
