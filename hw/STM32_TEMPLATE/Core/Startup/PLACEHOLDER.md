# Startup file

Place the MCU-specific startup assembly file here, then apply the bootloader
modification described below.

## Where to get the file

- **STM32CubeIDE / STM32CubeMX**: generate a project for your MCU; the file
  appears in `Core/Startup/` named `startup_stm32<variant>.s`.
- **CMSIS device pack**: copy from
  `Device/Source/Templates/gcc/startup_<device>.s`.

Update `CMakeLists.txt → APP_ASM_SOURCES` to reference the correct filename.

---

## Required bootloader modification

The standard CubeMX startup file calls `SystemInit()` before `main()`:

```asm
/* Call the clock system initialization function.*/
    bl  SystemInit
```

**Comment it out and add a `.weak SystemInit` declaration:**

```asm
/* Call the clock system initialization function.*/
 // bl  SystemInit
 /* Bootloader approach: SystemInit() is intentionally skipped.
  * The MCU boots from the internal HSI oscillator (typically 8–16 MHz) by
  * default — sufficient for 115200 baud UART without any clock config.
  * The FPU enable block inside SystemInit() is also a no-op here because
  * the bootloader compiles with -mfloat-abi=soft (__FPU_USED == 0).
  * system_stm32xxxx.c is therefore not compiled (see CMakeLists.txt).
  * SystemInit is declared .weak below so the linker does not require it. */
```

And near the end of the file, with the other `.weak` declarations:

```asm
    .weak   SystemInit
```

### Why this is safe for the bootloader

| Concern | Why it is not an issue |
|---------|------------------------|
| Clock speed | HSI reset default (8–16 MHz depending on family) is enough for 115200 baud. `bl_hw_config.h → CPU_F` must match. |
| Flash wait states | At the HSI default frequency, 0 wait states is correct on all STM32 families used here. |
| FPU enable | Bootloader compiled with `-mfloat-abi=soft` → `__FPU_USED == 0` → FPU block inside `SystemInit()` is already skipped by `#if`. |
| VTOR relocation | Not needed; the bootloader sits at flash base `0x08000000`. |
| `SystemCoreClock` variable | Never read; the bootloader uses the constant `CPU_F` from `bl_hw_config.h` directly. |

### Reference: STM32G474RE reset default clock

```
Clock source : HSI16 (16 MHz internal oscillator)
PLL          : off
SYSCLK       : 16 MHz
AHB/APB      : /1  →  all buses at 16 MHz
Flash WS     : 0   (correct for ≤ 30 MHz on STM32G4)
```

Verify the reset-default values for your specific MCU family in its
Reference Manual (`RCC` chapter, reset values table).
