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

#include "stm32g474xx.h"
#include <stddef.h>

#define CRC_INIT_VAL   0xFFFFFFFFUL
#define CRC_XOR_OUT    0xFFFFFFFFUL

void CRC_hw_init(void)
{
    /* enable CRC clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
}

uint32_t CRC_init(void)
{
    CRC->INIT = CRC_INIT_VAL;

    CRC->CR = CRC_CR_REV_IN_0 | CRC_CR_REV_OUT;

    CRC->CR |= CRC_CR_RESET;

    return CRC->DR;
}

uint32_t CRC_add_byte(uint32_t crc, uint8_t b)
{
    (void)crc;

    *((volatile uint8_t *)&CRC->DR) = b;

    return CRC->DR;
}

uint32_t CRC_add_byte_tab(uint32_t crc, uint8_t const *data, size_t dataLen)
{
    (void)crc;

    for (; dataLen > 0; ++data, --dataLen)
    {
        *((volatile uint8_t *)&CRC->DR) = *data;
    }

    return CRC->DR;
}

uint32_t CRC_result(uint32_t crc)
{
    (void)crc;

    return (CRC->DR ^ 0xFFFFFFFF);
}