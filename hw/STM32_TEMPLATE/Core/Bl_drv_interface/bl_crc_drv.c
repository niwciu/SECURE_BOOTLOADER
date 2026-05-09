/**
 * @file bl_crc_drv.c
 * @brief HW CRC driver stub – implement for your target MCU.
 *
 *        Required algorithm: CRC-32/IEEE 802.3
 *          poly   = 0x04C11DB7
 *          init   = 0xFFFFFFFF
 *          refin  = true
 *          refout = true
 *          xorout = 0xFFFFFFFF
 *
 *        TODO: configure the CRC peripheral to match the algorithm above.
 *        If the peripheral cannot be configured for refin/refout, implement
 *        rev_u32() in software (bit-reversal of a 32-bit word).
 *
 *        NOTE: this file is only compiled when CRC_MODULE=HW.
 *              For software CRC use src/CRC/crc.c (CRC_MODULE=SW).
 */

#include "crc_api.h"
/* TODO: replace with the correct CMSIS device header */
#include "stm32xxxxxx.h"

#include <stddef.h>
#include <stdint.h>

#define CRC_INIT_VAL  0xFFFFFFFFUL
#define CRC_XOR_OUT   0xFFFFFFFFUL

void CRC_hw_init(void)
{
    /* TODO: enable the CRC peripheral clock via RCC */
    /* Example (STM32G0/F0/L0): RCC->AHBENR |= RCC_AHBENR_CRCEN; */
}

uint32_t CRC_init(void)
{
    /* TODO: configure the CRC peripheral:
     *   1. Load INIT value
     *   2. Set REV_IN and REV_OUT bits (for CRC-32/IEEE 802.3 refin/refout)
     *   3. Trigger RESET to load INIT into the internal CRC register
     *   4. Return current DR (flush pipeline) */
    CRC->INIT = CRC_INIT_VAL;
    CRC->CR   = 0; /* TODO: set REV_IN_0, REV_OUT, RESET bits per your RM */
    return CRC->DR;
}

uint32_t CRC_add_byte(uint32_t crc, uint8_t byte)
{
    /* TODO: feed one byte into the CRC peripheral DR register (byte-wide write) */
    (void)crc;
    *((volatile uint8_t *)&CRC->DR) = byte;
    return CRC->DR;
}

uint32_t CRC_add_byte_tab(uint32_t crc, uint8_t const *data, size_t dataLen)
{
    /* TODO: feed a buffer of bytes into the CRC peripheral */
    (void)crc;
    while (dataLen--)
    {
        *((volatile uint8_t *)&CRC->DR) = *data++;
    }
    return CRC->DR;
}

uint32_t CRC_result(uint32_t crc)
{
    /* TODO: apply final XOR (CRC-32/IEEE 802.3 xorout = 0xFFFFFFFF) */
    (void)crc;
    return (CRC->DR ^ CRC_XOR_OUT);
}
