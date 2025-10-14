#include "qp_internal.h"
#include "qp_comms.h"
#include "qp_oled_panel.h"
#include "qp_sh1122.h"
#include "qp_sh1122_opcodes.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Driver storage
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct sh1122_device_t {
    oled_panel_painter_device_t oled;

    // SH1122 framebuffer - using 1bpp for now (256 * 64 / 8 = 2048 bytes)
    // TODO: Implement 4bpp surface for full grayscale support
    uint8_t framebuffer[2048];
} sh1122_device_t;

static sh1122_device_t sh1122_drivers[SH1122_NUM_DEVICES] = {0};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Quantum Painter API implementations

// Initialisation
__attribute__((weak)) bool qp_sh1122_init(painter_device_t device, painter_rotation_t rotation) {
    sh1122_device_t *driver = (sh1122_device_t *)device;

    // Change the surface geometry based on the panel rotation
    if (rotation == QP_ROTATION_90 || rotation == QP_ROTATION_270) {
        driver->oled.surface.base.panel_width  = driver->oled.base.panel_height;
        driver->oled.surface.base.panel_height = driver->oled.base.panel_width;
    } else {
        driver->oled.surface.base.panel_width  = driver->oled.base.panel_width;
        driver->oled.surface.base.panel_height = driver->oled.base.panel_height;
    }

    // Init the internal surface
    if (!qp_init(&driver->oled.surface.base, QP_ROTATION_0)) {
        qp_dprintf("Failed to init internal surface in qp_sh1122_init\n");
        return false;
    }

    // clang-format off
    const uint8_t sh1122_init_sequence[] = {
        // Command,                         Delay,  N, Data[N]
        SH1122_DISPLAY_OFF,                     0,  0,
        SH1122_SET_START_LINE,                  0,  0,
        SH1122_PAGE_ADDRESSING_MODE,            0,  0,          // Critical: Set page addressing mode
        SH1122_SET_SEGMENT_REMAP_NORMAL,        0,  0,
        SH1122_COM_SCAN_DIR_INC,                0,  0,
        SH1122_SET_CONTRAST,                    0,  1, 0x40,    // Brightness: 0x00 (dimmest) to 0xFF (brightest). 0x40 = ~25%
        SH1122_SET_MUX_RATIO,                   0,  1, 0x3F,    // 1/64 duty
        SH1122_DC_DC_CONTROL,                   0,  1, 0x81,    // Use built-in DC-DC
        SH1122_SET_OSC_DIVFREQ,                 0,  1, 0x50,
        SH1122_DISPLAY_OFFSET,                  0,  1, 0x00,
        SH1122_SET_PRECHARGE_PERIOD,            0,  1, 0x22,
        SH1122_VCOM_DESELECT_LEVEL,             0,  1, 0x35,
        SH1122_PRECHARGE_VOLTAGE,               0,  1, 0x35,
        SH1122_DISCHARGE_LEVEL,                 0,  0,
        SH1122_ALL_ON_RESUME,                   0,  0,
        SH1122_NON_INVERTING_DISPLAY,           0,  0,
        SH1122_DISPLAY_ON,                      0,  0,
    };
    // clang-format on

    qp_comms_bulk_command_sequence(device, sh1122_init_sequence, sizeof(sh1122_init_sequence));
    return true;
}

// Custom flush for SH1122 with 1bpp to 4bpp conversion
static void qp_sh1122_flush_rot0(painter_device_t device, surface_dirty_data_t *dirty, const uint8_t *framebuffer) {
    painter_driver_t *                  driver = (painter_driver_t *)device;
    oled_panel_painter_driver_vtable_t *vtable = (oled_panel_painter_driver_vtable_t *)driver->driver_vtable;

    int min_row    = dirty->t;
    int max_row    = dirty->b;
    int min_column = dirty->l;
    int max_column = dirty->r;

    // SH1122 RAM is organized by rows (horizontal lines), not columns
    // Each row must be addressed individually and sent as horizontal pixel data
    for (int row = min_row; row <= max_row; ++row) {
        int     cols_required = max_column - min_column + 1;
        uint8_t row_data[cols_required];
        memset(row_data, 0, cols_required);

        // Build row data from 1bpp framebuffer (horizontal line of pixels)
        for (int x = min_column; x <= max_column; ++x) {
            uint32_t pixel_num   = row * driver->panel_width + x;
            uint32_t byte_offset = pixel_num / 8;
            uint8_t  bit_offset  = pixel_num % 8;

            // Extract pixel value (0 or 1) and store as 4-bit value (0x0 or 0xF)
            uint8_t pixel = (framebuffer[byte_offset] >> bit_offset) & 1;
            row_data[x - min_column] = pixel ? 0x0F : 0x00;
        }

        // Set row address for this specific row
        qp_comms_command(device, vtable->opcodes.set_page);
        qp_comms_command(device, row);

        // Set column start address (each column address is 2 pixels wide in 4bpp mode)
        uint8_t col_addr = min_column / 2;
        qp_comms_command(device, vtable->opcodes.set_column_lsb | (col_addr & 0x0F));
        qp_comms_command(device, vtable->opcodes.set_column_msb | (col_addr >> 4));

        // Convert and send row data in 4bpp format
        // Pack 2 pixels (4 bits each) into each byte sent to the display
        for (int i = 0; i < cols_required; i += 2) {
            uint8_t output;
            if (i + 1 < cols_required) {
                // Two pixels available - pack both (first pixel in high nibble, second in low nibble)
                output = (row_data[i] << 4) | row_data[i + 1];
            } else {
                // Only one pixel left - pack it in high nibble
                output = (row_data[i] << 4);
            }
            qp_comms_send(device, &output, 1);
        }
    }
}

