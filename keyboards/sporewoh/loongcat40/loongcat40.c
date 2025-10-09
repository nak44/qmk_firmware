// Copyright 2025 Christian Lo (@ChrisChrisLoLo)
// SPDX-License-Identifier: GPL-3.0-or-later
#include "quantum.h"

void keyboard_post_init_kb(void) {
    // Offload to the user func
    keyboard_post_init_user();
}
