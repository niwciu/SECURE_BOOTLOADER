/**
 * @file bl_crc_drv.h
 * @author niwciu (niwciu@gmail.com)
 * @brief
 *      CRC-32/IEEE 802.3:
 *          poly   = 0x04C11DB7
 *          init   = 0xFFFFFFFF
 *          refin  = true
 *          refout = true
 *          xorout = 0xFFFFFFFF
 * @version 1.0.0
 * @date 2026-05-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef BL_CRC_DRV_H_
#define BL_CRC_DRV_H_

#include <stdint.h>
#include <stddef.h>

void CRC_hw_init(void);
uint32_t CRC_init(void);
uint32_t CRC_add_byte(uint32_t crc, uint8_t b);
uint32_t CRC_add_byte_tab(uint32_t crc, uint8_t const *data, size_t dataLen);
uint32_t CRC_result(uint32_t crc);

#endif /* BL_CRC_DRV_H_ */