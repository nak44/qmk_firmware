// Copyright 2025 Christian Lo (@ChrisChrisLoLo)
// SPDX-License-Identifier: GPL-3.0-or-later
#include <stdio.h>
#include QMK_KEYBOARD_H
#include "qp.h"
#include "qp_sh1122.h"

#include "graphics/animu-image.qgf.c"

static painter_device_t oled;
static painter_image_handle_t animu_image;

void ui_init(void) {
    // Initialize SH1122 display (256x64, currently using 1bpp mode)
    oled = qp_sh1122_make_spi_device(256, 64, OLED_CS_PIN, OLED_DC_PIN, OLED_RST_PIN, 8, 0);

    if (oled == NULL) {
        return;
    }

    // Load the animu image
    animu_image = qp_load_image_mem(gfx_animu_image);

    // Initialize and clear the display (no rotation - image is pre-rotated)
    qp_init(oled, QP_ROTATION_0);
    qp_rect(oled, 0, 0, 255, 63, 0, 0, 0, true);

    // Draw the animu image (centered on display)
    // Image is 254x64 (pre-rotated), display is 256x64
    // Center horizontally: (256 - 254) / 2 = 1
    if (animu_image != NULL) {
        qp_drawimage(oled, 1, 0, animu_image);
    }

    qp_flush(oled);
}

void ui_task(void) {
    // Reserved for future hotkey-based image switching
}
