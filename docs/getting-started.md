# Getting Started

This guide takes you from a clean checkout to a running bootloader and your first encrypted firmware update in under fifteen minutes.

---

## Tool Ecosystem

The bootloader is one part of a three-tool chain. You need all three for a full firmware-update workflow:

| Tool | Purpose | Install |
|---|---|---|
| 🛡️ **SECURE_BOOTLOADER** *(this project)* | Runs on the MCU — decrypts, verifies, and flashes firmware | Build & flash with CMake (see below) |
| 🔐 **[EncryptBIN](https://github.com/niwciu/EncryptBIN)** | Produces the encrypted `.bin` package on the PC | `pip install` or pre-built binary |
| 📡 **[SecureLoader](https://github.com/niwciu/SecureLoader)** | Transfers the encrypted package to the device over serial | `pip install` or pre-built binary |

---

## Prerequisites

=== "Ubuntu / Debian"

    ```bash
    # ARM cross-compiler (STM32 targets)
    sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi

    # AVR cross-compiler (ATmega328P target)
    sudo apt install gcc-avr avr-libc avrdude

    # Build tools
    sudo apt install cmake ninja-build make

    # STM32 flashing — install STM32CubeProgrammer from st.com (CLI is auto-detected by CMake)
    # or open-source alternative:
    sudo apt install stlink-tools

    # Optional: code quality tools
    sudo apt install cppcheck clang-format
    pip install lizard
    ```

    **EncryptBIN** (produces the encrypted firmware package):

    ```bash
    # From source
    git clone https://github.com/niwciu/EncryptBIN.git
    cd EncryptBIN && pip install -e ".[gui]"

    # Or download a pre-built binary from the Releases page
    ```

    **SecureLoader** (uploads the encrypted package to the target):

    ```bash
    # From source
    git clone https://github.com/niwciu/SecureLoader.git
    cd SecureLoader && pip install -e ".[gui]"

    # Or download a pre-built binary from the Releases page
    ```

=== "Docker / Dev Container"

    Open the repository in VS Code and accept the prompt to reopen in the Dev Container.  
    All toolchains (ARM GCC, AVR GCC, CMake, Ninja, Python) are pre-installed in the container image.

    ```bash
    # Verify ARM GCC is available
    arm-none-eabi-gcc --version
    ```

    Install EncryptBIN and SecureLoader from source inside the container:

    ```bash
    git clone https://github.com/niwciu/EncryptBIN.git && cd EncryptBIN && pip install -e . && cd ..
    git clone https://github.com/niwciu/SecureLoader.git && cd SecureLoader && pip install -e . && cd ..
    ```

=== "Windows"

    1. Install [ARM GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
    2. Install [CMake](https://cmake.org/download/)
    3. Install [Ninja](https://github.com/ninja-build/ninja/releases)
    4. Install [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) — the CLI (`STM32_Programmer_CLI`) is auto-detected at CMake configure time
    5. Download **EncryptBIN** and **SecureLoader** Windows installers from their respective Releases pages

---

## Build

Each hardware target is configured once with CMake. After that, all build, flash, erase, and reset operations are available as predefined Ninja targets — no need to remember long CLI commands.

=== "STM32G070RB"

    ```bash
    cd hw/STM32G070RB

    # Configure once (Debug build, software CRC, default key and device ID)
    cmake -S . -B Debug -G Ninja -DBUILD_TYPE=Debug

    # Build all artefacts (.elf  .bin  .hex  .lss)
    ninja -C Debug

    # Flash to target  (requires STM32CubeProgrammer — detected at configure time)
    ninja -C Debug flash

    # Erase flash
    ninja -C Debug erase

    # Reset MCU
    ninja -C Debug reset
    ```

=== "STM32G071RB"

    ```bash
    cd hw/STM32G071RB
    cmake -S . -B Debug -G Ninja -DBUILD_TYPE=Debug
    ninja -C Debug
    ninja -C Debug flash   # flash
    ninja -C Debug erase   # erase
    ninja -C Debug reset   # reset
    ```

=== "STM32G474RE"

    ```bash
    cd hw/STM32G474RE
    cmake -S . -B Debug -G Ninja -DBUILD_TYPE=Debug
    ninja -C Debug
    ninja -C Debug flash
    ninja -C Debug erase
    ninja -C Debug reset
    ```

=== "ATmega328P"

    ```bash
    cd hw/ATMEGA328P_ARDUINO_UNO_R3
    cmake -S . -B Debug -G Ninja -DBUILD_TYPE=Debug
    ninja -C Debug
    ninja -C Debug flash         # flash via avrdude
    ninja -C Debug erase         # chip erase
    ninja -C Debug write_fuses   # program fuse bits
    ninja -C Debug read_fuses    # read current fuses
    ninja -C Debug read_flash    # read flash back
    ```

!!! tip "Programmer auto-detection"
    CMake detects `STM32_Programmer_CLI` (STM32) or `avrdude` (AVR) at configure time and creates the `flash`, `erase`, and `reset` targets automatically. If the tool is found, you will see a status message like:

    ```
    -- STM32 Programmer was found, you can work with your device using targets:
    --   flash, erase, reset.
    ```

    If the programmer is not on `PATH` at configure time, re-run CMake after adding it.

!!! tip "Production build"
    Add `-DBUILD_TYPE=Release` and supply your real device ID and AES key — see [Build System](build/cmake.md) for the full option reference.

---

## Supply the AES Key and Device ID

The AES key and device ID are injected at compile time so they never appear in source files.

```bash
cmake -S . -B Release -G Ninja \
    -DBUILD_TYPE=Release \
    -DDEVICE_ID=0x00A0000BC22510E1 \
    -DENCR_KEY="{0x21,0xBB,0xA1,0x8D,0xF4,0x9B,0x1E,0x77,0x26,0x6F,0x80,0x92,0x4C,0x35,0xE6,0xB8}"
```

See [Build System — CMake Options](build/cmake.md#cmake-options) for all accepted key formats.

---

## Verify the Bootloader is Running

Connect a UART terminal (115200 baud, 8N1) to the target's UART2 pins.

On reset, the bootloader starts a **5-tick (500 ms)** countdown. During this window it listens for a `CMD_GET_VERSION` byte (`0x01`).

Send `0x01` and you should receive:

```
0x41          # CMD_OK | CMD_GET_VERSION
0x01 0x00 0x00 0x00   # protocol_version = 1
<8 bytes>     # device ID
0x00 0x08 0x00 0x00   # flash_page_size = 2048
```

If no valid command is received before the timeout expires, the bootloader jumps to the application (if a valid image is present) or performs a system reset.

---

## Extend the Bootloader Window

Press and **hold** the USER button before or during reset. The bootloader detects the held button at startup and extends the stay-alive timeout to **`PUSH_BUTTON_DET_TIMEOUT`** ticks (60 seconds by default). Release the button; the timeout resets to **`PUSH_BUTTON_TIMEOUT`** ticks (300 seconds) after a confirmed press-release cycle.

This allows firmware updates without a host-side keep-alive loop.

---

## Update Firmware (EncryptBIN + SecureLoader)

Once the bootloader is running on the device, the firmware update workflow is:

**Step 1 — Encrypt your application binary with EncryptBIN:**

```bash
encrypt-bin \
  --input  app_firmware.bin \
  --output encrypted_firmware.bin \
  --device-id  0x00A0000BC22510E1 \
  --bootloader-id 0x00000001 \
  --key "21 BB A1 8D F4 9B 1E 77 26 6F 80 92 4C 35 E6 B8" \
  --app-version 0x20260301 \
  --prev-app-version 0x20260201
```

The AES key and device ID must match the values the bootloader was compiled with.

**Step 2 — Upload with SecureLoader CLI:**

```bash
sld flash --file encrypted_firmware.bin --port /dev/ttyUSB0 --baudrate 115200
```

**Or use the GUI:**

```bash
sld-gui
```

The GUI provides a live progress view and handles connect → handshake → transfer → disconnect automatically.

!!! tip "Configuration file"
    EncryptBIN supports a configuration file so you don't repeat flags on every call:

```bash
encrypt-bin -c my_device.txt
```

See the [EncryptBIN documentation](https://github.com/niwciu/EncryptBIN) for the config file format.

---

## Run Unit Tests

The test suite runs entirely on the host — no hardware required.

```bash
cd test/MAIN_APP
cmake -S . -B Debug -G Ninja -DBUILD_TYPE=Debug
```

After configuring, the following targets are available:

| Target | Description |
|---|---|
| `ninja -C Debug` | Build the test binary |
| `ninja -C Debug run` | Build and run all unit tests |
| `ninja -C Debug ccc` | Run tests and check code coverage (gcovr) |
| `ninja -C Debug ccr` | Generate HTML coverage report |
| `ninja -C Debug cppcheck` | Run static analysis with cppcheck |
| `ninja -C Debug ccm` | Code complexity metrics (lizard, console) |
| `ninja -C Debug ccmr` | Code complexity HTML report |
| `ninja -C Debug format` | Auto-format source files with clang-format |
| `ninja -C Debug format_check` | Check formatting without modifying files |

For the CRC module tests:

```bash
cd test/CRC
cmake -S . -B Debug -G Ninja -DBUILD_TYPE=Debug
ninja -C Debug ccc
```

See [Testing](testing/unit-tests.md) for a full description of the test suite and coverage setup.
