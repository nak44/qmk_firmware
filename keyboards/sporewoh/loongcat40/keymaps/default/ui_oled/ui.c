// Copyright 2025 Christian Lo (@ChrisChrisLoLo)
// SPDX-License-Identifier: GPL-3.0-or-later
#include <stdio.h>
#include QMK_KEYBOARD_H
#include "qp.h"
#include "qp_sh1122.h"

#include "graphics/oled/animu-image.qgf.c"
#include "graphics/oled/kicub_rotated.qgf.c"
#include "graphics/oled/nextpcb_rotated.qgf.c"

#define NUM_IMAGES 3

static painter_device_t oled;
static painter_image_handle_t animu_image;
static painter_image_handle_t kicub_image;
static painter_image_handle_t nextpcb_image;
static painter_image_handle_t images[NUM_IMAGES];
static uint8_t current_image_index = 0;

void ui_init(void) {
    // Initialize SH1122 display (256x64, currently using 1bpp mode)
    oled = qp_sh1122_make_spi_device(256, 64, OLED_CS_PIN, OLED_DC_PIN, OLED_RST_PIN, 8, 0);

    if (oled == NULL) {
        return;
    }

    // Load all three images
    animu_image = qp_load_image_mem(gfx_animu_image);
    kicub_image = qp_load_image_mem(gfx_kicub_rotated);
    nextpcb_image = qp_load_image_mem(gfx_nextpcb_rotated);

    // Store in array for easy indexing
    images[0] = animu_image;
    images[1] = kicub_image;
    images[2] = nextpcb_image;

    // Initialize and clear the display (no rotation - image is pre-rotated)
    qp_init(oled, QP_ROTATION_0);
    qp_rect(oled, 0, 0, 255, 63, 0, 0, 0, true);
    qp_flush(oled);

    // Draw the animu image (centered on display)
    // Image is 254x64 (pre-rotated), display is 256x64
    // Center horizontally: (256 - 254) / 2 = 1
    if (animu_image != NULL) {
        qp_drawimage(oled, 1, 0, animu_image);
    }

    qp_flush(oled);
}

void ui_task(void) {
    // Reserved for future animations or updates
}

void ui_cycle_image(void) {
    // Cycle to the next image
    current_image_index = (current_image_index + 1) % NUM_IMAGES;

    // Clear the display
    qp_rect(oled, 0, 0, 255, 63, 0, 0, 0, true);
    qp_flush(oled);

    // Draw the selected image using array indexing
    if (images[current_image_index] != NULL) {
        qp_drawimage(oled, 1, 0, images[current_image_index]);
    }

    qp_flush(oled);
}

void ui_cycle_image_reverse(void) {
    // Cycle to the previous image (add NUM_IMAGES to handle negative modulo correctly)
    current_image_index = (current_image_index - 1 + NUM_IMAGES) % NUM_IMAGES;

    // Clear the display
    qp_rect(oled, 0, 0, 255, 63, 0, 0, 0, true);
    qp_flush(oled);

    // Draw the selected image using array indexing
    if (images[current_image_index] != NULL) {
        qp_drawimage(oled, 1, 0, images[current_image_index]);
    }

    qp_flush(oled);
}
