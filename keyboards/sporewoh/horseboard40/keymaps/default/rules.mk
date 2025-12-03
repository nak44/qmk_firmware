NKRO_ENABLE = yes
ENCODER_MAP_ENABLE = yes
DEBUG_MATRIX_SCAN_RATE_ENABLE = yes
WPM_ENABLE = yes

# Conditionally include the correct UI based on keyboard variant
ifneq (,$(findstring /lcd,$(KEYBOARD)))
    SRC += ui_lcd/ui.c
    SRC += keyboards/sporewoh/horseboard40/graphics/lcd/animu-image-lcd.qgf.c
else
    SRC += ui_oled/ui.c
endif
