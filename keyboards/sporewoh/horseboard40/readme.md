# Horseboard40

![Horseboard40](https://i.imgur.com/T8wp7cEh.jpg)

Planck-like development board for QMK, using SparkFun MicroMod for hot-swap MCU capability.

* Keyboard Maintainer: [ChrisChrisLoLo](https://github.com/ChrisChrisLoLo)
* Hardware Supported: Horseboard40 rev1 PCB
* Hardware Availability: [Open-source](https://github.com/sporewoh/horseboard40)

Make example for this keyboard (after setting up your build environment):

    make sporewoh/horseboard40/rev1/lcd:default

Flashing example for this keyboard:

    make sporewoh/horseboard40/rev1/lcd:default:flash

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in 3 ways:

* **Bootmagic reset**: Hold down the top-left key (usually Escape) and plug in the keyboard
* **Physical reset button**: Either press all 5 left-most keys on the bottom row, or hold `BOOT` and press `RESET`
* **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available
