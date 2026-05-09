# System Overview

## Block Diagram

```mermaid
graph TB
    subgraph HOST["Host PC"]
        TOOL["Update Tool<br/>(Python / custom)"]
    end

    subgraph BL["Bootloader — src/main_app.c"]
        RX["UART RX ISR<br/>byte_rx_cb()"]
        FSM["Protocol FSM<br/>update_com_fsm()"]
        AES["AES-CBC<br/>tiny-AES-c"]
        CRC["CRC-32<br/>crc_api.h"]
        FLASH_W["Flash Writer<br/>write_page_to_flash()"]
        LED["Status LED<br/>update_bl_status_led()"]
        KEY["Button Poll<br/>bl_key_check()"]
        EXIT["Exit Check<br/>check_bl_exit_condition()"]
    end

    subgraph DRV["Hardware Driver Layer — hw/<TARGET>/Core/Bl_drv_interface/"]
        USART["bl_usart_drv"]
        FLASH["bl_flash_drv"]
        KEY_D["bl_key_drv"]
        LED_D["bl_status_led_drv"]
        CRC_D["bl_crc_drv"]
        CORE["core_drv"]
    end

    subgraph HW["Hardware"]
        UART_HW["UART2"]
        FLASH_HW["Internal Flash"]
        BTN["USER Button"]
        LED_HW["Status LED"]
        CRC_HW["CRC Peripheral<br/>(optional)"]
        NVIC["NVIC / SysTick"]
    end

    APP["Application<br/>@ APP_START"]

    TOOL -- "UART 115200 8N1" --> UART_HW
    UART_HW --> USART --> RX --> FSM
    FSM --> AES --> FLASH_W
    FSM --> CRC
    FLASH_W --> FLASH --> FLASH_HW
    CRC --> CRC_D --> CRC_HW
    LED_D --> LED_HW
    KEY_D --> BTN
    CORE --> NVIC
    EXIT -- "jump_to_app()" --> APP
    EXIT -- "system_reset()" --> CORE
```

---

## Module Responsibilities

### `src/main_app.c` — Bootloader Core

The only platform-independent source file. Contains:

| Function | Role |
|----------|------|
| `main_app()` | Entry point; initialises hardware, runs main loop |
| `byte_rx_cb()` | UART ISR callback; feeds bytes into the protocol FSM |
| `handle_new_cmd()` | Arms the receive buffer for multi-byte commands |
| `update_com_fsm()` | Dispatches fully received commands |
| `do_get_version()` | Replies with protocol version, device ID, flash page size |
| `do_start()` | Validates firmware header; erases app flash; initialises AES and CRC |
| `do_next_page()` | Decrypts one page; accumulates CRC; writes to flash |
| `write_page_to_flash()` | Issues the actual flash write; defers the first 8 bytes |
| `check_bl_exit_condition()` | Jumps to app or resets when the timeout expires |
| `update_bl_status_led()` | Toggles the status LED every 500 ms |
| `bl_key_check()` | Debounces the push-button; extends stay-alive timeout |
| `update_SysTick_tim()` | SysTick ISR callback; increments the 100 ms tick counter |

### `hw/<TARGET>/Core/Bl_drv_interface/` — Hardware Drivers

Six driver modules, each implementing a fixed interface. The core never accesses registers directly.

| Driver | Header | Responsibility |
|--------|--------|----------------|
| `bl_usart_drv` | `bl_usart_drv.h` | UART init/deinit, byte TX, RX ISR, callback registration |
| `bl_flash_drv` | `bl_flash_drv.h` | Flash unlock/lock, page erase, double-word write |
| `bl_key_drv` | `bl_key_drv.h` | GPIO input for push-button (active-low polling) |
| `bl_status_led_drv` | `bl_status_led_drv.h` | GPIO output for status LED |
| `bl_crc_drv` | `bl_crc_drv.h` | Hardware CRC peripheral (only compiled when `CRC_MODULE=HW`) |
| `core_drv` | `core_drv.h` | SysTick, NVIC, MSP setup, app jump, system reset, app validity |

### `src/CRC/crc.c` — Software CRC

Bit-by-bit software implementation of CRC-32/IEEE 802.3 (poly `0x04C11DB7`, refin/refout, xorout `0xFFFFFFFF`). Selected when `CRC_MODULE=SW` (the default).

### `lib/tiny-AES-c/` — AES Encryption

Third-party AES-128-CBC implementation by kokke. Used for decrypting the incoming firmware stream. The AES key is injected at compile time via the `ENCR_KEY` CMake option.

---

## Boot Flow

```mermaid
sequenceDiagram
    participant HW as Reset
    participant BL as Bootloader
    participant HOST as Host PC
    participant APP as Application

    HW->>BL: Reset (POR / WDT / software)
    BL->>BL: init_hardware()<br/>GPIO, UART, SysTick, CRC
    BL->>BL: Check USER button<br/>(bl_key_pressed())
    alt Button held
        BL->>BL: timeout = PUSH_BUTTON_DET_TIMEOUT (60 s)
    else Button released
        BL->>BL: timeout = START_TIMEOUT (500 ms)
    end

    loop Main loop
        BL->>BL: bl_key_check()
        BL->>BL: update_bl_status_led()
        BL->>BL: check_bl_exit_condition()
        BL->>BL: update_com_fsm()
    end

    HOST->>BL: CMD_GET_VERSION (0x01)
    BL->>HOST: version + device ID + page size

    HOST->>BL: CMD_START + header<br/>(version, ID, IV, page_count, CRC)
    BL->>BL: Validate header
    BL->>BL: Erase application flash
    BL->>BL: AES_CBC_init(KEY, IV)
    BL->>HOST: ACK

    loop For each firmware page
        HOST->>BL: CMD_NEXT_PAGE + encrypted page data
        BL->>BL: AES_CBC_decrypt_buffer()
        BL->>BL: CRC_add_byte_tab()
        BL->>BL: FLASH_write() (skip first 8 bytes on page 1)
        BL->>HOST: ACK
    end

    BL->>BL: CRC_result() == header.crc ?
    alt CRC matches
        BL->>BL: FLASH_write(APP_START, first 8 bytes)
        BL->>BL: timeout expires → check_bl_exit_condition()
        BL->>BL: deinit_hardware()
        BL->>APP: jump_to_app()
    else CRC mismatch
        BL->>HOST: CMD_ERR | CMD_NEXT_PAGE
    end
```

---

## Memory Map (STM32G070RB example)

```
0x08000000 ┌──────────────────────────┐
           │  Bootloader (4 KB)       │  FLASH (rx) in linker.ld
           │  ISR vectors + code      │
0x08001000 ├──────────────────────────┤  ← APP_START
           │  Application (≤124 KB)   │  Written by do_next_page()
           │                          │
           │  First 8 bytes written   │  ← deferred until CRC passes
           │  last (atomic guarantee) │
0x08020000 └──────────────────────────┘  ← APP_END
           
0x20000000 ┌──────────────────────────┐
           │  SRAM (36 KB)            │
           │  Stack, page_buf,        │
           │  volatile state          │
0x20009000 └──────────────────────────┘
```

!!! note "Page buffer alignment"
    `page_buf` is declared with `__attribute__((aligned(4)))` so the `uint32_t *` cast used during flash writes is always correctly aligned on Cortex-M0+ targets, which do not support unaligned word accesses.
