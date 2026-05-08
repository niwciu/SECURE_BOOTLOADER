/**
 * @file main_app.c
 * @author niwciu (niwciu@gmail.com)
 * @brief
 * @version 1.0.0
 * @date 2025-11-03
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "main_app.h"
#include "bl_usart_drv.h"
#include "core_drv.h"
#include "bl_flash_drv.h"
#include "bl_hw_config.h"
#include "bl_key_drv.h"
#include "bl_status_led_drv.h"
#include "aes.h"
#include "crc_api.h"
#include <assert.h>
#include <stddef.h>

typedef enum
{
    CMD_GET_VERSION = 0x01,
    CMD_START = 0x02,
    CMD_NEXT_PAGE = 0x03,
    CMD_RESET = 0x04,
    CMD_OK = 0x40,
    CMD_ERR = 0x80,
} cmd_t;

typedef enum
{
    COM_START,
    COM_READY,
    COM_NEXT_BYTE,
    COM_DONE
} comm_state_t;

typedef struct
{
    uint32_t protocol_version;
    uint32_t product_ID_MSB;
    uint32_t product_ID_LSB;
    uint32_t app_version;
    uint32_t page_count;
    uint32_t flash_page_size;
    uint8_t iv[IV_BLOCK_SIZE];
    uint32_t crc;
} header_t;


static volatile uint32_t _100ms_Tick;


static volatile cmd_t cmd;
static volatile comm_state_t com_state;
static volatile uint8_t page_buf[FLASH_PAGE_SIZE];
static volatile uint8_t *buf_ptr;
static volatile header_t header;
static volatile uint32_t page_tail;

static const uint8_t KEY[] = ENCR_KEY;
static uint32_t timeout;
static uint8_t blink_event = 0;
static uint32_t page_pos;
static uint32_t page_rem;
static uint32_t app_begin_word[2];

static uint32_t crc;

// static_assert(sizeof(header_t) <= sizeof(page_buf), "Data too big");

static void update_SysTick_tim(void);
static void byte_rx_cb(uint8_t data);

static void init_hardware(void);
static void deinit_hardware(void);
static void bl_key_check(void);
static void update_bl_status_LED(void);
static void check_bl_exit_codition(void);
static void update_com_fsm(void);
static void do_reset(void);
static void do_get_version(void);
static void do_start(void);
static void do_next_page(void);



int main_app(void)
{
    com_state = COM_START;
    timeout = START_TIMEOUT;
    init_hardware();

    
    if(bl_key_pressed())
    {
        _100ms_Tick = 0;
        timeout = PUSH_BUTTON_DET_TIMEOUT;
    }
    while (1)
    {
        bl_key_check();
        update_bl_status_LED();
        check_bl_exit_codition();
        update_com_fsm();

    }
return 0;
}

// ================================================================= IRQ callbacks= =================================================================



static void update_SysTick_tim(void)
{
    _100ms_Tick++;
}

static void byte_rx_cb(uint8_t data)
{
    switch (com_state)
    {
    case COM_START:
        if (data == (uint8_t)CMD_GET_VERSION)
        {
            // falltrough
        }
        else
        {
            break;
        }

    case COM_READY:
        cmd = (cmd_t)data;
        switch (cmd)
        {
        case CMD_START:
            buf_ptr = (uint8_t *)&header;
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


// ===================================================== Internall Functions Definition =====================================================

static void init_hardware(void)
{
    init_bl_key();
    init_bl_status_led();
    register_data_rx_cb(byte_rx_cb);
    init_bl_usart();
    register_sysTick_cb(update_SysTick_tim);
    init_sys_tick();
    CRC_hw_init();
}

static void deinit_hardware(void)
{

    deinit_bl_usart_periph();
    deinit_sys_tick();
    deinit_bl_status_led_port();
    deinit_bl_key();
    // clk need to be deinit after all periph deinit
    deinit_bl_usart_clk();
    deinit_bl_status_led_clk();
    deinit_bl_key_clk();
}

static void bl_key_check(void)
{
    static uint8_t key_timer;

    if ((bl_key_pressed()) && !key_timer)
    {
        key_timer = 255 - F_KEY_DELAY; // ustawiam opoznienie o wartosci ilosci powtorzen glownej petli
    }
    else if ((!bl_key_pressed()) && key_timer)
    {
        key_timer++;
    }
    if ((!bl_key_pressed()) && (key_timer == 255))
    {
        _100ms_Tick = 0;
        timeout = PUSH_BUTTON_TIMEOUT;
    }
}

static void update_bl_status_LED(void)
{
    if (((_100ms_Tick % 5) == 0))
    {
        if (!blink_event)
        {
            toogle_bl_led();
            blink_event = 1;
        }
    }
    else
    {
        blink_event = 0;
    }
}
static void check_bl_exit_codition(void)
{
    if ((_100ms_Tick >= timeout))
    {
        if ((com_state != COM_NEXT_BYTE) && CORE_is_valid_app())
        {
            deinit_hardware();
            jump_to_app();
        }
        else
        {
            do_reset();
        }
    }
}
static void update_com_fsm(void)
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
                break; //
            case CMD_RESET:
                do_reset();
                break;
            default:
                send_byte((uint8_t)CMD_ERR);
            }
            com_state = COM_READY;
            timeout = _100ms_Tick + COMM_TIMEOUT;
        }
}

static void do_reset(void)
{
    send_byte((uint8_t)CMD_OK | (uint8_t)CMD_RESET);
    while (!byte_transmission_complete())
    {

    }
    system_reset();
}

static void do_get_version(void)
{
    uint32_t ver = PROTOCOL_VERSION;
    uint64_t dev_id = DEVICE_ID;
    uint32_t fp_size = FLASH_PAGE_SIZE;

    send_byte((uint8_t)CMD_OK | (uint8_t)CMD_GET_VERSION);
    send_byte_table((const uint8_t *)&ver, sizeof(ver));
    send_byte_table((const uint8_t *)&dev_id, sizeof(dev_id));
    send_byte_table((const uint8_t *)&fp_size, sizeof(fp_size));
}

static void do_start(void)
{
    uint64_t product_id = (((uint64_t)(header.product_ID_MSB) << 32)) | ((uint64_t)(header.product_ID_LSB));

    if (header.protocol_version != PROTOCOL_VERSION || product_id != DEVICE_ID)
    {
        send_byte((uint8_t)CMD_ERR | (uint8_t)CMD_START);
        return;
    }

    FLASH_unlock();

    // wyliczanie adresu jest w oparciu o info z RM0454
    for (uint32_t a = APP_START_PAGE; a <= APP_LAST_PAGE; a++)
    {
        FLASH_erasePage(a);
    }
    FLASH_lock();
    page_pos = APP_START;
    page_rem = header.page_count;

    AES_CBC_init(KEY, (const uint8_t *)header.iv);
    crc = CRC_init();

    send_byte((uint8_t)CMD_OK | (uint8_t)CMD_START);
}

static void do_next_page(void)
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
    size_t write_len = sizeof(page_buf) / 8; // 2words =8byten in one write sequence;

    if (page_pos == APP_START)
    { // skip first 8 bytes (2 Words) of the app ->
        app_begin_word[0] = *data;
        ++data;
        app_begin_word[1] = *data;
        ++data;
        write_pos += sizeof(uint32_t);
        write_pos += sizeof(uint32_t);
        --write_len;
    }

    bool res = true;
    FLASH_unlock();

    FLASH_write(write_pos, data, write_len);
    page_pos += FLASH_PAGE_SIZE;
    if (--page_rem == 0)
    { // end of transfer, validate and write the skipped 8 bytes
        crc = CRC_result(crc);
        if (crc != header.crc)
        {
            res = false;
        }
        else
        {
            FLASH_write(APP_START, &app_begin_word[0], 1);
        }
    }
    FLASH_lock();

    if (res)
        send_byte((uint8_t)CMD_OK | (uint8_t)CMD_NEXT_PAGE);
    else
        send_byte((uint8_t)CMD_ERR | (uint8_t)CMD_NEXT_PAGE);
}




