// Rotation 90 degrees - transpose and flip
static void qp_sh1122_flush_rot90(painter_device_t device, surface_dirty_data_t *dirty, const uint8_t *framebuffer) {
    sh1122_device_t *                   sh_driver = (sh1122_device_t *)device;
    painter_driver_t *                  driver = (painter_driver_t *)device;
    oled_panel_painter_driver_vtable_t *vtable = (oled_panel_painter_driver_vtable_t *)driver->driver_vtable;

    // For 90° rotation: physical(row, col) = logical(col, height - 1 - row)
    // The logical framebuffer is in the rotated coordinate space
    int min_row    = dirty->t;
    int max_row    = dirty->b;
    int min_column = dirty->l;
    int max_column = dirty->r;

    for (int row = min_row; row <= max_row; ++row) {
        int     cols_required = max_column - min_column + 1;
        uint8_t row_data[cols_required];
        memset(row_data, 0, cols_required);

        // For 90° rotation, read from logical column (which becomes physical row)
        for (int x = min_column; x <= max_column; ++x) {
            // Map: physical(row, x) = logical(x, height - 1 - row)
            int      logical_x   = x;
            int      logical_y   = sh_driver->oled.surface.base.panel_height - 1 - row;
            uint32_t pixel_num   = logical_y * sh_driver->oled.surface.base.panel_width + logical_x;
            uint32_t byte_offset = pixel_num / 8;
            uint8_t  bit_offset  = pixel_num % 8;

            uint8_t pixel = (framebuffer[byte_offset] >> bit_offset) & 1;
            row_data[x - min_column] = pixel ? 0x0F : 0x00;
        }

        qp_comms_command(device, vtable->opcodes.set_page);
        qp_comms_command(device, row);

        uint8_t col_addr = min_column / 2;
        qp_comms_command(device, vtable->opcodes.set_column_lsb | (col_addr & 0x0F));
        qp_comms_command(device, vtable->opcodes.set_column_msb | (col_addr >> 4));

        for (int i = 0; i < cols_required; i += 2) {
            uint8_t output;
            if (i + 1 < cols_required) {
                output = (row_data[i] << 4) | row_data[i + 1];
            } else {
                output = (row_data[i] << 4);
            }
            qp_comms_send(device, &output, 1);
        }
    }
}

// Rotation 180 degrees - flip both axes
static void qp_sh1122_flush_rot180(painter_device_t device, surface_dirty_data_t *dirty, const uint8_t *framebuffer) {
    sh1122_device_t *                   sh_driver = (sh1122_device_t *)device;
    painter_driver_t *                  driver = (painter_driver_t *)device;
    oled_panel_painter_driver_vtable_t *vtable = (oled_panel_painter_driver_vtable_t *)driver->driver_vtable;

    // For 180° rotation: physical(row, col) = logical(width - 1 - col, height - 1 - row)
    int min_row    = dirty->t;
    int max_row    = dirty->b;
    int min_column = dirty->l;
    int max_column = dirty->r;

    for (int row = min_row; row <= max_row; ++row) {
        int     cols_required = max_column - min_column + 1;
        uint8_t row_data[cols_required];
        memset(row_data, 0, cols_required);

        for (int x = min_column; x <= max_column; ++x) {
            // Map: physical(row, x) = logical(width - 1 - x, height - 1 - row)
            int      logical_x   = sh_driver->oled.surface.base.panel_width - 1 - x;
            int      logical_y   = sh_driver->oled.surface.base.panel_height - 1 - row;
            uint32_t pixel_num   = logical_y * sh_driver->oled.surface.base.panel_width + logical_x;
            uint32_t byte_offset = pixel_num / 8;
            uint8_t  bit_offset  = pixel_num % 8;

            uint8_t pixel = (framebuffer[byte_offset] >> bit_offset) & 1;
            row_data[x - min_column] = pixel ? 0x0F : 0x00;
        }

        qp_comms_command(device, vtable->opcodes.set_page);
        qp_comms_command(device, row);

        uint8_t col_addr = min_column / 2;
        qp_comms_command(device, vtable->opcodes.set_column_lsb | (col_addr & 0x0F));
        qp_comms_command(device, vtable->opcodes.set_column_msb | (col_addr >> 4));

        for (int i = 0; i < cols_required; i += 2) {
            uint8_t output;
            if (i + 1 < cols_required) {
                output = (row_data[i] << 4) | row_data[i + 1];
            } else {
                output = (row_data[i] << 4);
            }
            qp_comms_send(device, &output, 1);
        }
    }
}

