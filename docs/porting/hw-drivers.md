# Hardware Driver Interface Reference

All six driver modules live in `hw/<TARGET>/Core/Bl_drv_interface/`. Each module has a `.h` header that declares the interface and a `.c` implementation file. The bootloader core (`src/main_app.c`) depends only on the headers — never on the implementation files.

Start from `hw/STM32_TEMPLATE/Core/Bl_drv_interface/` which contains skeleton implementations with `TODO` comments for every function.

---

## `bl_usart_drv` — UART Communication

**Header:** `bl_usart_drv.h`

```c
typedef void (*data_rx_cb_t)(uint8_t data);

void init_bl_usart(void);
void register_data_rx_cb(data_rx_cb_t callback);
void deinit_bl_usart_periph(void);
void deinit_bl_usart_clk(void);
void send_byte_table(const uint8_t *d, size_t l);
void send_byte(uint8_t b);
bool byte_transmission_complete(void);
```

### Requirements

| Function | Contract |
|----------|----------|
| `init_bl_usart()` | Configure the UART for 115200 baud, 8N1. Enable the RX-complete interrupt. Enable the NVIC for the UART IRQ. |
| `register_data_rx_cb(cb)` | Store `cb`. Call it from the RX ISR for every received byte, passing the byte value. Must be safe to call before `init_bl_usart()`. |
| `deinit_bl_usart_periph()` | Disable the UART peripheral and its NVIC IRQ. Restore GPIO pins to their reset state. |
| `deinit_bl_usart_clk()` | Gate off the UART clock in RCC. Call **after** `deinit_bl_usart_periph()`. |
| `send_byte(b)` | Wait until the transmit data register is empty, then write `b`. Must include a timeout guard to prevent infinite blocking if the UART is in a fault state. |
| `send_byte_table(d, l)` | Call `send_byte()` for each of the `l` bytes at `d`. |
| `byte_transmission_complete()` | Return `true` when the TC (transmission complete) flag is set — i.e., the last byte has fully shifted out on the wire. Used by `do_reset()` before calling `system_reset()`. |

### ISR pattern

```c
void USARTx_IRQHandler(void)
{
    uint32_t isr = USARTx->ISR;

    /* Clear any error flags first */
    if (isr & (USART_ISR_ORE | USART_ISR_FE | USART_ISR_NE | USART_ISR_PE)) {
        USARTx->ICR = ...;
        if (isr & USART_ISR_RXNE_RXFNE) (void)USARTx->RDR;
        return;
    }

    /* Forward received byte to the protocol FSM */
    if (isr & USART_ISR_RXNE_RXFNE) {
        uint8_t c = (uint8_t)USARTx->RDR;
        if (byte_rx_cb != NULL) byte_rx_cb(c);
    }
}
```

---

## `bl_flash_drv` — Internal Flash

**Header:** `bl_flash_drv.h`

```c
void FLASH_waitBusy(void);
void FLASH_unlock(void);
void FLASH_lock(void);
void FLASH_erasePage(uint32_t flash_page_no);
void FLASH_write(uint32_t addr, uint32_t const *data, size_t dataLen);
```

### Requirements

| Function | Contract |
|----------|----------|
| `FLASH_waitBusy()` | Poll the flash busy flag (BSY in FLASH->SR) until clear. |
| `FLASH_unlock()` | Write `FLASH_KEY1` then `FLASH_KEY2` to `FLASH->KEYR`. Call `FLASH_waitBusy()` before and after. |
| `FLASH_lock()` | Set the `LOCK` bit in `FLASH->CR`. Call `FLASH_waitBusy()` before. |
| `FLASH_erasePage(page_no)` | Erase the page identified by `page_no` (0-based page index). Clear error flags first. Set PER/PNB, set STRT, wait busy, clear PER. |
| `FLASH_write(addr, data, dataLen)` | Write `dataLen` double-words (2 × `uint32_t` = 8 bytes each) starting at `addr`. Each double-word requires two consecutive 32-bit writes followed by a busy wait. |

### `FLASH_write` data layout

`dataLen` counts 8-byte units (one double-word each). The core always calls this function with `dataLen = sizeof(page_buf) / (2 * sizeof(uint32_t))` minus any adjustment for the deferred first 8 bytes.

