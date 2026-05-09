# Configuration Reference

`bl_hw_config.h` is the single file that makes the generic bootloader core aware of your target's hardware. It lives at `hw/<TARGET>/Core/Inc/bl_hw_config.h`.

---

## Complete Example (STM32G070RB)

```c
#ifndef HW_BL_CONFIG_H_
#define HW_BL_CONFIG_H_

#include <stdint.h>

/* ---- Bootloader timing (1 tick = 100 ms) --------------------------------- */
#define F_KEY_DELAY              100U
#define START_TIMEOUT            5U
#define COMM_TIMEOUT             10U
#define PUSH_BUTTON_TIMEOUT      3000U
#define PUSH_BUTTON_DET_TIMEOUT  600U

/* ---- Protocol ------------------------------------------------------------ */
#define PROTOCOL_VERSION         0x00000001UL

/* ---- Peripheral settings ------------------------------------------------- */
#define CPU_F                    16000000U
#define BAUD                     115200U
#define IV_BLOCK_SIZE            16U

/* ---- Flash map ----------------------------------------------------------- */
#define FLASH_START_ADR          0x08000000UL
#define APP_START                0x08001000UL
#define APP_END                  0x08020000UL
#define FLASH_PAGE_SIZE          2048U

/* ---- SRAM map ------------------------------------------------------------ */
#define SRAM_START               0x20000000UL
#define SRAM_END                 (SRAM_START + (36U * 1024U))

/* ---- Derived (DO NOT EDIT) ----------------------------------------------- */
#define APP_START_PAGE           ((APP_START - FLASH_START_ADR) / FLASH_PAGE_SIZE)
#define APP_LAST_PAGE            ((APP_END   - FLASH_START_ADR) / FLASH_PAGE_SIZE - 1U)

#endif /* HW_BL_CONFIG_H_ */
```

---

## Constant Reference

### Timing

All timing constants use the **100 ms tick** as the unit. The tick is incremented by `update_SysTick_tim()` which is registered as the SysTick callback in `init_hardware()`.

| Constant | Type | Unit | Default | Description |
|----------|------|------|---------|-------------|
| `F_KEY_DELAY` | `uint8_t`-range | loop iterations | `100` | Number of `bl_key_check()` iterations after button release before the press-release cycle is confirmed. Provides software debounce. Increase for noisier buttons. |
| `START_TIMEOUT` | `uint32_t` | ticks (100 ms each) | `5` (500 ms) | How long the bootloader waits at startup before jumping to the application or resetting. Set to at least `3` to allow time for the host to connect. |
| `COMM_TIMEOUT` | `uint32_t` | ticks | `10` (1 s) | Added to `_100ms_Tick` after each successful command. The resulting value becomes the new deadline. A silent interval longer than this causes a timeout exit. |
| `PUSH_BUTTON_TIMEOUT` | `uint32_t` | ticks | `3000` (300 s) | Timeout applied after a confirmed button press-release cycle. Gives a large window for a manual firmware update. |
| `PUSH_BUTTON_DET_TIMEOUT` | `uint32_t` | ticks | `600` (60 s) | Timeout applied at startup when the button is detected as already held. Provides time for the host to connect before the user releases the button. |

### Protocol

| Constant | Type | Description |
|----------|------|-------------|
| `PROTOCOL_VERSION` | `uint32_t` | Must match the value sent in the firmware header's `protocol_version` field. Increment when the wire protocol changes in a breaking way. Current value: `0x00000001UL`. |

### Peripheral Settings

| Constant | Type | Description |
|----------|------|-------------|
| `CPU_F` | `uint32_t` | CPU clock frequency in Hz at the time the bootloader runs. Used by `core_drv.c` to compute the SysTick reload value (`CPU_F / 10 - 1` for 100 ms) and by `bl_usart_drv.c` for the BRR register (`CPU_F / BAUD`). **Must match the actual clock** — no PLL is configured by the bootloader. |
| `BAUD` | `uint32_t` | UART baud rate. Default `115200`. Change this together with the host tool configuration. |
| `IV_BLOCK_SIZE` | `uint8_t`-range | AES-CBC IV length in bytes. **Fixed at `16U`. Do not change.** |

