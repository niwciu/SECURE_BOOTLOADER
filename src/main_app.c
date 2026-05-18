/**
 * @file main_app.c
 * @author niwciu (niwciu@gmail.com)
 * @brief Platform-independent secure bootloader core.
 *
 *        Receives AES-CBC-encrypted, CRC-32-verified firmware over UART,
 *        writes it page-by-page to flash, and boots a validated application.
 *        All hardware operations are delegated to platform-specific drivers
 *        found under hw/<TARGET>/Core/Bl_drv_interface/.
 *
 * @version 1.0.0
 * @date 2025-11-03
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "main_app.h"
#include "main_app_priv.h"
#include "bl_usart_drv.h"
#include "core_drv.h"
#include "bl_flash_drv.h"
#include "bl_hw_config.h"
#include "bl_key_drv.h"
#include "bl_status_led_drv.h"
#include "aes.h"
#include "crc_api.h"
#include <stdbool.h>
#include <stddef.h>

/** Number of bytes per FLASH_write dataLen unit (2 x uint32_t). */
#define FLASH_WRITE_UNIT (2U * sizeof(uint32_t))

#ifdef UNIT_TESTS
#define PRIVATE
#else
#define PRIVATE static
#endif

/* ISR-shared variables – all volatile */
PRIVATE volatile uint32_t _100ms_Tick;
PRIVATE volatile cmd_t cmd;
PRIVATE volatile comm_state_t com_state;
PRIVATE volatile uint8_t page_buf[FLASH_PAGE_SIZE] __attribute__((aligned(4)));
PRIVATE volatile uint8_t *buf_ptr;
PRIVATE volatile header_t header;
PRIVATE volatile uint32_t page_tail;

/* Main-loop-only variables */
static const uint8_t KEY[] = ENCR_KEY; /* never exposed — contains the AES key */
PRIVATE uint32_t timeout;
PRIVATE uint8_t blink_event;
PRIVATE uint32_t page_pos;
PRIVATE uint32_t page_rem;
PRIVATE uint32_t app_begin_word[2];
PRIVATE uint32_t crc;
PRIVATE uint8_t key_timer; /* debounce counter; promoted from bl_key_check() local static */

_Static_assert(sizeof(header_t) <= sizeof(page_buf),
               "header_t does not fit in page buffer");

PRIVATE void update_SysTick_tim(void);
PRIVATE void handle_new_cmd(uint8_t data);
PRIVATE void byte_rx_cb(uint8_t data);
PRIVATE bool write_page_to_flash(const uint32_t *data, uint32_t write_pos, size_t write_count);

PRIVATE void init_hardware(void);
PRIVATE void deinit_hardware(void);
PRIVATE void bl_key_check(void);
PRIVATE void update_bl_status_led(void);
PRIVATE void check_bl_exit_condition(void);
PRIVATE void update_com_fsm(void);
PRIVATE void do_reset(bool ok);
PRIVATE void do_get_version(void);
PRIVATE void do_start(void);
PRIVATE void do_next_page(void);

/**
 * @brief  Bootloader entry point.
 *
 *         Initialises all hardware, then spins in the main bootloader loop.
 *         Returns @c int to satisfy the hw/main.c call convention; under
 *         normal operation the function never returns.
 *
 * @return Never returns.
 */
int main_app(void)
{
    com_state = COM_START;
    timeout = START_TIMEOUT;
    init_hardware();

    if (bl_key_pressed())
    {
        _100ms_Tick = 0;
        timeout = PUSH_BUTTON_DET_TIMEOUT;
    }
    while (1)
    {
        bl_key_check();
        update_bl_status_led();
        check_bl_exit_condition();
        update_com_fsm();
    }
}

// ================================================================= IRQ callbacks =================================================================

/**
 * @brief  SysTick callback – increments the 100 ms tick counter.
 *
 *         Registered via register_sysTick_cb() and invoked from the
 *         platform timer ISR every 100 ms.
 */
PRIVATE void update_SysTick_tim(void)
{
    _100ms_Tick++;
}

/**
 * @brief  Dispatch a newly received command byte.
 *
 *         Records the command, then for CMD_START and CMD_NEXT_PAGE arms
 *         the receive buffer and transitions to COM_NEXT_BYTE so the
 *         following bytes are stored.  All other commands transition
 *         directly to COM_DONE.
 *
 * @param  data  Command byte received from the host.
 */
PRIVATE void handle_new_cmd(uint8_t data)
{
    cmd = (cmd_t)data;
    switch (cmd)
    {
    case CMD_START:
        buf_ptr = (volatile uint8_t *)&header;
        page_tail = sizeof(header);
        com_state = COM_NEXT_BYTE;
        break;
    case CMD_NEXT_PAGE:
        buf_ptr = page_buf;
        page_tail = sizeof(page_buf);
        com_state = COM_NEXT_BYTE;
        break;
    default:
        com_state = COM_DONE;
    }
}