```c
/* STM32G0/G4 typical write loop */
for (size_t i = 0; i < dataLen; i++) {
    *(volatile uint32_t *) addr        = *data++;   // low word
    *(volatile uint32_t *)(addr + 4U)  = *data++;   // high word
    FLASH_waitBusy();
    FLASH->SR &= ~FLASH_SR_EOP;
    addr += 8U;
}
```

!!! warning "Flash keys in .c only"
    Do not define `FLASH_KEY1` / `FLASH_KEY2` in `bl_hw_config.h`. Define them as file-scope constants in `bl_flash_drv.c` where they are used. Keeping unlock keys out of the public header prevents accidental exposure.

---

## `bl_key_drv` — Push-Button Input

**Header:** `bl_key_drv.h`

```c
void init_bl_key(void);
void deinit_bl_key(void);
void deinit_bl_key_clk(void);
bool bl_key_pressed(void);
```

### Requirements

| Function | Contract |
|----------|----------|
| `init_bl_key()` | Enable the GPIO clock. Configure the button pin as a digital input. Enable internal pull-up if the button is active-low (typical). |
| `deinit_bl_key()` | Reset the pin to its power-on state (analog/input, no pull). |
| `deinit_bl_key_clk()` | Gate off the GPIO port clock in RCC. Call **after** all pins on the port have been deinitialized. |
| `bl_key_pressed()` | Return `true` when the button is in the pressed state. Must be side-effect-free; the core calls it from `bl_key_check()` exactly once per main-loop iteration. |

### Active-level convention

The convention (active-low vs. active-high) is fully encapsulated inside `bl_key_drv.c`. The bootloader core only sees the logical `bool` result.

```c
/* Example: active-low button on PC13 (NUCLEO USER button) */
bool bl_key_pressed(void)
{
    bool pressed = true;
    if ((GPIOC->IDR & GPIO_IDR_ID13_Msk) == GPIO_IDR_ID13_Msk)
        pressed = false;   /* pin high → button released */
    return pressed;
}
```

---

## `bl_status_led_drv` — Status LED Output

**Header:** `bl_status_led_drv.h`

```c
void init_bl_status_led(void);
void deinit_bl_status_led_port(void);
void deinit_bl_status_led_clk(void);
void toggle_bl_led(void);
```

### Requirements

| Function | Contract |
|----------|----------|
| `init_bl_status_led()` | Enable the GPIO clock. Configure the LED pin as push-pull output. Drive it low (LED off) initially. |
| `deinit_bl_status_led_port()` | Reset the pin to its power-on state (input, no pull, LED off). |
| `deinit_bl_status_led_clk()` | Gate off the GPIO port clock. Call **after** `deinit_bl_status_led_port()`. |
| `toggle_bl_led()` | Toggle the LED output. Use `ODR ^= pin` or `BSRR`-based atomic toggle. Called from `update_bl_status_led()` every 500 ms. |

---

## `bl_crc_drv` — Hardware CRC (optional)

**Header:** `bl_crc_drv.h` (includes `crc_api.h`)

Only compiled when `CRC_MODULE=HW`. The file implements the same `crc_api.h` interface as the software implementation.

```c
/* crc_api.h — shared interface for both SW and HW implementations */
void     CRC_hw_init(void);
uint32_t CRC_init(void);
uint32_t CRC_add_byte(uint32_t crc, uint8_t b);
uint32_t CRC_add_byte_tab(uint32_t crc, uint8_t const *data, size_t dataLen);
uint32_t CRC_result(uint32_t crc);
```

### STM32 HW CRC requirements

The STM32 CRC peripheral must be configured for **CRC-32/IEEE 802.3**:

| Register field | Setting |
|----------------|---------|
| `CRC->INIT` | `0xFFFFFFFF` |
| `CRC_CR_REV_IN` | `01b` (byte reversal) |
| `CRC_CR_REV_OUT` | `1` (output reversal) |
| `CRC_CR_RESET` | Write 1 to load INIT |
| Final XOR | `^ 0xFFFFFFFF` applied in `CRC_result()` |

