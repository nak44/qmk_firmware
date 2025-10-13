// Copyright 2025 Christian Lo (@ChrisChrisLoLo)
// SPDX-License-Identifier: GPL-3.0-or-later
#include <stdio.h>
#include QMK_KEYBOARD_H
#include "qp.h"
#include "qp_st7789.h"

static painter_device_t lcd;

// Color definitions (RGB565 format)
#define COLOR_BLACK 0, 0, 0
#define COLOR_WHITE 255, 255, 255
#define COLOR_RED 255, 0, 0
#define COLOR_GREEN 0, 255, 0
#define COLOR_BLUE 0, 0, 255
#define COLOR_YELLOW 255, 255, 0
#define COLOR_CYAN 0, 255, 255
#define COLOR_MAGENTA 255, 0, 255
#define COLOR_ORANGE 255, 165, 0
#define COLOR_PURPLE 128, 0, 128

// Display dimensions (76x284 rotated 90 degrees = 284x76)
#define LCD_WIDTH 284
#define LCD_HEIGHT 76

void ui_init(void) {
    // Initialize backlight GPIO (simple on/off control)
    // TODO: Implement custom PWM for brightness control if needed
    setPinOutput(LCD_BACKLIGHT_PIN);
    writePinHigh(LCD_BACKLIGHT_PIN);  // Turn on backlight at full brightness

    // Initialize ST7789 display (76x284, rotated 90 degrees)
    lcd = qp_st7789_make_spi_device(76, 284, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN, 8, 0);

    if (lcd == NULL) {
        return;
    }

    // Initialize with 90 degree rotation
    qp_init(lcd, QP_ROTATION_90);

    // Clear display with black background
    qp_rect(lcd, 0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, COLOR_BLACK, true);

    // Draw welcome screen with colorful elements
    // Top banner
    qp_rect(lcd, 0, 0, LCD_WIDTH - 1, 20, COLOR_BLUE, true);
    qp_rect(lcd, 0, 21, LCD_WIDTH - 1, 23, COLOR_CYAN, true);

    // Title text area (placeholder for future text/images)
    qp_rect(lcd, 10, 5, LCD_WIDTH - 10, 18, COLOR_WHITE, false);

    // Side accent bars
    qp_rect(lcd, 0, 25, 5, LCD_HEIGHT - 25, COLOR_GREEN, true);
    qp_rect(lcd, LCD_WIDTH - 6, 25, LCD_WIDTH - 1, LCD_HEIGHT - 25, COLOR_GREEN, true);

    // Center area for content (reserved for future images/graphics)
    // Draw a simple gradient effect using rectangles
    for (int y = 30; y < LCD_HEIGHT - 30; y += 10) {
        uint8_t brightness = 50 + ((y - 30) * 200 / (LCD_HEIGHT - 60));
        qp_rect(lcd, 10, y, LCD_WIDTH - 10, y + 8, brightness, brightness, brightness, true);
    }

    // Bottom status bar area
    qp_rect(lcd, 0, LCD_HEIGHT - 25, LCD_WIDTH - 1, LCD_HEIGHT - 1, COLOR_BLUE, true);

    // Layer indicator boxes (colored)
    for (int i = 0; i < 4; i++) {
        qp_rect(lcd, 10 + (i * 30), LCD_HEIGHT - 20, 35 + (i * 30), LCD_HEIGHT - 5, COLOR_WHITE, false);
    }

    qp_flush(lcd);
}

void ui_task(void) {
    if (lcd == NULL) {
        return;
    }

    // Layer indicator with colors
    static uint8_t last_layer = 0;
    uint8_t current_layer = get_highest_layer(layer_state);

    if (last_layer != current_layer) {
        last_layer = current_layer;

        // Clear indicator area
        qp_rect(lcd, 8, LCD_HEIGHT - 22, LCD_WIDTH - 8, LCD_HEIGHT - 3, COLOR_BLUE, true);

        // Define colors for each layer
        uint8_t layer_colors[][3] = {
            {COLOR_GREEN},    // Layer 0
            {COLOR_YELLOW},   // Layer 1
            {COLOR_ORANGE},   // Layer 2
            {COLOR_MAGENTA}   // Layer 3
        };

        // Draw layer indicator boxes with colors
        for (int i = 0; i < 4; i++) {
            bool is_active = (i == current_layer);
            if (is_active) {
                // Fill active layer with its color
                qp_rect(lcd, 10 + (i * 30), LCD_HEIGHT - 20, 35 + (i * 30), LCD_HEIGHT - 5,
                       layer_colors[i][0], layer_colors[i][1], layer_colors[i][2], true);
            } else {
                // Draw outline for inactive layers
                qp_rect(lcd, 10 + (i * 30), LCD_HEIGHT - 20, 35 + (i * 30), LCD_HEIGHT - 5,
                       COLOR_WHITE, false);
            }
        }

        qp_flush(lcd);
    }

    // TODO: Add more dynamic UI elements here
    // - Lock state indicators (Caps Lock, Num Lock, etc.)
    // - Animation frames
    // - Custom graphics/images
    // - WPM counter
    // - Time display
    // etc.
}