/**
 * @brief  UART RX callback – feeds received bytes into the protocol FSM.
 *
 *         Called from the platform USART RX ISR for every received byte.
 *         In COM_START state only CMD_GET_VERSION is accepted; all other
 *         bytes are silently discarded so the stay-alive timeout keeps
 *         counting down.
 *
 * @param  data  Received byte.
 */
PRIVATE void byte_rx_cb(uint8_t data)
{
    switch (com_state)
    {
    case COM_START:
        if (data != (uint8_t)CMD_GET_VERSION)
            break;
        __attribute__((fallthrough));
    case COM_READY:
        handle_new_cmd(data);
        break;
    case COM_NEXT_BYTE:
        *(buf_ptr++) = data;
        if (--page_tail == 0)
            com_state = COM_DONE;
        break;
    case COM_DONE:
        break;
    }
}

// ===================================================== Internal Functions Definition =====================================================

/**
 * @brief  Initialise all bootloader peripherals.
 *
 *         Order: GPIO first, then UART (which enables the RX interrupt),
 *         then the system tick so the timeout counter starts only after
 *         the UART is ready to receive.
 */
PRIVATE void init_hardware(void)
{
    init_bl_key();
    init_bl_status_led();
    register_data_rx_cb(byte_rx_cb);
    init_bl_usart();
    register_sysTick_cb(update_SysTick_tim);
    init_sys_tick();
    CRC_hw_init();
}

/**
 * @brief  Deinitialise all peripherals before handing off to the application.
 *
 * @note   Clock gating must be disabled after the peripheral itself is
 *         disabled; that ordering is preserved here.
 */
PRIVATE void deinit_hardware(void)
{
    deinit_bl_usart_periph();
    deinit_sys_tick();
    deinit_bl_status_led_port();
    deinit_bl_key();
    /* clocks must be gated after peripherals are disabled */
    deinit_bl_usart_clk();
    deinit_bl_status_led_clk();
    deinit_bl_key_clk();
}

/**
 * @brief  Poll the push-button and extend the stay-alive timeout on a
 *         confirmed press-release cycle.
 *
 *         A static counter is pre-loaded so that it wraps to UINT8_MAX
 *         after exactly F_KEY_DELAY main-loop iterations following button
 *         release, providing software debounce.
 */
PRIVATE void bl_key_check(void)
{
    bool pressed = bl_key_pressed();
    if (pressed && !key_timer)
    {
        key_timer = UINT8_MAX - F_KEY_DELAY;
    }
    else if (!pressed && key_timer)
    {
        key_timer++;
    }
    if (!pressed && (key_timer == UINT8_MAX))
    {
        _100ms_Tick = 0;
        timeout = PUSH_BUTTON_TIMEOUT;
    }
}

/**
 * @brief  Toggle the status LED every 500 ms (5 x 100 ms ticks).
 */
PRIVATE void update_bl_status_led(void)
{
    if (((_100ms_Tick % 5) == 0))
    {
        if (!blink_event)
        {
            toggle_bl_led();
            blink_event = 1;
        }
    }
    else
    {
        blink_event = 0;
    }
}

/**
 * @brief  Evaluate the bootloader exit condition on every main-loop pass.
 *
 *         When the stay-alive timeout expires:
 *         - jumps to the application if a valid image is present and the
 *           protocol FSM is not currently mid-receive (COM_NEXT_BYTE);
 *         - otherwise performs a system reset.
 */
PRIVATE void check_bl_exit_condition(void)
{
    if (_100ms_Tick >= timeout)
    {
        if ((com_state != COM_NEXT_BYTE) && CORE_is_valid_app())
        {
            deinit_hardware();
            jump_to_app();
        }
        else
        {
            do_reset(false);
        }
    }
}

/**
 * @brief  Process a fully received command frame.
 *
 *         Dispatches to the appropriate handler when the protocol FSM
 *         reaches COM_DONE, then transitions to COM_READY and resets the
 *         communication timeout.
 */
PRIVATE void update_com_fsm(void)
{
    if (com_state == COM_DONE)
    {
        switch (cmd)
        {
        case CMD_GET_VERSION:
            do_get_version();
            break;
        case CMD_START:
            do_start();
            break;
        case CMD_NEXT_PAGE:
            do_next_page();
            break;
        case CMD_RESET:
            do_reset(true);
            break;
        default:
            send_byte((uint8_t)CMD_ERR);
        }
        com_state = COM_READY;
        timeout = _100ms_Tick + COMM_TIMEOUT;
    }
}

/**
 * @brief  Send a reset acknowledgement and trigger a system reset.
 *
 *         Serves both as the CMD_RESET command handler and as the error-exit
 *         path (no valid application, partial-transfer timeout).
 */
