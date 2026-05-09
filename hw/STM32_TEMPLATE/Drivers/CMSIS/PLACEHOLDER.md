# CMSIS Drivers

Place the MCU-specific CMSIS files here, mirroring the structure used by
existing targets (e.g. STM32G070RB):

```
Drivers/CMSIS/
├── Device/          ← vendor device header  (stm32<variant>.h, system_stm32<family>.h)
├── Include/         ← core CMSIS headers    (core_cm*.h, cmsis_gcc.h, …)
└── STM32XX_Drivers/ ← optional: STM32Cube HAL/LL headers if needed
```

Source:
- STM32CubeMX project export, or
- STM32Cube firmware package (`Drivers/CMSIS/Device/ST/STM32<family>/`)
- CMSIS 5 / CMSIS 6 core headers from https://github.com/ARM-software/CMSIS_5

After copying, update `CMakeLists.txt → APP_INCLUDE_DIRS` to point to
the correct sub-directories.
