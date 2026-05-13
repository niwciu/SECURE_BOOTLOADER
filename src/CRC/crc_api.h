/**
 * @file crc_api.h
 * @author niwciu (niwciu@gmail.com)
 * @brief CRC-32/IEEE 802.3 API.
 *
 *        Software implementation is in crc.c.  STM32 targets may substitute
 *        a hardware-accelerated implementation via bl_crc_drv.c while keeping
 *        this same interface.
 *
 *        CRC parameters:
 *          - Polynomial : 0x04C11DB7
 *          - Init       : 0xFFFFFFFF
 *          - RefIn      : true
 *          - RefOut     : true
 *          - XorOut     : 0xFFFFFFFF
 *
 * @version 1.0.0
 * @date 2026-05-07
 *
 * @copyright Copyright (c) 2026
 *
 */
#ifndef CRC_API_H_
#define CRC_API_H_

#include <stdint.h>
#include <stddef.h>

/**
 * @brief  Initialise or reset the hardware CRC peripheral (no-op on SW targets).
 */
void CRC_hw_init(void);

/**
 * @brief  Return the initial CRC accumulator value (0xFFFFFFFF).
 *
 * @return Initial accumulator.
 */
uint32_t CRC_init(void);

/**
 * @brief  Accumulate one byte into a running CRC.
 *
 * @param  crc   Running accumulator.
 * @param  b     Byte to process.
 * @return Updated accumulator.
 */
uint32_t CRC_add_byte(uint32_t crc, uint8_t b);

/**
 * @brief  Accumulate a buffer into a running CRC.
 *
 * @param  crc      Running accumulator.
 * @param  data     Pointer to data.
 * @param  dataLen  Number of bytes.
 * @return Updated accumulator.
 */
uint32_t CRC_add_byte_tab(uint32_t crc, uint8_t const *data, size_t dataLen);

/**
 * @brief  Finalise the CRC: apply output reflection and XorOut.
 *
 * @param  crc  Last accumulator value from CRC_add_byte / CRC_add_byte_tab.
 * @return Final CRC-32/IEEE 802.3 checksum.
 */
uint32_t CRC_result(uint32_t crc);

#endif /* CRC_API_H_ */
