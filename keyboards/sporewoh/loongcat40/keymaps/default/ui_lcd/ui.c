// Copyright 2025 Christian Lo (@ChrisChrisLoLo)
// SPDX-License-Identifier: GPL-3.0-or-later
#include <stdio.h>
#include QMK_KEYBOARD_H
#include "qp.h"
#include "qp_st7789.h"
#include "qp_comms.h"
#include "qp_st77xx_opcodes.h"

#include "graphics/lcd/animu-image-lcd.qgf.c"

static painter_device_t lcd;
static painter_image_handle_t animu_image;

// Display dimensions (76x284 rotated 90 degrees = 284x76)
#define LCD_WIDTH 284
#define LCD_HEIGHT 76

void ui_init(void) {
    // Initialize backlight GPIO (simple on/off control)
    // setPinOutput(LCD_BACKLIGHT_PIN);
    // writePinLow(LCD_BACKLIGHT_PIN);  // Turn on backlight at full brightness

    // Initialize ST7789 display (76x284, rotated 90 degrees)
    lcd = qp_st7789_make_spi_device(LCD_WIDTH, LCD_HEIGHT, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN, 16, 3);

    qp_init(lcd, QP_ROTATION_90);

    qp_power(lcd, true);

    // Apply Offset
    qp_set_viewport_offsets(lcd, 18, 82);

    // Turn on the LCD and clear the display
    qp_power(lcd, true);
    qp_rect(lcd, 0, 0, LCD_WIDTH, LCD_HEIGHT, 255, 255, 255, true);

    // TEST: Draw red rectangle in top-left corner to verify LCD works
    //qp_rect(lcd, 10, 10, 100, 40, 255, 0, 0, true);

    // TEST: Draw green rectangle in center
    // qp_rect(lcd, LCD_WIDTH / 2 - 30, LCD_HEIGHT / 2 - 15,
    //         LCD_WIDTH / 2 + 30, LCD_HEIGHT / 2 + 15, 0, 255, 0, true);

    // TEST: Draw blue rectangle in bottom-right
    // qp_rect(lcd, LCD_WIDTH - 110, LCD_HEIGHT - 45,
    //         LCD_WIDTH - 10, LCD_HEIGHT - 10, 0, 0, 255, true);

    // Load the animu image (commented out for testing)
    animu_image = qp_load_image_mem(gfx_cool_lcd);
    if (animu_image != NULL) {
        qp_drawimage(lcd, 0, 0, animu_image);
    }

     //qp_rect(lcd, 10, 10, 100, 40, 255, 0, 0, true);

    // qp_rect(lcd, LCD_WIDTH / 2 - 30, LCD_HEIGHT / 2 - 15,
    //         LCD_WIDTH / 2 + 30, LCD_HEIGHT / 2 + 15, 0, 255, 0, true);

     qp_rect(lcd, LCD_WIDTH - 110, LCD_HEIGHT - 45,
              LCD_WIDTH - 10, LCD_HEIGHT - 10, 120, 0, 120, true);
    qp_rect(lcd, LCD_WIDTH - 110, LCD_HEIGHT - 45,
              LCD_WIDTH - 10, LCD_HEIGHT - 10, 120, 120, 120, true);

    qp_flush(lcd);
}

void ui_task(void) {
    // Reserved for future functionality
}
