/**
 * @file bl_flash_drv.h
 * @author niwciu (niwciu@gmail.com)
 * @brief
 * @version 1.0.0
 * @date 2026-05-07
 *
 * @copyright Copyright (c) 2026
 *
 */
#ifndef _BL_FLASH_DRV_H_
#define _BL_FLASH_DRV_H_

#include <stdint.h>
#include <stddef.h>

void FLASH_waitBusy(void);
void FLASH_unlock(void);
void FLASH_lock(void);
void FLASH_erasePage(uint32_t page_no);
void FLASH_write(uint32_t addr, uint32_t const *data, size_t dataLen);

#endif /* _BL_FLASH_DRV_H_ */
