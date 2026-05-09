/**
 * @file bl_hw_config.h
 * @brief Hardware configuration template for a new STM32 bootloader target.
 *
 *        TODO: fill every value marked with TODO: for your specific MCU.
 *        Consult the MCU Reference Manual and datasheet.
 */

#ifndef HW_BL_CONFIG_H_
#define HW_BL_CONFIG_H_

#include <stdint.h>

/* ---- Bootloader timing (1 tick = 100 ms) --------------------------------- */
#define F_KEY_DELAY              100    /* debounce: main-loop iterations      */
#define START_TIMEOUT            5      /* ticks before jumping / resetting    */
#define COMM_TIMEOUT             10     /* ticks of inactivity during transfer */
#define PUSH_BUTTON_TIMEOUT      3000   /* ticks after button press detected   */
#define PUSH_BUTTON_DET_TIMEOUT  600    /* ticks to detect button hold         */

/* ---- Protocol ------------------------------------------------------------ */
#define PROTOCOL_VERSION         0x00000001UL

/* ---- Peripheral settings ------------------------------------------------- */
/* TODO: set CPU frequency in Hz */
#define CPU_F                    16000000U
#define BAUD                     115200U
#define IV_BLOCK_SIZE            16U    /* AES-CBC IV size – do not change     */

/* ---- Flash map ----------------------------------------------------------- */
/* TODO: set the correct values from the MCU memory map (RM / datasheet)      */
#define FLASH_START_ADR          0x08000000UL   /* flash base address          */
#define APP_START                0x08001000UL   /* bootloader size = 4 KB      */
#define APP_END                  0x08020000UL   /* TODO: end of application    */
#define FLASH_PAGE_SIZE          2048U          /* TODO: page/sector size      */
#define FLASH_KEY1 			     0x45670123UL   /* TODO: key taken from RM     */
#define FLASH_KEY2 			     0xCDEF89ABUL   /* TODO: key taken from RM     */

/* ---- SRAM map ------------------------------------------------------------ */
/* TODO: set correct SRAM base and size for your MCU                          */
#define SRAM_START               0x20000000UL
#define SRAM_END                 (SRAM_START + (36U * 1024U))  /* TODO: adjust */

/* ---- Derived (DO NOT EDIT) ----------------------------------------------- */
#define APP_START_PAGE           ((APP_START - FLASH_START_ADR) / FLASH_PAGE_SIZE)
#define APP_LAST_PAGE            ((APP_END   - FLASH_START_ADR) / FLASH_PAGE_SIZE - 1U)

#endif /* HW_BL_CONFIG_H_ */