PRIVATE void do_reset(bool ok)
{
    send_byte(((ok ? (uint8_t)CMD_OK : (uint8_t)CMD_ERR) | (uint8_t)CMD_RESET));
    while (!byte_transmission_complete())
    {
    }
    system_reset();
}

/**
 * @brief  Handle CMD_GET_VERSION – transmit protocol version, device ID
 *         and flash page size to the host.
 */
PRIVATE void do_get_version(void)
{
    uint32_t ver = PROTOCOL_VERSION;
    uint64_t dev_id = DEVICE_ID;
    uint32_t fp_size = FLASH_PAGE_SIZE;

    send_byte((uint8_t)CMD_OK | (uint8_t)CMD_GET_VERSION);
    send_byte_table((const uint8_t *)&ver, sizeof(ver));
    send_byte_table((const uint8_t *)&dev_id, sizeof(dev_id));
    send_byte_table((const uint8_t *)&fp_size, sizeof(fp_size));
}

/**
 * @brief  Handle CMD_START – validate the firmware header, erase the
 *         application flash region and prepare AES / CRC state.
 *
 *         Responds with CMD_ERR | CMD_START if the protocol version,
 *         device ID or flash page size do not match the build configuration.
 *         Responds with CMD_OK | CMD_START on success.
 */
PRIVATE void do_start(void)
{
    uint64_t product_id = ((uint64_t)header.product_ID_MSB << 32) |
                          (uint64_t)header.product_ID_LSB;

    if (header.protocol_version != PROTOCOL_VERSION ||
        product_id != DEVICE_ID ||
        header.flash_page_size != FLASH_PAGE_SIZE ||
        header.page_count == 0 ||
        header.page_count > (APP_LAST_PAGE - APP_START_PAGE + 1U))
    {
        send_byte((uint8_t)CMD_ERR | (uint8_t)CMD_START);
        return;
    }

    FLASH_unlock();
    for (uint32_t a = APP_START_PAGE; a <= APP_LAST_PAGE; a++)
    {
        FLASH_erasePage(a);
    }
    FLASH_lock();

    page_pos = APP_START;
    page_rem = header.page_count;

    uint8_t iv[IV_BLOCK_SIZE];
    for (size_t i = 0; i < IV_BLOCK_SIZE; i++)
        iv[i] = header.iv[i];
    AES_CBC_init(KEY, iv);
    crc = CRC_init();

    send_byte((uint8_t)CMD_OK | (uint8_t)CMD_START);
}

/* Writes data to flash and advances page_pos/page_rem; finalises crc and
 * writes the deferred first word when page_rem reaches 0. */
PRIVATE bool write_page_to_flash(const uint32_t *data, uint32_t write_pos, size_t write_count)
{
    bool ok = true;
    FLASH_unlock();
    FLASH_write(write_pos, data, write_count);
    page_pos += FLASH_PAGE_SIZE;
    if (--page_rem == 0)
    {
        crc = CRC_result(crc);
        if (crc != header.crc)
            ok = false;
        else
            FLASH_write(APP_START, &app_begin_word[0], 1);
    }
    FLASH_lock();
    return ok;
}

/**
 * @brief  Handle CMD_NEXT_PAGE – decrypt and write one firmware page.
 *
 *         The first eight bytes of the first page (reset vector / initial
 *         stack pointer) are buffered and written last, after the CRC of the
 *         complete image passes, to prevent a partially-written image from
 *         passing the application-validity check.
 *
 *         Responds with CMD_ERR | CMD_NEXT_PAGE if the write position is
 *         out of range or the final CRC does not match; CMD_OK otherwise.
 */
PRIVATE void do_next_page(void)
{
    if (page_pos < APP_START || page_pos >= APP_END || !page_rem)
    {
        send_byte((uint8_t)CMD_ERR | (uint8_t)CMD_NEXT_PAGE);
        return;
    }

    AES_CBC_decrypt_buffer((uint8_t *)page_buf, sizeof(page_buf));
    crc = CRC_add_byte_tab(crc, (uint8_t *)page_buf, sizeof(page_buf));

    const uint32_t *data = (const uint32_t *)page_buf;
    uint32_t write_pos = page_pos;
    size_t write_count = sizeof(page_buf) / FLASH_WRITE_UNIT;

    if (page_pos == APP_START)
    {
        app_begin_word[0] = *data++;
        app_begin_word[1] = *data++;
        write_pos += 2U * sizeof(uint32_t);
        --write_count;
    }

    bool res = write_page_to_flash(data, write_pos, write_count);
    if (res)
        send_byte((uint8_t)CMD_OK | (uint8_t)CMD_NEXT_PAGE);
    else
        send_byte((uint8_t)CMD_ERR | (uint8_t)CMD_NEXT_PAGE);
}
