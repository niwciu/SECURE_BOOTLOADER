/**
 * @file bl_crc_drv.c
 * @author niwciu (niwciu@gmail.com)
 * @brief
 *      CRC-32/IEEE 802.3:
 *          poly   = 0x04C11DB7
 *          init   = 0xFFFFFFFF
 *          refin  = true
 *          refout = true
 *          xorout = 0xFFFFFFFF
 *
 * @version 1.0.0
 * @date 2026-05-07
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "crc_api.h"
#include "stm32g070xx.h"

#include <stddef.h>
#include <stdint.h>

#define CRC_INIT_VAL   0xFFFFFFFFUL
#define CRC_XOR_OUT    0xFFFFFFFFUL

/* ========================================================= */

void CRC_hw_init(void)
{
    /* enable CRC clock */
    RCC->AHBENR |= RCC_AHBENR_CRCEN;
}

void CRC_hw_deinit(void)
{
    /* enable CRC clock */
    RCC->AHBENR &= ~RCC_AHBENR_CRCEN;
}

/* ========================================================= */
/* NOTE:
 * rev_u32 is handled by hardware via CRC_CR_REV_OUT
 */
uint32_t rev_u32(uint32_t d)
{
    (void)d;

    return CRC->DR;
}

/* ========================================================= */

uint32_t CRC_init(void)
{
    /* IMPORTANT:
     * 1. enable INIT value BEFORE reset
     * 2. reset loads INIT into internal CRC register
     */

    CRC->INIT = CRC_INIT_VAL;

    CRC->CR =
        CRC_CR_REV_IN_0 |
        CRC_CR_REV_OUT  |
        CRC_CR_RESET;

    /* optional read to flush pipeline */
    return CRC->DR;
}

/* ========================================================= */

uint32_t CRC_add_byte(uint32_t crc, uint8_t byte)
{
    (void)crc;

    /* feed yteyte-wise into CRC DR */
    *((volatile uint8_t *)&CRC->DR) = byte;

    return CRC->DR;
}

/* ========================================================= */

uint32_t CRC_add_byte_tab(uint32_t crc, uint8_t const *data, size_t dataLen)
{
    (void)crc;

    while (dataLen--)
    {
        *((volatile uint8_t *)&CRC->DR) = *data++;
    }

    return CRC->DR;
}

/* ========================================================= */

uint32_t CRC_result(uint32_t crc)
{
    (void)crc;

    /* final XOR (CRC-32/IEEE 802.3) */
    return (CRC->DR ^ CRC_XOR_OUT);
}