```c
uint32_t CRC_init(void)
{
    CRC->INIT = 0xFFFFFFFFUL;
    CRC->CR   = CRC_CR_REV_IN_0 | CRC_CR_REV_OUT | CRC_CR_RESET;
    return CRC->DR;  /* flush pipeline */
}

uint32_t CRC_add_byte_tab(uint32_t crc, uint8_t const *data, size_t dataLen)
{
    (void)crc;
    while (dataLen--)
        *((volatile uint8_t *)&CRC->DR) = *data++;
    return CRC->DR;
}

uint32_t CRC_result(uint32_t crc)
{
    (void)crc;
    return CRC->DR ^ 0xFFFFFFFFUL;
}
```

!!! note "No `rev_u32` export"
    `rev_u32` is an internal implementation detail of `crc.c`. It is declared `static` and must not appear in `bl_crc_drv.h` or `crc_api.h`.

---

## `core_drv` — Core / SysTick / App Jump

**Header:** `core_drv.h`

```c
typedef void (*SysTick_cb_t)(void);

void init_sys_tick(void);
void deinit_sys_tick(void);
void register_sysTick_cb(SysTick_cb_t callback);
void jump_to_app(void);
void system_reset(void);
bool CORE_is_valid_app(void);
```

### Requirements

| Function | Contract |
|----------|----------|
| `init_sys_tick()` | Configure SysTick for a **100 ms** interrupt. Enable the SysTick ISR. The ISR must call the registered callback. |
| `deinit_sys_tick()` | Stop SysTick (clear `CTRL` register). Clear `LOAD` and `VAL`. |
| `register_sysTick_cb(cb)` | Store `cb`. Call it from `SysTick_Handler()`. |
| `jump_to_app()` | Transfer execution to the application. See sequence below. |
| `system_reset()` | Trigger an immediate system reset. On Cortex-M: `NVIC_SystemReset()`. On AVR: watchdog with shortest timeout. |
| `CORE_is_valid_app()` | Return `true` only if the application vector table at `APP_START` contains a plausible SP and reset vector. See [Security Model](../architecture/security.md#application-validity-check). |

### `jump_to_app()` sequence (Cortex-M)

```c
void jump_to_app(void)
{
    volatile uint32_t *app_begin = (volatile uint32_t *)APP_START;
    uint32_t stack = *app_begin;
    uint32_t start = *(app_begin + 1);

    __disable_irq();

    /* Disable and clear all pending IRQs */
    NVIC->ICER[0] = 0xFFFFFFFFUL;   /* Cortex-M0/M0+: one register */
    NVIC->ICPR[0] = 0xFFFFFFFFUL;
    /* For Cortex-M4 (G474): iterate all ICER/ICPR registers */

    SCB->VTOR = APP_START;  /* relocate vector table */

    __DSB();
    __ISB();

    __set_MSP(stack);

    __DSB();
    __ISB();

    ((void (*)(void))start)();  /* branch to application reset handler */
}
```

!!! tip "SysTick period formula"
    `SysTick->LOAD = CPU_F / 10 - 1` gives exactly 100 ms at `CPU_F` Hz using the processor clock source.

---

## Driver Initialisation Order

`init_hardware()` in `main_app.c` calls drivers in this sequence:

```
1. init_bl_key()                  GPIO — safe to call first; no interrupts
2. init_bl_status_led()           GPIO — same
3. register_data_rx_cb(byte_rx_cb) register callback before enabling ISR
4. init_bl_usart()                enables RX interrupt → ISR now live
5. register_sysTick_cb(...)       register callback before enabling SysTick
6. init_sys_tick()                starts the 100 ms tick
7. CRC_hw_init()                  enable CRC peripheral clock (HW mode only)
```

`deinit_hardware()` reverses in a safe order:

```
1. deinit_bl_usart_periph()       disable ISR before touching GPIO
2. deinit_sys_tick()              stop tick
3. deinit_bl_status_led_port()    GPIO
4. deinit_bl_key()                GPIO
5. deinit_bl_usart_clk()          clock gate after peripheral disabled
6. deinit_bl_status_led_clk()     clock gate
7. deinit_bl_key_clk()            clock gate last (shared port possible)
```
