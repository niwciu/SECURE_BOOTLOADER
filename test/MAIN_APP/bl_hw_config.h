/**
 * @file bl_hw_config.h
 * @brief Test-specific hardware configuration for MAIN_APP unit tests.
 *
 *        Shadows the real hw/<TARGET>/Core/Inc/bl_hw_config.h so that
 *        main_app.c can be compiled without any target-specific BSP.
 *        Values are chosen to keep test data small (FLASH_PAGE_SIZE=64).
 */

#ifndef HW_BL_CONFIG_H_
#define HW_BL_CONFIG_H_

#include <stdint.h>

/* ---- Bootloader timing (ticks = 100 ms) --------------------------------- */
#define F_KEY_DELAY 5U
#define START_TIMEOUT 5U
#define COMM_TIMEOUT 10U
#define PUSH_BUTTON_TIMEOUT 300U
#define PUSH_BUTTON_DET_TIMEOUT 60U

/* ---- Protocol ----------------------------------------------------------- */
#define PROTOCOL_VERSION 0x00000001UL

/* ---- Peripheral settings ------------------------------------------------ */
#define CPU_F 16000000U
#define BAUD 115200U
#define IV_BLOCK_SIZE 16U

/* ---- Flash map ---------------------------------------------------------- */
#define FLASH_START_ADR 0x08000000UL
#define APP_START 0x08001000UL /* 4 kB bootloader area     */
#define APP_END 0x08002000UL   /* 4 kB app area (test)     */
#define FLASH_PAGE_SIZE 64U    /* small page for test data */

/* ---- SRAM map ----------------------------------------------------------- */
#define SRAM_START 0x20000000UL
#define SRAM_END (SRAM_START + (36U * 1024U))

/* ---- Derived (DO NOT EDIT) ---------------------------------------------- */
#define APP_START_PAGE ((APP_START - FLASH_START_ADR) / FLASH_PAGE_SIZE)
#define APP_LAST_PAGE ((APP_END - FLASH_START_ADR) / FLASH_PAGE_SIZE - 1U)

#endif /* HW_BL_CONFIG_H_ */
