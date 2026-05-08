/**
 * @file bl_flash_drv.c
 * @author niwciu (niwciu@gmail.com)
 * @brief All info needed to define this driver are placed in RM0440
 * @version 1.0.0
 * @date 2026-05-05
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "bl_flash_drv.h"
#include "stm32g474xx.h"
#include "bl_hw_config.h"

#define BYTE_WORD_LEN       8
#define BYTE_HALF_WORD_LEN  4
#define PAGES_PER_BANK      128U

static void FLASH_clearErrors(void);

void FLASH_waitBusy(void)
{
    while (FLASH->SR & FLASH_SR_BSY)
    {
    }
}

void FLASH_unlock(void)
{
    FLASH_waitBusy();

    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;

    FLASH_waitBusy();
}

void FLASH_lock(void)
{
    FLASH_waitBusy();
    FLASH->CR |= FLASH_CR_LOCK;
    FLASH_waitBusy();
}

void FLASH_erasePage(uint32_t flash_page_no)
{
    /*
     * To erase a page (2 Kbytes), follow the procedure below (RM0440):
        1. Check that no Flash memory operation is ongoing by checking the BSY bit of
           the FLASH status register (FLASH_SR).
        2. Check and clear all error programming flags due to a previous programming. If not,
           PGSERR is set.
        3. Set the PER bit and select the page to erase (PNB) and bank (BKER) in the
           FLASH control register (FLASH_CR).
        4. Set the STRT bit of the FLASH control register (FLASH_CR).
        5. Wait until the BSY bit of the FLASH status register (FLASH_SR) is cleared.
     */
    FLASH_waitBusy();
    FLASH_clearErrors();

    FLASH->CR |= FLASH_CR_PER;
    FLASH->CR &= ~(FLASH_CR_PNB | FLASH_CR_BKER);

    if (flash_page_no >= PAGES_PER_BANK)
    {
        FLASH->CR |= FLASH_CR_BKER;
        FLASH->CR |= ((flash_page_no - PAGES_PER_BANK) << FLASH_CR_PNB_Pos);
    }
    else
    {
        FLASH->CR |= (flash_page_no << FLASH_CR_PNB_Pos);
    }

    FLASH->CR |= FLASH_CR_STRT;

    FLASH_waitBusy();
    FLASH->CR &= ~FLASH_CR_PER;
}

void FLASH_write(uint32_t addr, uint32_t const *data, size_t dataLen)
{
    // Info in RM0440 - programming is done in double-words (64 bits)
    FLASH_waitBusy();
    FLASH_clearErrors();
    FLASH->CR |= FLASH_CR_PG;

    for(size_t i=0; i<dataLen; i++)
    {
        uint32_t hW = *data;
        data++;
        uint32_t lW = *data;
        data++;
        *(volatile uint32_t *)addr = hW;
        *(volatile uint32_t *)(addr + BYTE_HALF_WORD_LEN) = lW;
        FLASH_waitBusy();

        FLASH->SR &= ~FLASH_SR_EOP;

        addr += BYTE_WORD_LEN;
    }

    FLASH->CR &= ~FLASH_CR_PG;
}

static void FLASH_clearErrors(void)
{
    FLASH->SR |= FLASH_SR_OPERR |
                 FLASH_SR_PROGERR |
                 FLASH_SR_WRPERR |
                 FLASH_SR_PGAERR |
                 FLASH_SR_SIZERR |
                 FLASH_SR_PGSERR |
                 FLASH_SR_MISERR |
                 FLASH_SR_FASTERR |
                 FLASH_SR_RDERR |
                 FLASH_SR_OPTVERR;
}