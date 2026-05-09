/**
 * @file crc.c
 * @author niwciu (niwciu@gmail.com)
 * @brief Software CRC-32/IEEE 802.3 implementation.
 *
 *        Parameters:
 *          - Polynomial : 0x04C11DB7
 *          - Init value : 0xFFFFFFFF
 *          - RefIn      : true  (input bytes reflected)
 *          - RefOut     : true  (output word reflected)
 *          - XorOut     : 0xFFFFFFFF
 *
 *        Usage pattern:
 *        @code
 *          uint32_t crc = CRC_init();
 *          crc = CRC_add_byte_tab(crc, data, len);
 *          uint32_t result = CRC_result(crc);
 *        @endcode
 *
 * @version 1.0.0
 * @date 2026-05-07
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "crc_api.h"

/* ===================== CRC CONFIG ===================== */

#define CRC_WIDTH (32u)
#define CRC_INIT_VALUE (0xFFFFFFFFUL)
#define CRC_XOR_OUT (0xFFFFFFFFUL)
#define CRC_POLYNOMIAL (0x04C11DB7UL)
#define CRC_MSB_MASK (0x80000000UL)
#define CRC_LSB_MASK (0x00000001UL)

#ifdef UNIT_TESTS
#define PRIVATE
#else
#define PRIVATE static
#endif

/* ===================== Internal helpers ===================== */

/**
 * @brief  Reflect all 32 bits of @p value (reverse bit order).
 *
 *         Used to convert between the MSB-first polynomial representation
 *         and the reflected (LSB-first) byte processing used by this algorithm.
 *
 * @param  value  32-bit input word.
 * @return Bit-reversed copy of @p value.
 */
PRIVATE uint32_t rev_u32(uint32_t value)
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

/* ===================== Public API ===================== */

/**
 * @brief  No-op for targets without a hardware CRC peripheral.
 *
 *         Provided so that @ref main_app.c can call CRC_hw_init() uniformly
 *         across all platforms.  STM32 targets supply a real implementation
 *         in their bl_crc_drv.c.
 */
void CRC_hw_init(void)
{
}

/**
 * @brief  Return the initial CRC accumulator value.
 *
 * @return CRC_INIT_VALUE (0xFFFFFFFF).
 */
uint32_t CRC_init(void)
{
    return CRC_INIT_VALUE;
}

/**
 * @brief  Accumulate one byte into a running CRC.
 *
 *         Processes the byte LSB-first (reflected input) using the
 *         MSB-first polynomial, producing the same result as a table-driven
 *         implementation.
 *
 * @param  crc   Running CRC accumulator (from CRC_init() or a previous call).
 * @param  byte  Byte to process.
 * @return Updated CRC accumulator.
 */
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

/**
 * @brief  Accumulate a buffer of bytes into a running CRC.
 *
 * @param  crc      Running CRC accumulator.
 * @param  data     Pointer to the data buffer.
 * @param  length   Number of bytes to process.
 * @return Updated CRC accumulator.
 */
uint32_t CRC_add_byte_tab(uint32_t crc, uint8_t const *data, size_t length)
{
    while (length--)
    {
        crc = CRC_add_byte(crc, *data++);
    }

    return crc;
}

/**
 * @brief  Finalise the CRC computation.
 *
 *         Applies output reflection and XorOut to produce the final
 *         CRC-32/IEEE 802.3 checksum.
 *
 * @param  crc  Accumulated CRC value (last value from CRC_add_byte / CRC_add_byte_tab).
 * @return Final CRC-32 checksum.
 */
uint32_t CRC_result(uint32_t crc)
{
    return (rev_u32(crc) ^ CRC_XOR_OUT);
}