// Rotation 270 degrees - transpose and flip opposite to 90
static void qp_sh1122_flush_rot270(painter_device_t device, surface_dirty_data_t *dirty, const uint8_t *framebuffer) {
    sh1122_device_t *                   sh_driver = (sh1122_device_t *)device;
    painter_driver_t *                  driver = (painter_driver_t *)device;
    oled_panel_painter_driver_vtable_t *vtable = (oled_panel_painter_driver_vtable_t *)driver->driver_vtable;

    // For 270° rotation: physical(row, col) = logical(height - 1 - col, row)
    int min_row    = dirty->t;
    int max_row    = dirty->b;
    int min_column = dirty->l;
    int max_column = dirty->r;

    for (int row = min_row; row <= max_row; ++row) {
        int     cols_required = max_column - min_column + 1;
        uint8_t row_data[cols_required];
        memset(row_data, 0, cols_required);

        for (int x = min_column; x <= max_column; ++x) {
            // Map: physical(row, x) = logical(height - 1 - x, row)
            int      logical_x   = sh_driver->oled.surface.base.panel_height - 1 - x;
            int      logical_y   = row;
            uint32_t pixel_num   = logical_y * sh_driver->oled.surface.base.panel_width + logical_x;
            uint32_t byte_offset = pixel_num / 8;
            uint8_t  bit_offset  = pixel_num % 8;

            uint8_t pixel = (framebuffer[byte_offset] >> bit_offset) & 1;
            row_data[x - min_column] = pixel ? 0x0F : 0x00;
        }

        qp_comms_command(device, vtable->opcodes.set_page);
        qp_comms_command(device, row);

        uint8_t col_addr = min_column / 2;
        qp_comms_command(device, vtable->opcodes.set_column_lsb | (col_addr & 0x0F));
        qp_comms_command(device, vtable->opcodes.set_column_msb | (col_addr >> 4));

        for (int i = 0; i < cols_required; i += 2) {
            uint8_t output;
            if (i + 1 < cols_required) {
                output = (row_data[i] << 4) | row_data[i + 1];
            } else {
                output = (row_data[i] << 4);
            }
            qp_comms_send(device, &output, 1);
        }
    }
}

