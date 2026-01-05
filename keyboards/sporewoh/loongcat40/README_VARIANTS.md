# Loongcat40 Firmware Variants

This keyboard now has two display variants:

## Variants

### 1. OLED Variant (`rev1/oled`)
- **Display**: SH1122 OLED (256x64, monochrome)
- **Driver**: `sh1122_spi`
- **UI Location**: `keymaps/default/ui_oled/ui.c`
- **Build Command**: `make sporewoh/loongcat40/rev1/oled:default`

### 2. LCD Variant (`rev1/lcd`)
- **Display**: ST7789 LCD (176x284 pixels, RGB color)
- **Driver**: `st7789_spi`
- **Rotation**: 90 degrees (effective resolution: 284x176)
- **Backlight**: GPIO13 (simple on/off control)
- **UI Location**: `keymaps/default/ui_lcd/ui.c`
- **Build Command**: `make sporewoh/loongcat40/rev1/lcd:default`

## Pin Configuration

### Common SPI Pins (Both Variants)
- **SCK**: GP18
- **MOSI**: GP19

### OLED-Specific Pins
- **CS**: GP17 (OLED_CS_PIN)
- **DC**: GP15 (OLED_DC_PIN)
- **RST**: GP14 (OLED_RST_PIN)

### LCD-Specific Pins
- **CS**: GP17 (LCD_CS_PIN)
- **DC**: GP15 (LCD_DC_PIN)
- **RST**: GP14 (LCD_RST_PIN)
- **Backlight**: GP13 (LCD_BACKLIGHT_PIN) - **NEW**

## LCD UI Features

The LCD variant includes a colorful UI skeleton designed for customization:

### Current Features
- Color layer indicators (Green, Yellow, Orange, Magenta)
- Top blue banner with cyan accent
- Side green accent bars
- Gradient effect in center area
- Bottom status bar

### Color Definitions
Pre-defined colors available in `ui_lcd/ui.c`:
- `COLOR_BLACK`, `COLOR_WHITE`
- `COLOR_RED`, `COLOR_GREEN`, `COLOR_BLUE`
- `COLOR_YELLOW`, `COLOR_CYAN`, `COLOR_MAGENTA`
- `COLOR_ORANGE`, `COLOR_PURPLE`

### Adding Custom Graphics

The LCD UI is structured to make it easy to add custom images:

1. **Convert images to QGF format** using QMK's image tools
2. **Place image files** in the `graphics/` folder
3. **Include them** in `ui_lcd/ui.c`
4. **Draw them** using `qp_drawimage()` function

Example placeholder locations in the UI:
- **Top banner** (y: 0-23): Logo or title
- **Center area** (y: 30-150): Main graphics/animations
- **Bottom bar** (y: 151-175): Status information

### TODO: Backlight PWM Control

Currently, the backlight uses simple GPIO on/off control. To implement PWM brightness control:

1. Research RP2040 PWM configuration for GP13
2. Implement custom PWM driver in keyboard code
3. Add brightness adjustment keycodes if desired

## File Structure

```
keyboards/sporewoh/loongcat40/
├── rev1/
│   ├── oled/              # OLED variant
│   │   ├── config.h       # OLED pin configuration
│   │   ├── halconf.h
│   │   ├── mcuconf.h
│   │   ├── keyboard.json
│   │   └── rules.mk       # sh1122_spi driver
│   │
│   └── lcd/               # LCD variant
│       ├── config.h       # LCD pin configuration + backlight
│       ├── halconf.h
│       ├── mcuconf.h
│       ├── keyboard.json
│       └── rules.mk       # st7789_spi driver
│
└── keymaps/
    └── default/
        ├── ui_oled/       # OLED UI implementation
        │   └── ui.c
        │
        ├── ui_lcd/        # LCD UI implementation
        │   └── ui.c
        │
        └── rules.mk       # Conditionally compiles correct UI
```

## Building

### Build OLED variant:
```bash
make sporewoh/loongcat40/rev1/oled:default
```

### Build LCD variant:
```bash
make sporewoh/loongcat40/rev1/lcd:default
```

### Flash to keyboard:
After building, the `.uf2` file will be in the root of the qmk_firmware folder. 
Copy it to your RP2040 while in bootloader mode.

## Development Tips

1. **Test both variants** after making changes to ensure compatibility
2. **Keep common code** in the base `loongcat40.c` file
3. **Variant-specific features** should go in the respective `ui.c` files
4. **Graphics files** can be shared in the `graphics/` folder

## Next Steps for LCD Customization

1. Create custom graphics using QMK's image conversion tools
2. Implement PWM backlight brightness control
3. Add more UI elements (WPM counter, time display, animations)
4. Optimize colors and layout for your preferences
