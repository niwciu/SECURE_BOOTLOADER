/**
 * @file bl_flash_drv.c
 * @brief Flash driver stub – implement for your target MCU.
 *
 *        TODO: replace the MCU header and implement each function using
 *        the register-level API from your MCU Reference Manual.
 *        Key RM sections to read:
 *          - Flash programming sequence (PG bit flow)
 *          - Page/sector erase sequence (PER/SER bit flow)
 *          - Flash unlock/lock key sequence (KEYR register)
 */

#include "bl_flash_drv.h"
/* TODO: replace with the correct CMSIS device header */
#include "stm32xxxxxx.h"
#include "bl_hw_config.h"

void FLASH_waitBusy(void)
{
    /* TODO: poll the busy flag in FLASH->SR (BSY bit name varies by family) */
}

void FLASH_unlock(void)
{
    FLASH_waitBusy();
    /* TODO: write FLASH_KEY1 then FLASH_KEY2 to FLASH->KEYR */
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
    FLASH_waitBusy();
}

void FLASH_lock(void)
{
    FLASH_waitBusy();
    /* TODO: set the LOCK bit in FLASH->CR */
    FLASH->CR |= FLASH_CR_LOCK;
    FLASH_waitBusy();
}

void FLASH_erasePage(uint32_t flash_page_no)
{
    /* TODO: implement page/sector erase sequence from the RM.
     * Typical steps:
     *   1. Wait busy
     *   2. Clear error flags
     *   3. Set PER (or SER) bit and write page number
     *   4. Set STRT bit to start erase
     *   5. Wait busy
     *   6. Clear PER/SER bit */
    (void)flash_page_no;
}

void FLASH_write(uint32_t addr, uint32_t const *data, size_t dataLen)
{
    /* TODO: implement double-word (64-bit) flash programming sequence.
     * Typical steps:
     *   1. Wait busy
     *   2. Clear error flags
     *   3. Set PG bit
     *   4. Write two consecutive 32-bit words per double-word
     *   5. Wait busy, check EOP
     *   6. Clear PG bit
     * dataLen counts double-words (2 x uint32_t each). */
    (void)addr;
    (void)data;
    (void)dataLen;
}
