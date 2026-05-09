# Changelog

All notable changes to this project are documented here.
Format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
Versions are tagged as `vX.Y.Z` in the repository.

---

## [v1.0.0] - 2026-05-09

### ✨ Added

#### Core bootloader

- 🔐 **AES-128-CBC encrypted firmware transport** — per-image IV embedded in the firmware header; key injected at compile time as a preprocessor definition
- ✅ **CRC-32/IEEE 802.3 integrity verification** — computed over the complete decrypted firmware image before any flash write is committed
- ⚛️ **Atomic flash write** — application reset vector deferred and written last, only after the full-image CRC passes; a partial or interrupted transfer can never produce a bootable image
- 🔑 **64-bit device ID check** — firmware built for a different target is rejected at header validation; prevents cross-device flashing
- 🔘 **Push-button stay-alive** — press-release cycle extends the bootloader window to `PUSH_BUTTON_TIMEOUT` (300 s default) without host intervention; button held at reset extends to `PUSH_BUTTON_DET_TIMEOUT` (60 s default)
- ⏱️ **Configurable timeouts** — `START_TIMEOUT`, `COMM_TIMEOUT`, `PUSH_BUTTON_TIMEOUT`, `PUSH_BUTTON_DET_TIMEOUT` (all in 100 ms ticks via `bl_hw_config.h`)
- ⚙️ **Selectable CRC implementation** — `CRC_MODULE=SW` (platform-independent) or `CRC_MODULE=HW` (STM32 CRC peripheral), chosen at CMake configure time; both implementations expose the same `crc_api.h` interface

#### Hardware targets

| Target | MCU | Core | Flash | CRC modes |
|---|---|---|---|---|
| STM32G070RB | STM32G070RBTx | Cortex-M0+ | 128 KB | SW + HW |
| STM32G071RB | STM32G071RBTx | Cortex-M0+ | 128 KB | SW + HW |
| STM32G474RE | STM32G474RETx | Cortex-M4F | 512 KB | SW + HW |
| ATmega328P (Arduino Uno R3) | ATmega328P | AVR8 | 32 KB | SW |

All STM32 targets occupy the first **4 KB** of flash; the application region starts immediately after.

#### Architecture

- 🧩 **Platform-independent core** (`src/main_app.c`) — all hardware calls delegated to a six-driver interface (`bl_usart_drv`, `bl_flash_drv`, `bl_key_drv`, `bl_status_led_drv`, `core_drv`, `crc_api`)
- 📐 **`STM32_TEMPLATE`** porting skeleton with `TODO`-annotated driver stubs for adding new STM32 targets
- Single `bl_hw_config.h` per target — flash map, timing constants, and peripheral settings in one place

#### Unit tests

- 🧪 **44 tests** for the bootloader core (`test/MAIN_APP`) — full protocol FSM, header validation, page write, CRC check, exit conditions, LED/key logic
- 🧪 **33 tests** for the CRC module (`test/CRC`) — algorithm correctness and all public API functions
- Tests run on the host with **Unity + CMock**; no hardware required
- CMock-generated mocks isolate the core from all hardware drivers
- `gcovr` code coverage with ≥ 90 % line threshold; HTML report generated automatically

#### Build system

- 🛠️ CMake + Ninja with predefined `flash`, `erase`, `reset` targets — `STM32_Programmer_CLI` and `avrdude` auto-detected at configure time
- AES key accepted in three formats: C initialiser list, plain hex string, colon-separated hex
- AVR targets: additional `write_fuses`, `read_fuses`, `read_flash` targets

#### CI / CD

- 🤖 **GitHub Actions CI pipeline** — builds all HW targets (SW CRC + HW CRC variants), runs unit tests, enforces ≥ 90 % coverage, cppcheck static analysis, Lizard complexity (CCN ≤ 12)
- 📄 **MkDocs Material documentation** site with auto-deployed coverage and complexity reports on every merge to `main`
- 🚀 **Automated release workflow** — detects new `vX.Y.Z` entries in `CHANGELOG.md` on PR merge and creates a tagged GitHub release with the extracted release notes

#### Tool ecosystem

- Compatible with **[EncryptBIN](https://github.com/niwciu/EncryptBIN)** for producing AES-CBC encrypted firmware packages on the host
- Compatible with **[SecureLoader](https://github.com/niwciu/SecureLoader)** for uploading encrypted packages to the target over serial (CLI + GUI)
