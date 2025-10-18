// Copyright 2025 Christian Lo (@ChrisChrisLoLo)
// SPDX-License-Identifier: GPL-3.0-or-later
#include <stdio.h>
#include QMK_KEYBOARD_H
#include "qp.h"
#include "qp_st7789.h"
#include "qp_comms.h"
#include "qp_st77xx_opcodes.h"

#include "graphics/lcd/animu-image-lcd.qgf.h"

static painter_device_t lcd;
static painter_image_handle_t animu_image;

// Display dimensions (76x284 rotated 90 degrees = 284x76)
#define LCD_WIDTH 284
#define LCD_HEIGHT 76


void ui_init(void) {
    // Initialize ST7789 display (76x284, rotated 90 degrees)
    lcd = qp_st7789_make_spi_device(LCD_HEIGHT, LCD_WIDTH, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN, 16, 3);

    qp_init(lcd, QP_ROTATION_180);

    qp_comms_start(lcd);
    #ifdef LCD_INVERT_COLOUR
    qp_comms_command(lcd, ST77XX_CMD_INVERT_ON);
    #else
    qp_comms_command(lcd, ST77XX_CMD_INVERT_OFF);
    #endif
    qp_comms_stop(lcd);


    qp_power(lcd, true);

    // Apply viewport offset for this display panel
    qp_set_viewport_offsets(lcd, 82, 18);

    // Clear the display with yellow background
    qp_rect(lcd, 0, 0, LCD_HEIGHT, LCD_WIDTH, 255, 255, 0, true);

    // Load and draw the image
    animu_image = qp_load_image_mem(gfx_cool_lcd);
    if (animu_image != NULL) {
        qp_drawimage(lcd, 0, 0, animu_image);
        qp_flush(lcd);
        qp_close_image(animu_image);
    }
}

void ui_task(void) {
    // Reserved for future functionality
}
