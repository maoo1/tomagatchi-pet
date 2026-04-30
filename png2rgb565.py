from PIL import Image
import numpy as np
import sys
import os

def convert(input_path, var_name=None):
    img = Image.open(input_path).convert("RGBA")
    w, h = img.size

    if var_name is None:
        var_name = os.path.splitext(os.path.basename(input_path))[0]

    var_name = var_name.replace('-', '_')

    pixels = np.array(img)
    out = []

    for row in pixels:
        for px in row:
            r, g, b, a = px
            brightness = (int(r) + int(g) + int(b)) / 3

            # transparent or magenta or near-white = background (white)
            if a == 0 or (r > 200 and g < 50 and b > 200) or brightness > 128:
                out.append(0xFFFF)  # white background
            else:
                out.append(0x0000)  # black outline

    header = f"// Auto-generated from {input_path}\n"
    header += f"// Size: {w}x{h}\n"
    header += f"#pragma once\n"
    header += f"#include <pgmspace.h>\n\n"
    header += f"#define {var_name.upper()}_W {w}\n"
    header += f"#define {var_name.upper()}_H {h}\n\n"
    header += f"const uint16_t {var_name}[] PROGMEM = {{\n"

    for i, val in enumerate(out):
        header += f"0x{val:04X},"
        if (i + 1) % 16 == 0:
            header += "\n"

    header += "\n};\n"

    out_path = os.path.splitext(input_path)[0] + ".h"
    with open(out_path, "w") as f:
        f.write(header)

    print(f"Done: {out_path} ({w}x{h}, {len(out)} pixels)")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python png2rgb565.py sprite.png [var_name]")
    else:
        vname = sys.argv[2] if len(sys.argv) > 2 else None
        convert(sys.argv[1], vname)