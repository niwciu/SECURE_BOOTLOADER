/**
 * @file bl_hw_config.h
 * @author niwciu (niwciu@gmail.com)
 * @brief
 * @version 1.0.0
 * @date 2026-05-07
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef HW_BL_CONFIG_H_
#define HW_BL_CONFIG_H_

#include <stdint.h>

// ============================ BootLoader Time Slots & Settings =====================================
#define F_KEY_DELAY             100     // delay in number of main loops
#define START_TIMEOUT           5       // in ticks, 1 tick is 100 ms
#define COMM_TIMEOUT            10      // in ticks
#define PUSH_BUTTON_TIMEOUT     3000    // in ticks, 1 tick is 100 ms 10= 1sek
#define PUSH_BUTTON_DET_TIMEOUT 600     // in ticks, 1 tick is 100 ms 10= 1sek

#define PROTOCOL_VERSION        0x00000001

// ================================ Periph Settings =================================================
#define CPU_F           16000000UL
#define BAUD            115200UL
#define IV_BLOCK_SIZE   16

// ============================== MCU Flash Settings based on datasheet ==============================
#define FLASH_START_ADR     0x0000
#define APP_START           0x0000      // Application starts at beginning of flash
#define APP_END             0x7000      // Bootloader occupies 0x7000-0x7FFF (4KB)
#define FLASH_PAGE_SIZE     128         // SPM_PAGESIZE = 128 bytes on ATmega328P

// ============================== MCU SRAM Settings =================================================
#define SRAM_START  (0x0100U)           // ATmega328P: SRAM starts at 0x0100
#define SRAM_END    (0x08FFU)           // ATmega328P: 2KB SRAM => 0x0100 + 0x800 - 1

// ====================== automatic calculation based on settings ==================================
/* DO NOT EDIT */
#define APP_START_PAGE  (APP_START / FLASH_PAGE_SIZE)               // = 0
#define APP_LAST_PAGE   ((APP_END  / FLASH_PAGE_SIZE) - 1)          // = 223


#endif /* HW_BL_CONFIG_H_ */
