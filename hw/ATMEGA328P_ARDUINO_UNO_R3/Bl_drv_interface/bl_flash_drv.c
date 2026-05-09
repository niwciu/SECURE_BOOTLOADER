/**
 * @file bl_flash_drv.c
 * @author niwciu (niwciu@gmail.com)
 * @brief
 * @version 1.0.0
 * @date 2026-05-07
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "bl_flash_drv.h"
#include "bl_hw_config.h"

#include <avr/io.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include <stdint.h>
#include <stddef.h>
#include <string.h>

/*
 * ATmega328P Flash write notes:
 *
 * SPM_PAGESIZE = 128 bytes (64 words).
 * The application section is 0x0000-0x77FF (240 pages of 128 bytes).
 * The bootloader section size depends on BOOTSZ fuse configuration.
 *
 * FLASH_write performs a read-modify-write so that partial-page updates
 * (e.g. two consecutive calls covering different slices of the same page)
 * are handled correctly. The upper layer (main_app.c) writes the first
 * page in two separate calls: bytes 8-127 first, then bytes 0-7.
 *
 * NOTE:
 * This driver must execute from the bootloader section because AVR SPM
 * instructions are only permitted from boot flash.
 */

/**
 * @brief  Wait until any pending SPM (Store Program Memory)
 *         operation has completed.
 */
void FLASH_waitBusy(void)
{
    boot_spm_busy_wait();
}

/**
 * @brief  No-op on AVR: the flash write hardware has no separate
 *         unlock sequence (the SPM instruction is gated by
 *         fuse/lock bits only).
 *
 *         Provided to satisfy the common driver interface.
 */
void FLASH_unlock(void)
{
    /* No unlock mechanism on AVR */
}

/**
 * @brief  No-op on AVR: there is no software lock register to set.
 *
 *         Provided to satisfy the common driver interface.
 */
void FLASH_lock(void)
{
    /* No lock mechanism on AVR */
}

/**
 * @brief  Erase a single flash page.
 *
 * @param  page_no  Zero-based page number within flash memory.
 */
void FLASH_erasePage(uint32_t page_no)
{
    uint32_t addr =
        page_no * (uint32_t)SPM_PAGESIZE;

    uint8_t sreg = SREG;

    cli();

    boot_page_erase(addr);
    boot_spm_busy_wait();

    /* Re-enable Read-While-Write section */
    boot_rww_enable_safe();

    SREG = sreg;
}

/**
 * @brief  Write data into flash using a read-modify-write cycle.
 *
 *         The function reads the full flash page that contains @p addr,
 *         overlays the new data, erases the page, fills the SPM temporary
 *         buffer word by word, and programs the page.
 *
 *         The RWW section is re-enabled at the end so the CPU can
 *         fetch instructions from application flash again.
 *
 * @param  addr     Byte address in flash where writing starts.
 *
 * @param  data     Pointer to source data
 *                  (pairs of uint32_t = 8 bytes per dataLen unit,
 *                  matching the upper layer chunking).
 *
 * @param  dataLen  Number of 8-byte chunks to write.
 */
void FLASH_write(uint32_t addr,
                 uint32_t const *data,
                 size_t dataLen)
{
    uint8_t tmp[SPM_PAGESIZE];

    uint16_t i;

    /* Determine page-aligned base address */
    uint32_t page_addr =
        addr & ~((uint32_t)(SPM_PAGESIZE - 1U));

    /* Determine byte offset within page */
    uint16_t offset =
        (uint16_t)(addr - page_addr);

    /*
     * Number of bytes carried in this call.
     *
     * Each dataLen unit contains:
     * 2 x uint32_t = 8 bytes.
     */
    uint16_t byte_count =
        (uint16_t)(dataLen * 8U);

    /*
     * Prevent writes beyond a single flash page.
     *
     * The upper layer is expected to split writes
     * on page boundaries.
     */
    if ((offset + byte_count) > SPM_PAGESIZE)
    {
        return;
    }

    /* --- Step 1: Read existing page content from flash --- */

    for (i = 0U; i < SPM_PAGESIZE; i++)
    {
        uint16_t read_addr =
            (uint16_t)(page_addr + i);

        tmp[i] =
            pgm_read_byte((const uint8_t *)read_addr);
    }

    /* --- Step 2: Overlay new data onto page buffer --- */

    memcpy(&tmp[offset], data, byte_count);

    /*
     * Flash programming must execute atomically.
     *
     * Save interrupt state and disable interrupts.
     */
    uint8_t sreg = SREG;

    cli();

    /* --- Step 3: Erase page --- */

    boot_page_erase(page_addr);
    boot_spm_busy_wait();

    /* --- Step 4: Fill temporary SPM page buffer --- */

    for (i = 0U; i < SPM_PAGESIZE; i += 2U)
    {
        uint16_t word =
            (uint16_t)tmp[i] |
            ((uint16_t)tmp[i + 1U] << 8U);

        boot_page_fill(page_addr + i, word);
    }

    /* --- Step 5: Program page --- */

    boot_page_write(page_addr);
    boot_spm_busy_wait();

    /* --- Step 6: Re-enable Read-While-Write section --- */

    boot_rww_enable_safe();

    /* Restore previous interrupt state */
    SREG = sreg;
}