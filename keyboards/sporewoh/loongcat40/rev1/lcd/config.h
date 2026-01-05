// Copyright 2025 Christian Lo (@sporewoh)
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// Matrix IO Delay - Adds delay between matrix scans to prevent ghosting on RP2040
// Default is 30us, increased to 75us for stability
// Adjust if experiencing ghosting: increase to 100us, or decrease to 50us if stable
// #define MATRIX_IO_DELAY 100

// SPI Configuration
#define SPI_DRIVER SPID0
#define SPI_SCK_PIN GP18
#define SPI_MOSI_PIN GP19

// LCD Display Configuration (ST7789)
#define LCD_CS_PIN GP17
#define LCD_DC_PIN GP15
#define LCD_RST_PIN GP14
#define LCD_BACKLIGHT_PIN GP13