// Screen flush
bool qp_sh1122_flush(painter_device_t device) {
    sh1122_device_t *driver = (sh1122_device_t *)device;

    if (!driver->oled.surface.dirty.is_dirty) {
        return true;
    }

    switch (driver->oled.base.rotation) {
        default:
        case QP_ROTATION_0:
            qp_sh1122_flush_rot0(device, &driver->oled.surface.dirty, driver->framebuffer);
            break;
        case QP_ROTATION_90:
            qp_sh1122_flush_rot90(device, &driver->oled.surface.dirty, driver->framebuffer);
            break;
        case QP_ROTATION_180:
            qp_sh1122_flush_rot180(device, &driver->oled.surface.dirty, driver->framebuffer);
            break;
        case QP_ROTATION_270:
            qp_sh1122_flush_rot270(device, &driver->oled.surface.dirty, driver->framebuffer);
            break;
    }

    // Clear the dirty area
    qp_flush(&driver->oled.surface);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Driver vtable
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const oled_panel_painter_driver_vtable_t sh1122_driver_vtable = {
    .base =
        {
            .init            = qp_sh1122_init,
            .power           = qp_oled_panel_power,
            .clear           = qp_oled_panel_clear,
            .flush           = qp_sh1122_flush,
            .pixdata         = qp_oled_panel_passthru_pixdata,
            .viewport        = qp_oled_panel_passthru_viewport,
            .palette_convert = qp_oled_panel_passthru_palette_convert,
            .append_pixels   = qp_oled_panel_passthru_append_pixels,
            .append_pixdata  = qp_oled_panel_passthru_append_pixdata,
        },
    .opcodes =
        {
            .display_on     = SH1122_DISPLAY_ON,
            .display_off    = SH1122_DISPLAY_OFF,
            .set_page       = SH1122_PAGE_ADDR,
            .set_column_lsb = SH1122_SETCOLUMN_LSB,
            .set_column_msb = SH1122_SETCOLUMN_MSB,
        },
};

#ifdef QUANTUM_PAINTER_SH1122_SPI_ENABLE
// Factory function for creating a handle to the SH1122 device
painter_device_t qp_sh1122_make_spi_device(uint16_t panel_width, uint16_t panel_height, pin_t chip_select_pin, pin_t dc_pin, pin_t reset_pin, uint16_t spi_divisor, int spi_mode) {
    for (uint32_t i = 0; i < SH1122_NUM_DEVICES; ++i) {
        sh1122_device_t *driver = &sh1122_drivers[i];
        if (!driver->oled.base.driver_vtable) {
            // Using 1bpp surface for now - TODO: implement 4bpp for full grayscale
            painter_device_t surface = qp_make_mono1bpp_surface_advanced(&driver->oled.surface, 1, panel_width, panel_height, driver->framebuffer);
            if (!surface) {
                return NULL;
            }

            // Setup the OLED device
            driver->oled.base.driver_vtable         = (const painter_driver_vtable_t *)&sh1122_driver_vtable;
            driver->oled.base.comms_vtable          = (const painter_comms_vtable_t *)&spi_comms_with_dc_vtable;
            driver->oled.base.native_bits_per_pixel = 1; // Using 1bpp for now
            driver->oled.base.panel_width           = panel_width;
            driver->oled.base.panel_height          = panel_height;
            driver->oled.base.rotation              = QP_ROTATION_0;
            driver->oled.base.offset_x              = 0;
            driver->oled.base.offset_y              = 0;

            // SPI and other pin configuration
            driver->oled.base.comms_config                                   = &driver->oled.spi_dc_reset_config;
            driver->oled.spi_dc_reset_config.spi_config.chip_select_pin      = chip_select_pin;
            driver->oled.spi_dc_reset_config.spi_config.divisor              = spi_divisor;
            driver->oled.spi_dc_reset_config.spi_config.lsb_first            = false;
            driver->oled.spi_dc_reset_config.spi_config.mode                 = spi_mode;
            driver->oled.spi_dc_reset_config.dc_pin                          = dc_pin;
            driver->oled.spi_dc_reset_config.reset_pin                       = reset_pin;
            driver->oled.spi_dc_reset_config.command_params_uses_command_pin = true;

            if (!qp_internal_register_device((painter_device_t)driver)) {
                memset(driver, 0, sizeof(sh1122_device_t));
                return NULL;
            }

            return (painter_device_t)driver;
        }
    }
    return NULL;
}

#endif // QUANTUM_PAINTER_SH1122_SPI_ENABLE

#ifdef QUANTUM_PAINTER_SH1122_I2C_ENABLE
// Factory function for creating a handle to the SH1122 device
painter_device_t qp_sh1122_make_i2c_device(uint16_t panel_width, uint16_t panel_height, uint8_t i2c_address) {
    for (uint32_t i = 0; i < SH1122_NUM_DEVICES; ++i) {
        sh1122_device_t *driver = &sh1122_drivers[i];
        if (!driver->oled.base.driver_vtable) {
            // Using 1bpp surface for now - TODO: implement 4bpp for full grayscale
            painter_device_t surface = qp_make_mono1bpp_surface_advanced(&driver->oled.surface, 1, panel_width, panel_height, driver->framebuffer);
            if (!surface) {
                return NULL;
            }

            // Setup the OLED device
            driver->oled.base.driver_vtable         = (const painter_driver_vtable_t *)&sh1122_driver_vtable;
            driver->oled.base.comms_vtable          = (const painter_comms_vtable_t *)&i2c_comms_cmddata_vtable;
            driver->oled.base.native_bits_per_pixel = 1; // Using 1bpp for now
            driver->oled.base.panel_width           = panel_width;
            driver->oled.base.panel_height          = panel_height;
            driver->oled.base.rotation              = QP_ROTATION_0;
            driver->oled.base.offset_x              = 0;
            driver->oled.base.offset_y              = 0;

            // I2C configuration
            driver->oled.base.comms_config       = &driver->oled.i2c_config;
            driver->oled.i2c_config.chip_address = i2c_address;

            if (!qp_internal_register_device((painter_device_t)driver)) {
                memset(driver, 0, sizeof(sh1122_device_t));
                return NULL;
            }

            return (painter_device_t)driver;
        }
    }
    return NULL;
}

#endif // QUANTUM_PAINTER_SH1122_I2C_ENABLE
