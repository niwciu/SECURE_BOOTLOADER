# Porting Guide

Adding a new MCU target requires implementing **six driver modules** and one configuration header. The bootloader core (`src/main_app.c`) does not need modification.

## Steps at a Glance

1. Copy `hw/STM32_TEMPLATE/` to `hw/<YOUR_TARGET>/`
2. Fill in `Core/Inc/bl_hw_config.h` with your MCU's memory map and timing constants
3. Implement the six driver modules in `Core/Bl_drv_interface/`
4. Write `Core/Src/main.c` — the hardware entry point
5. Add a linker script and toolchain file
6. Add a `CMakeLists.txt` following the existing target pattern

| Page | Contents |
|------|----------|
| [Hardware Drivers](hw-drivers.md) | Full interface specification for all six driver modules |
| [Configuration Reference](configuration.md) | Every constant in `bl_hw_config.h` explained |
