# Build System

Each hardware target has its own `CMakeLists.txt` in `hw/<TARGET>/`. All targets share the same source pool (`src/`, `lib/`) and differ only in driver sources, toolchain, and configuration.

---

## Quick Reference

```bash
# Debug build — software CRC, default key, default device ID
cmake -S hw/STM32G070RB -B hw/STM32G070RB/Debug -G Ninja -DBUILD_TYPE=Debug
ninja -C hw/STM32G070RB/Debug

# Release build — hardware CRC, real key, real device ID
cmake -S hw/STM32G070RB -B hw/STM32G070RB/Release -G Ninja \
    -DBUILD_TYPE=Release \
    -DCRC_MODULE=HW \
    -DDEVICE_ID=0x00A0000BC22510E1 \
    -DENCR_KEY="{0x21,0xBB,0xA1,0x8D,0xF4,0x9B,0x1E,0x77,0x26,0x6F,0x80,0x92,0x4C,0x35,0xE6,0xB8}"
ninja -C hw/STM32G070RB/Release
```

---

## CMake Options

| Option | Values | Default | Description |
|--------|--------|---------|-------------|
| `BUILD_TYPE` | `Debug`, `Release` | `Debug` | Selects compiler optimisation level and debug symbols. Debug: `-Og -g`. Release: `-O2`. |
| `CRC_MODULE` | `SW`, `HW` | `SW` | Selects the CRC implementation. `SW` links `src/CRC/crc.c`. `HW` links `hw/<TARGET>/Core/Bl_drv_interface/bl_crc_drv.c` and uses the MCU CRC peripheral. |
| `DEVICE_ID` | 64-bit hex | `0x0000000000000000` | The device identifier burned into the bootloader. The firmware header must carry the same value or the upload is rejected. |
| `ENCR_KEY` | 16-byte value | Default demo key | The AES-128 encryption key. See [Key Formats](#encr_key-formats) below. |

### Inspecting active options

CMake prints the active configuration at configure time:

```
-- Build configuration:
--   Available options:
--     BUILD_TYPE  : <Debug, Release> (default: Debug)
--     CRC_MODULE  : <HW, SW> (default: SW)
--     DEVICE_ID   : <64-bit hex value>
--                   example: -DDEVICE_ID=0x00A0000BC22510E1
--     ENCR_KEY    : <16 bytes key>
--   Active settings:
--     BUILD_TYPE  = Release
--     CRC_MODULE  = HW
--     DEVICE_ID   = 0x00A0000BC22510E1
--     ENCR_KEY    = {0x21,0xBB,...}
```

---

## `ENCR_KEY` Formats

The CMake build system accepts the key in three formats and normalises them to a C initialiser list. All three produce identical binaries.

=== "C Initialiser (recommended)"

    ```bash
    -DENCR_KEY="{0x21,0xBB,0xA1,0x8D,0xF4,0x9B,0x1E,0x77,0x26,0x6F,0x80,0x92,0x4C,0x35,0xE6,0xB8}"
    ```

=== "Hex String (no separators)"

    ```bash
    -DENCR_KEY="21BBA18DF49B1E77266F80924C35E6B8"
    ```

=== "Hex String (colon-separated)"

    ```bash
    -DENCR_KEY="21:BB:A1:8D:F4:9B:1E:77:26:6F:80:92:4C:35:E6:B8"
    ```

The key is injected as a preprocessor definition:

```cmake
target_compile_definitions(${TARGET_NAME} PRIVATE
    ENCR_KEY=${ENCR_KEY}
    DEVICE_ID=${DEVICE_ID}
    ...
)
```

And consumed in `main_app.c`:

```c
static const uint8_t KEY[] = ENCR_KEY;
```

!!! warning "Key security"
    The key is stored in the compiled binary's flash. Enable STM32 Read-Out Protection (RDP Level 1 or 2) in production to prevent extraction via the debug port.

    Never commit the production key to version control. Supply it via CI/CD secrets or a hardware security module (HSM).

---

## `DEVICE_ID` Format

Any 64-bit hexadecimal value:

```bash
-DDEVICE_ID=0x00A0000BC22510E1
```

The value is split into MSB and LSB halves and placed into `DEVICE_ID_MSB` and `DEVICE_ID_LSB` preprocessor definitions. In `do_start()`:

```c
uint64_t product_id = ((uint64_t)header.product_ID_MSB << 32)
                    | (uint64_t)header.product_ID_LSB;
if (product_id != DEVICE_ID) { ... reject ... }
```

---

## CRC Module Selection

```cmake
if(CRC_MODULE STREQUAL "SW")
    # Software CRC — platform-independent, no peripheral needed
    list(APPEND APP_C_SOURCES ../../src/CRC/crc.c)
else()
    # Hardware CRC — target-specific peripheral driver
    list(APPEND APP_C_SOURCES Core/Bl_drv_interface/bl_crc_drv.c)
endif()
```

Both implementations expose the same `crc_api.h` interface and produce byte-identical CRC values for the same input.

Use `CRC_MODULE=HW` in production on STM32 targets if flash size is tight or if you want the CRC computation to run faster. Use `CRC_MODULE=SW` during early bring-up or on targets without a CRC peripheral.

---

## Toolchains

Toolchain files are in `hw/config/`.

| Target family | Toolchain file |
|---------------|----------------|
| STM32 (all) | `hw/config/STM32/Toolchain-arm-gcc.cmake` |
| ATmega328P | `hw/config/AVR/Toolchain-avr-gcc.cmake` |

The toolchain is set automatically by `CMakeLists.txt`:

```cmake
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../config/STM32/Toolchain-arm-gcc.cmake)
```

Override with `-DCMAKE_TOOLCHAIN_FILE=...` only if you need a non-standard compiler path.

---

## Build Outputs

After a successful build, the output directory (`Debug/` or `Release/`) contains:

| File | Description |
|------|-------------|
| `<TARGET>.elf` | ELF binary with debug symbols (for GDB and flashing tools) |
| `<TARGET>.bin` | Raw binary for `st-flash` |
| `<TARGET>.hex` | Intel HEX for STM32CubeProgrammer and avrdude |
| `<TARGET>.map` | Linker map — check bootloader size and symbol placement |

!!! tip "Checking bootloader size"
    The linker script limits the bootloader to 4 KB (`LENGTH = 4K`). If your build produces an ELF that does not fit, the linker will error. Check the map file for large symbols and consider enabling Release mode or disabling unused features.

---

## Running the Tests

The unit test suite builds independently of hardware targets.

```bash
# MAIN_APP tests
cd test/MAIN_APP
cmake -S . -B Debug -G Ninja -DBUILD_TYPE=Debug
ninja -C Debug
ninja -C Debug ccc     # build + run + coverage report

# CRC module tests
cd test/CRC
cmake -S . -B Debug -G Ninja -DBUILD_TYPE=Debug
ninja -C Debug
ninja -C Debug ccc
```

See [Testing](../testing/unit-tests.md) for details.
