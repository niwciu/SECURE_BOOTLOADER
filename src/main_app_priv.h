/**
 * @file main_app_priv.h
 * @author niwciu (niwciu@gmail.com)
 * @brief Internal protocol types for the secure bootloader core.
 *
 *        Included by main_app.c and by unit-test files that need direct
 *        access to protocol types or module-level variables.
 *        Not part of the public API — do not include from application code.
 *
 * @version 1.0.0
 * @date 2026-05-09
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef MAIN_APP_PRIV_H_
#define MAIN_APP_PRIV_H_

#include <stdint.h>
#include "bl_hw_config.h" /* FLASH_PAGE_SIZE, IV_BLOCK_SIZE */

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

#endif /* MAIN_APP_PRIV_H_ */
