// Copyright 2025 Christian Lo (@sporewoh)
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include_next <mcuconf.h>

// Used for RGB
#undef RP_SPI_USE_SPI0
#define RP_SPI_USE_SPI0 TRUE
