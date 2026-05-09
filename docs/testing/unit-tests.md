# Unit Tests

The bootloader has a comprehensive unit test suite that runs entirely on the host machine — no hardware required. Tests are written using [Unity](http://www.throwtheswitch.org/unity) and [CMock](http://www.throwtheswitch.org/cmock).

---

## Overview

| Test module | Location | Tests | Coverage target |
|-------------|----------|-------|-----------------|
| `MAIN_APP` | `test/MAIN_APP/` | 44 | `src/main_app.c` |
| `CRC` | `test/CRC/` | 33 | `src/CRC/crc.c` |

All hardware calls (`bl_usart_drv`, `bl_flash_drv`, `bl_key_drv`, `bl_status_led_drv`, `core_drv`, `crc_api`) are replaced by CMock-generated mocks. The core never touches real registers.

---

## Running Tests

```bash
cd test/MAIN_APP

# Configure
cmake -S . -B Debug -G Ninja -DBUILD_TYPE=Debug

# Build and run
ninja -C Debug

# Build, run, and generate gcovr HTML coverage report
ninja -C Debug ccc
```

The coverage report is written to `Debug/coverage/`.

To run the CRC tests:

```bash
cd test/CRC
cmake -S . -B Debug -G Ninja -DBUILD_TYPE=Debug
ninja -C Debug ccc
```

---

## Test Groups — MAIN_APP

### `main_app_init` (3 tests)

Tests the hardware initialisation and deinitialisation sequences.

| Test | Verifies |
|------|----------|
| `GivenBootloader_WhenInitHardwareCalled_ThenAllPeripheralsInitialisedInOrder` | `init_hardware()` calls drivers in the correct order: key → LED → register_cb → USART → SysTick → CRC |
| `GivenSysTickCallbackFires_WhenCalled_ThenTickIncremented` | The registered SysTick callback increments `_100ms_Tick` |
| `GivenBootloader_WhenDeinitHardwareCalled_ThenAllPeripheralsDeinitInOrder` | `deinit_hardware()` disables peripherals before gating clocks |

---

### `main_app_led_key` (5 tests)

Tests the status LED blinking logic and push-button debounce.

| Test | Verifies |
|------|----------|
| `GivenTickDivisibleBy5AndBlinkEventClear_WhenLEDUpdateCalled_ThenLEDToggled` | `toggle_bl_led()` is called when `_100ms_Tick % 5 == 0` and `blink_event == 0` |
| `GivenTickDivisibleBy5AndBlinkEventSet_WhenLEDUpdateCalled_ThenLEDNotToggled` | LED is not toggled if `blink_event` is already set (prevents double-toggle on slow loops) |
| `GivenTickNotDivisibleBy5_WhenLEDUpdateCalled_ThenBlinkEventCleared` | `blink_event` is cleared when the tick is not on a 5-multiple boundary |
| `GivenKeyNotPressedAndTimerZero_WhenKeyCheckCalled_ThenTimeoutUnchanged` | No timeout change when button is not pressed and `key_timer == 0` |
| `GivenPressFollowedByRelease_WhenKeyCheckCalledUntilTimerWraps_ThenTimeoutExtended` | After `F_KEY_DELAY` release iterations the timer wraps to `UINT8_MAX`, triggering `PUSH_BUTTON_TIMEOUT` |

---

### `main_app_rx_fsm` (13 tests)

Tests the UART receive state machine (`byte_rx_cb` / `handle_new_cmd`).

| Test | Verifies |
|------|----------|
| `GivenComStart_WhenNonVersionByteReceived_ThenStateRemainsComStart` | Only `CMD_GET_VERSION` is accepted in `COM_START` |
| `GivenComStart_WhenGetVersionByteReceived_ThenStateBecomesComDone` | `CMD_GET_VERSION` transitions to `COM_DONE` |
| `GivenComStart_WhenGetVersionByteReceived_ThenCmdIsGetVersion` | `cmd` variable is set correctly |
| `GivenComReady_WhenCmdStartReceived_ThenStateBecomesComNextByte` | `CMD_START` arms the header receive buffer |
| `GivenComReady_WhenCmdStartReceived_ThenPageTailEqualsHeaderSize` | `page_tail` is set to `sizeof(header_t)` |
| `GivenComReady_WhenCmdNextPageReceived_ThenStateBecomesComNextByte` | `CMD_NEXT_PAGE` arms the page receive buffer |
| `GivenComReady_WhenCmdNextPageReceived_ThenPageTailEqualsPageBufSize` | `page_tail` is set to `FLASH_PAGE_SIZE` |
| `GivenComReady_WhenUnknownCmdReceived_ThenStateBecomesComDone` | Unknown commands go directly to `COM_DONE` |
| `GivenComNextByte_WhenByteReceived_ThenByteStoredAtBufPtr` | Bytes are stored at `*buf_ptr` |
| `GivenComNextByte_WhenByteReceived_ThenPageTailDecremented` | `page_tail` decrements on each byte |
| `GivenComNextByte_WhenByteReceivedButNotLast_ThenStateRemainsComNextByte` | State stays in `COM_NEXT_BYTE` until the last byte |
| `GivenComNextByte_WhenLastByteReceived_ThenStateBecomesComDone` | Last byte transitions to `COM_DONE` |
| `GivenComDone_WhenAnyByteReceived_ThenStateRemainsComDone` | `COM_DONE` ignores additional bytes |

---

### `main_app_com_fsm` (7 tests)

Tests the command dispatch logic (`update_com_fsm`).

| Test | Verifies |
|------|----------|
| `GivenStateNotComDone_WhenComFsmUpdated_ThenNoCmdDispatched` | No action taken unless `com_state == COM_DONE` |
| `GivenComDoneWithGetVersion_WhenComFsmUpdated_ThenVersionResponseSent` | `do_get_version()` sends `CMD_OK|CMD_GET_VERSION` + version + device ID + page size |
| `GivenComDoneWithReset_WhenComFsmUpdated_ThenResetAckSentAndSystemResetCalled` | `do_reset()` sends ACK, waits for TX complete, then calls `system_reset()` |
| `GivenComDoneWithUnknownCmd_WhenComFsmUpdated_ThenErrSent` | Unknown commands receive `CMD_ERR` |
| `GivenComDoneWithCmdStart_WhenComFsmUpdated_ThenDoStartDispatched` | `CMD_START` calls `do_start()` |
| `GivenComDoneWithCmdNextPage_WhenComFsmUpdated_ThenDoNextPageDispatched` | `CMD_NEXT_PAGE` calls `do_next_page()` |
| `GivenComDone_WhenComFsmUpdated_ThenTimeoutRefreshedByCommTimeout` | After dispatch, `timeout = _100ms_Tick + COMM_TIMEOUT` |

---

### `main_app_start_cmd` (11 tests)

Tests the `CMD_START` handler, including all header validation rules.

| Test | Verifies |
|------|----------|
| `GivenWrongProtocolVersion_WhenStartCmdExecuted_ThenErrStartSent` | Wrong `protocol_version` → `CMD_ERR | CMD_START` |
| `GivenWrongProductIdMSB_WhenStartCmdExecuted_ThenErrStartSent` | Wrong upper device ID → rejected |
| `GivenWrongProductIdLSB_WhenStartCmdExecuted_ThenErrStartSent` | Wrong lower device ID → rejected |
| `GivenWrongFlashPageSize_WhenStartCmdExecuted_ThenErrStartSent` | Wrong `flash_page_size` → rejected |
| `GivenPageCountZero_WhenStartCmdExecuted_ThenErrStartSent` | `page_count == 0` → rejected |
| `GivenPageCountExceedsAppRegion_WhenStartCmdExecuted_ThenErrStartSent` | `page_count > max_pages` → rejected (prevents uncontrolled erase) |
| `GivenValidHeader_WhenStartCmdExecuted_ThenFlashUnlockedAndLocked` | `FLASH_unlock()` and `FLASH_lock()` are called |
| `GivenValidHeader_WhenStartCmdExecuted_ThenEveryAppPageErased` | `FLASH_erasePage()` called for each page from `APP_START_PAGE` to `APP_LAST_PAGE` |
| `GivenValidHeader_WhenStartCmdExecuted_ThenAesInitCalledWithHeaderIV` | `AES_CBC_init()` receives the IV from the header |
| `GivenValidHeader_WhenStartCmdExecuted_ThenCRCInitialisedAndPagePosSet` | `CRC_init()` called; `page_pos == APP_START` |
| `GivenValidHeader_WhenStartCmdExecuted_ThenOkStartSent` | Success response `CMD_OK | CMD_START` sent |

---

### `main_app_page_cmd` (9 tests)

Tests the `CMD_NEXT_PAGE` handler, including the deferred first-word write and CRC verification.

| Test | Verifies |
|------|----------|
| `GivenPagePosBeforeAppStart_WhenNextPageCalled_ThenErrNextPageSent` | Out-of-range write position → `CMD_ERR | CMD_NEXT_PAGE` |
| `GivenPagePosAtOrAfterAppEnd_WhenNextPageCalled_ThenErrNextPageSent` | Write beyond `APP_END` → rejected |
| `GivenPageRemZero_WhenNextPageCalled_ThenErrNextPageSent` | `page_rem == 0` → rejected (protocol violation) |
| `GivenFirstPageNotLast_WhenNextPageCalled_ThenFirstEightBytesBufferedNotWritten` | First 8 bytes of first page are stored in `app_begin_word[]`, not written to `APP_START` |
| `GivenFirstPageNotLast_WhenNextPageCalled_ThenOkNextPageSent` | Success response sent |
| `GivenFirstPageNotLast_WhenNextPageCalled_ThenPagePosAdvanced` | `page_pos` advances by `FLASH_PAGE_SIZE` |
| `GivenLastPageWithCRCMismatch_WhenNextPageCalled_ThenErrNextPageSent` | CRC mismatch on final page → `CMD_ERR | CMD_NEXT_PAGE` |
| `GivenLastPageWithCRCMatch_WhenNextPageCalled_ThenFirstBytesWrittenToFlash` | On CRC match, `FLASH_write(APP_START, app_begin_word, 1)` is called |
| `GivenLastPageWithCRCMatch_WhenNextPageCalled_ThenOkNextPageSent` | Success response sent |

---

### `main_app_exit` (5 tests)

Tests the bootloader exit logic (`check_bl_exit_condition`).

| Test | Verifies |
|------|----------|
| `GivenTickBelowTimeout_WhenExitCheckCalled_ThenNoAction` | No exit action when `_100ms_Tick < timeout` |
| `GivenTickAtTimeoutAndValidApp_WhenExitCheckCalled_ThenDeinitAndJumpToApp` | Valid app + timeout → `deinit_hardware()` then `jump_to_app()` |
| `GivenTickAtTimeoutAndInvalidApp_WhenExitCheckCalled_ThenSystemReset` | No valid app → `do_reset()` |
| `GivenTickAtTimeoutAndMidReceive_WhenExitCheckCalled_ThenSystemResetWithoutValidityCheck` | `COM_NEXT_BYTE` state → reset regardless of app validity (protects partial transfer) |
| `GivenTickExceedsTimeout_WhenExitCheckCalled_ThenConditionAlsoTriggered` | `_100ms_Tick > timeout` also triggers exit |

---

## Mock Infrastructure

Hardware driver calls are intercepted by CMock-generated mocks in `test/MAIN_APP/cmocks/`:

| Mock file | Replaces |
|-----------|---------|
| `mock_bl_usart_drv.c/h` | `bl_usart_drv.h` — all UART functions |
| `mock_bl_flash_drv.c/h` | `bl_flash_drv.h` — all flash functions |
| `mock_bl_key_drv.c/h` | `bl_key_drv.h` — `bl_key_pressed()` |
| `mock_bl_status_led_drv.c/h` | `bl_status_led_drv.h` — `toggle_bl_led()` |
| `mock_core_drv.c/h` | `core_drv.h` — jump, reset, validity |
| `mock_crc_api.c/h` | `crc_api.h` — CRC functions |
| `mock_aes.c/h` | `aes.h` — `AES_CBC_init`, `AES_CBC_decrypt_buffer` |

The `PRIVATE` macro in `main_app.c` is defined as empty (`#define PRIVATE`) when `UNIT_TESTS` is set, making internal `static` variables externally visible so tests can read and set them directly.

---

## Internals Visible to Tests

The following internal variables and functions are declared `extern` in the test files when `UNIT_TESTS` is defined:

```c
extern volatile uint32_t _100ms_Tick;
extern uint32_t timeout;
extern uint8_t blink_event;
extern uint8_t key_timer;
void update_bl_status_led(void);
void bl_key_check(void);
```

This approach avoids test-only getter/setter boilerplate while keeping the production build fully `static`.

---

## Coverage

Coverage is measured with `gcovr` after running `ninja ccc`. The report is generated in `Debug/coverage/index.html`.

!!! tip "Gcovr merge mode"
    The build system passes `--merge-mode-functions=separate` to gcovr because `main_app.c` is compiled once but attributed across multiple test binaries. This prevents false "multiple definition" warnings in the coverage merge step.
