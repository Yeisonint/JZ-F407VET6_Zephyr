SSD1316 OLED Driver & Font Generation

# Font Generation
## Zephyr CFB Font (Native)
Use the official Zephyr SDK script to generate a font header for the Character Frame Buffer (CFB) subsystem.

```Bash
../../zephyr/scripts/build/gen_cfb_font_header.py \
  -i ./fonts/droid-sans-mono.ttf \
  -x 5 -y 8 -s 5 --center-x \
  -o ./src/cfb_font_58.c
```

## LVGL Font (Graphical)
To use custom fonts with LVGL, install the font converter utility via npm:

```Bash
npm i lv_font_conv -g
```

## Conversion to 8px (Monochrome)
Run the following command to generate a 1bpp (1 bit per pixel) font. This is optimized for SSD13xx OLEDs to ensure sharp edges without anti-aliasing artifacts.

```Bash
npx lv_font_conv \
  --font fonts/droid-sans-mono.ttf \
  --size 8 \
  --bpp 1 \
  --format lvgl \
  -r 0x20-0x7F \
  -o src/droid_sans_mono_8.c \
  --lv-include "lvgl.h"
```