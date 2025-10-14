// Copyright 2025 Christian Lo (@ChrisChrisLoLo)
// SPDX-License-Identifier: GPL-3.0-or-later
#include QMK_KEYBOARD_H

extern void ui_init(void);
extern void ui_task(void);

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        KC_Q,      KC_W,    KC_E,  KC_R,           KC_T,            KC_Y,  KC_U,         KC_I,    KC_O,    KC_P,
        KC_A,      KC_S,    KC_D,  KC_F,           KC_G,            KC_H,  KC_J,         KC_K,    KC_L,    KC_SEMICOLON,
        KC_Z,      KC_X,    KC_C,  KC_V,           KC_B,            KC_N,  KC_M,         KC_COMM, KC_DOT,  KC_SLSH,
        KC_ESCAPE, KC_LGUI, KC_NO, LCTL_T(KC_TAB), LT(2, KC_SPACE), MO(1), KC_BACKSPACE, KC_LEFT_SHIFT, KC_LALT, KC_ENTER),
    [1] = LAYOUT(
        KC_1,      KC_2,     KC_3,     KC_4,        KC_5,        KC_6,    KC_7,    KC_8,      KC_9,     KC_0,
        KC_GRAVE,  KC_MINUS, KC_EQUAL, KC_LEFT_BRACKET, KC_RIGHT_BRACKET, KC_LEFT, KC_DOWN, KC_UP,     KC_RIGHT, KC_QUOTE,
        KC_BACKSLASH, KC_TRNS,  KC_TRNS,  KC_TRNS,     KC_TRNS,     KC_TRNS, KC_TRNS, KC_TRNS,   KC_TRNS,  KC_QUOTE,
        KC_NO,     KC_NO,    KC_NO,    KC_NO,       MO(3),       KC_NO,   KC_NO,   KC_BACKSLASH, KC_NO,    KC_NO),
    [2] = LAYOUT(
        LSFT(KC_1), LSFT(KC_2), LSFT(KC_3), LSFT(KC_4), LSFT(KC_5), LSFT(KC_6), LSFT(KC_7), LSFT(KC_8), LSFT(KC_9), LSFT(KC_0),
        LSFT(KC_GRAVE), LSFT(KC_MINUS), LSFT(KC_EQUAL), LSFT(KC_LEFT_BRACKET), LSFT(KC_RIGHT_BRACKET), KC_LEFT, KC_DOWN, KC_UP, KC_RIGHT, LSFT(KC_QUOTE),
        LSFT(KC_BACKSLASH), KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, LSFT(KC_QUOTE),
        KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, MO(3), KC_NO, LSFT(KC_BACKSLASH), KC_NO, KC_NO),
    [3] = LAYOUT(
        QK_REBOOT, QK_BOOT, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_DELETE,
        KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_VOLD, KC_VOLU, KC_NO, KC_NO,
        KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12, KC_NO, KC_NO, KC_NO, KC_NO,
        KC_NO, QK_MAGIC_SWAP_LCTL_LGUI, KC_NO, QK_MAGIC_UNSWAP_LCTL_LGUI, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO),
};

void keyboard_post_init_user(void) {
    // Init the display
    ui_init();
}

void housekeeping_task_user(void) {
    // Draw the display
    ui_task();
}
