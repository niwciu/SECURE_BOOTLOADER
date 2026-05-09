/**
 * @file bl_hw_config.h
 * @author niwciu (niwciu@gmail.com)
 * @brief Hardware configuration for STM32G070RB bootloader target.
 * @version 1.0.0
 * @date 2026-05-05
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef HW_BL_CONFIG_H_
#define HW_BL_CONFIG_H_

#include <stdint.h>

/* ---- Bootloader timing (1 tick = 100 ms) --------------------------------- */
#define F_KEY_DELAY              100U   /* debounce: main-loop iterations      */
#define START_TIMEOUT            5U     /* ticks before jumping / resetting    */
#define COMM_TIMEOUT             10U    /* ticks of inactivity during transfer */
#define PUSH_BUTTON_TIMEOUT      3000U  /* ticks after button press detected   */
#define PUSH_BUTTON_DET_TIMEOUT  600U   /* ticks to detect button hold         */

/* ---- Protocol ------------------------------------------------------------ */
#define PROTOCOL_VERSION         0x00000001UL

/* ---- Peripheral settings ------------------------------------------------- */
#define CPU_F                    16000000U
#define BAUD                     115200U
#define IV_BLOCK_SIZE            16U    /* AES-CBC IV size – do not change     */

/* ---- Flash map ----------------------------------------------------------- */
#define FLASH_START_ADR          0x08000000UL
#define APP_START                0x08001000UL   /* bootloader size = 4 KB      */
#define APP_END                  0x08020000UL   /* 128 KB application region   */
#define FLASH_PAGE_SIZE          2048U
#define FLASH_KEY1 			     0x45670123UL
#define FLASH_KEY2 			     0xCDEF89ABUL

/* ---- SRAM map ------------------------------------------------------------ */
#define SRAM_START               0x20000000UL
#define SRAM_END                 (SRAM_START + (36U * 1024U))

/* ---- Derived (DO NOT EDIT) ----------------------------------------------- */
#define APP_START_PAGE           ((APP_START - FLASH_START_ADR) / FLASH_PAGE_SIZE)
#define APP_LAST_PAGE            ((APP_END   - FLASH_START_ADR) / FLASH_PAGE_SIZE - 1U)

#endif /* HW_BL_CONFIG_H_ */
