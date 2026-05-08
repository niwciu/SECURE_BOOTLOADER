/**
 * @file crc.c
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
#include "crc_api.h"

/* ===================== CRC CONFIG ===================== */

#define CRC_WIDTH              (32u)

#define CRC_INIT_VALUE         (0xFFFFFFFFUL)
#define CRC_XOR_OUT_VALUE      (0xFFFFFFFFUL)

#define CRC_POLYNOMIAL        (0x04C11DB7UL)

/* bit masks */
#define CRC_MSB_MASK          (0x80000000UL)
#define CRC_LSB_MASK          (0x00000001UL)


void CRC_hw_init(void)
{

}

uint32_t rev_u32(uint32_t value)
{
    uint32_t result = 0;

    for (size_t i = 0; i < CRC_WIDTH; ++i)
    {
        result <<= 1;
        result |= (value & CRC_LSB_MASK);
        value >>= 1;
    }

    return result;
}

/* ===================== API ===================== */

uint32_t CRC_init(void)
{
    return CRC_INIT_VALUE;
}

uint32_t CRC_add_byte(uint32_t crc, uint8_t byte)
{
    for (uint8_t bit = 0x01u; bit != 0; bit <<= 1)
    {
        uint32_t msb = crc & CRC_MSB_MASK;

        if (byte & bit)
        {
            msb ^= CRC_MSB_MASK;
        }

        crc <<= 1;

        if (msb)
        {
            crc ^= CRC_POLYNOMIAL;
        }
    }

    return crc;
}

uint32_t CRC_add_byte_tab(uint32_t crc, uint8_t const *data, size_t length)
{
    while (length--)
    {
        crc = CRC_add_byte(crc, *data++);
    }

    return crc;
}

uint32_t CRC_result(uint32_t crc)
{
    return (rev_u32(crc) ^ CRC_XOR_OUT_VALUE);
}