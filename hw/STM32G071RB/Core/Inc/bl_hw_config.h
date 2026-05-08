/**
 * @file bl_hw_config.h
 * @author niwciu (niwciu@gmail.com)
 * @brief
 * @version 1.0.0
 * @date 2026-05-05
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef HW_BL_CONFIG_H_
#define HW_BL_CONFIG_H_

#include <stdint.h>

// ============================ BoatLoader Time Slots & Settings =====================================
#define F_KEY_DELAY 			100 	// delay in number of main loops
#define START_TIMEOUT 			5   	// in ticks, 1 tick is 100 ms
#define COMM_TIMEOUT 			10  	// in ticks
#define PUSH_BUTTON_TIMEOUT 	3000	// in ticks, 1 tick is 100 ms 10= 1sek
#define PUSH_BUTTON_DET_TIMEOUT 600 	// in ticks, 1 tick is 100 ms 10= 1sek

#define PROTOCOL_VERSION 		0x00000001

// ================================ Periph Settings =================================================
#define CPU_F 			16000000
#define BAUD 			115200
#define IV_BLOCK_SIZE 	16

// ============================== MCU Flash Settinfs base on RM =====================================
#define FLASH_START_ADR		0x08000000
#define APP_START 			0x08001000 	// 4k for bootloader
#define APP_END 			0x08020000  // 128K // 0x08040000 //256K
// #define VTOR_SIZE 			0xC0	// irq vecot table size (needed when copying the vector to SRAM - MCU without SBC->VTOR)
#define FLASH_PAGE_SIZE 	2048 		
#define FLASH_KEY1 			0x45670123
#define FLASH_KEY2 			0xCDEF89AB

// ============================== MCU SRAM Settinfs ================================================
#define SRAM_START 	(0x20000000U)
#define SRAM_END 	(SRAM_START + (36 * 1024))

// ====================== automatic calculation based on settings ==================================
/* DO NOT EDIT*/
#define APP_START_PAGE 	((APP_START - FLASH_START_ADR) / FLASH_PAGE_SIZE)
#define APP_LAST_PAGE 	((APP_END - FLASH_START_ADR) / FLASH_PAGE_SIZE) - 1



#endif /* HW_BL_CONFIG_H_ */
