// Copyright 2025 Christian Lo (@ChrisChrisLoLo)
// SPDX-License-Identifier: GPL-3.0-or-later
#include <stdio.h>
#include QMK_KEYBOARD_H
#include "qp.h"
#include "qp_st7789.h"
#include "qp_comms.h"
#include "qp_st77xx_opcodes.h"

#include "graphics/lcd/animu-image-lcd.qgf.h"
#include "graphics/lcd/shock.qgf.h"

static painter_device_t lcd;

// Display dimensions (320x170 rotated 90 degrees = 170x320)
#define LCD_WIDTH 320
#define LCD_HEIGHT 170

#define LCD_OFFSET_X 35
#define LCD_OFFSET_Y 0

#define NUM_IMAGES 2


static painter_image_handle_t images[NUM_IMAGES];
static uint8_t current_image_index = 0;


void ui_init(void) {
    // Initialize ST7789 display (320x170, rotated 90 degrees)
    lcd = qp_st7789_make_spi_device(LCD_HEIGHT, LCD_WIDTH, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN, 16, 3);

    qp_init(lcd, QP_ROTATION_180);

    qp_comms_start(lcd);

    // Display requires inverting
    qp_comms_command(lcd, ST77XX_CMD_INVERT_ON);

    // #ifdef LCD_INVERT_COLOUR
    // #else
    // qp_comms_command(lcd, ST77XX_CMD_INVERT_OFF);
    // #endif
    qp_comms_stop(lcd);


    qp_power(lcd, true);

    // Apply viewport offset for this display panel
    qp_set_viewport_offsets(lcd, LCD_OFFSET_X, LCD_OFFSET_Y);

    // Clear the display with yellow background
    qp_rect(lcd, 0, 0, LCD_HEIGHT, LCD_WIDTH, 255, 255, 0, true);

    // Load images
    images[0] = qp_load_image_mem(gfx_cool_lcd);
    images[1] = qp_load_image_mem(gfx_shock);

    // Draw the initial image (shock)
    current_image_index = 1;
    if (images[current_image_index] != NULL) {
        qp_drawimage(lcd, 0, 0, images[current_image_index]);
        qp_flush(lcd);
    }

    // Turn on the backlight
    gpio_set_pin_output(LCD_BACKLIGHT_PIN);
    gpio_write_pin_high(LCD_BACKLIGHT_PIN);
}

void ui_task(void) {
    static bool backlight_on = true;

    #if QUANTUM_PAINTER_DISPLAY_TIMEOUT > 0
    bool should_be_on = last_input_activity_elapsed() < QUANTUM_PAINTER_DISPLAY_TIMEOUT;

    if (backlight_on != should_be_on) {
        if (should_be_on) {
            gpio_write_pin_high(LCD_BACKLIGHT_PIN);
        } else {
            gpio_write_pin_low(LCD_BACKLIGHT_PIN);
        }
        backlight_on = should_be_on;
    }
    #endif
}

void ui_cycle_image(void) {
    // Cycle to the next image
    current_image_index = (current_image_index + 1) % NUM_IMAGES;

    // Clear the display
    qp_rect(lcd, 0, 0, 255, 63, 0, 0, 0, true);
    qp_flush(lcd);

    // Draw the selected image using array indexing
    if (images[current_image_index] != NULL) {
        qp_drawimage(lcd, 1, 0, images[current_image_index]);
    }

    qp_flush(lcd);
}

void ui_cycle_image_reverse(void) {
    // Cycle to the previous image (add NUM_IMAGES to handle negative modulo correctly)
    current_image_index = (current_image_index - 1 + NUM_IMAGES) % NUM_IMAGES;

    // Clear the display
    qp_rect(lcd, 0, 0, 255, 63, 0, 0, 0, true);
    qp_flush(lcd);

    // Draw the selected image using array indexing
    if (images[current_image_index] != NULL) {
        qp_drawimage(lcd, 1, 0, images[current_image_index]);
    }

    qp_flush(lcd);
}

void ui_suspend(void) {
    gpio_set_pin_output(LCD_BACKLIGHT_PIN);
    gpio_write_pin_low(LCD_BACKLIGHT_PIN);
}

void ui_wakeup(void) {
    gpio_set_pin_output(LCD_BACKLIGHT_PIN);
    gpio_write_pin_high(LCD_BACKLIGHT_PIN);
}
