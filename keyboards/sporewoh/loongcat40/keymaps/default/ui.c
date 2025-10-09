// Copyright 2025 Christian Lo (@ChrisChrisLoLo)
// SPDX-License-Identifier: GPL-3.0-or-later
#include <stdio.h>
#include QMK_KEYBOARD_H
#include "qp.h"
#include "qp_sh1122.h"

static painter_device_t oled;

void ui_init(void) {
    // Initialize SH1122 display (256x64, currently using 1bpp mode)
    oled = qp_sh1122_make_spi_device(256, 64, OLED_CS_PIN, OLED_DC_PIN, OLED_RST_PIN, 8, 0);
    
    if (oled == NULL) {
        return;
    }
    
    // Initialize and clear the display
    qp_init(oled, QP_ROTATION_0);
    qp_rect(oled, 0, 0, 255, 63, 0, 0, 0, true);
    
    // Draw a simple welcome pattern
    // Top border
    qp_rect(oled, 0, 0, 255, 0, 255, 255, 255, true);
    qp_rect(oled, 0, 63, 255, 63, 255, 255, 255, true);
    qp_rect(oled, 0, 0, 0, 63, 255, 255, 255, true);
    qp_rect(oled, 255, 0, 255, 63, 255, 255, 255, true);
    
    // Title box
    qp_rect(oled, 10, 5, 245, 15, 255, 255, 255, false);
    
    // Center pattern - simple checkerboard
    for (int y = 20; y < 58; y += 4) {
        for (int x = 15 + ((y / 4) % 2) * 4; x < 240; x += 8) {
            qp_rect(oled, x, y, x + 3, y + 3, 255, 255, 255, true);
        }
    }
    
    qp_flush(oled);
}

void ui_task(void) {
    if (oled == NULL) {
        return;
    }
    
    // Simple layer indicator - 4 boxes in top right
    static uint8_t last_layer = 0;
    uint8_t current_layer = get_highest_layer(layer_state);
    
    if (last_layer != current_layer) {
        last_layer = current_layer;
        
        // Clear indicator area
        qp_rect(oled, 200, 2, 254, 13, 0, 0, 0, true);
        
        // Draw layer indicator boxes
        for (int i = 0; i < 4; i++) {
            bool filled = (i == current_layer);
            qp_rect(oled, 210 + (i * 11), 3, 218 + (i * 11), 12, 255, 255, 255, filled);
        }
        
        qp_flush(oled);
    }
}