### Flash Map

All flash addresses and sizes must come from the MCU Reference Manual.

| Constant | Type | Description |
|----------|------|-------------|
| `FLASH_START_ADR` | `uint32_t` | Base address of the internal flash. STM32: `0x08000000UL`. AVR: `0x0000`. |
| `APP_START` | `uint32_t` | Start address of the application region — immediately after the bootloader. The bootloader occupies everything from `FLASH_START_ADR` to `APP_START - 1`. Default: `0x08001000UL` (4 KB bootloader). |
| `APP_END` | `uint32_t` | First address **past** the end of the application region (exclusive). Must be page-aligned. |
| `FLASH_PAGE_SIZE` | `uint32_t` | Flash page or sector size in bytes. Must match the value sent in `CMD_GET_VERSION` and checked in the firmware header. STM32G0/G4: `2048U`. STM32F4 sectors: varies. |

!!! warning "Linker script alignment"
    `APP_START` must match the `ORIGIN` of the application's linker script. The bootloader linker script must have `LENGTH` equal to `APP_START - FLASH_START_ADR`.

!!! warning "FLASH_KEY1 / FLASH_KEY2 do not belong here"
    Flash unlock keys are implementation details of `bl_flash_drv.c`. Define them as `static const` in the `.c` file, not in this header. Keeping them here would expose them to any file that includes `bl_hw_config.h`.

### SRAM Map

Used by `CORE_is_valid_app()` to validate the application's initial stack pointer.

| Constant | Type | Description |
|----------|------|-------------|
| `SRAM_START` | `uint32_t` | Base address of internal SRAM. STM32: `0x20000000UL`. |
| `SRAM_END` | `uint32_t` | Inclusive upper bound of SRAM. Computed as `SRAM_START + (size_in_KB * 1024U)`. |

### Derived Constants (Do Not Edit)

These are computed from the values above and used by the bootloader core.

| Constant | Formula | Meaning |
|----------|---------|---------|
| `APP_START_PAGE` | `(APP_START - FLASH_START_ADR) / FLASH_PAGE_SIZE` | Zero-based index of the first application page. Passed to `FLASH_erasePage()` as the start of the erase loop. |
| `APP_LAST_PAGE` | `(APP_END - FLASH_START_ADR) / FLASH_PAGE_SIZE - 1U` | Zero-based index of the last application page (inclusive). Passed to `FLASH_erasePage()` as the end of the erase loop. |

!!! danger "Parenthesisation"
    `APP_LAST_PAGE` **must** have `-1U` inside the outer parentheses:
    ```c
    #define APP_LAST_PAGE  ((APP_END - FLASH_START_ADR) / FLASH_PAGE_SIZE - 1U)  // correct
    #define APP_LAST_PAGE  ((APP_END - FLASH_START_ADR) / FLASH_PAGE_SIZE) - 1   // WRONG
    ```
    The second form is not fully parenthesised and will produce wrong results when the macro appears in expressions like `a <= APP_LAST_PAGE + n`.

---

## Target Comparison

| Constant | STM32G070RB | STM32G071RB | STM32G474RE | ATmega328P |
|----------|-------------|-------------|-------------|------------|
| `CPU_F` | `16000000U` | `16000000U` | `16000000U` | `16000000UL` |
| `FLASH_PAGE_SIZE` | `2048U` | `2048U` | `2048U` | `SPM_PAGESIZE` (128) |
| `APP_START` | `0x08001000UL` | `0x08001000UL` | `0x08001000UL` | `0x0000` + BL offset |
| `APP_END` | `0x08020000UL` | `0x08020000UL` | `0x08080000UL` | `0x7800` |
| `SRAM_START` | `0x20000000UL` | `0x20000000UL` | `0x20000000UL` | `0x0100` (AVR data) |
| `SRAM_END` | `+36 KB` | `+36 KB` | `+128 KB` | `+2 KB` |
