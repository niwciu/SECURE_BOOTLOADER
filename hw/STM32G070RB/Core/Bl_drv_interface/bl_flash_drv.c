/**
 * @file bl_flash_drv.c
 * @author niwciu (niwciu@gmail.com)
 * @brief All info nedded to define this driver are placed in in RM0454
 * @version 1.0.0
 * @date 2026-05-05
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "bl_flash_drv.h"
#include "stm32g070xx.h"
#include "bl_hw_config.h"

#define BYTE_WROD_LEN 8
#define BYTE_HALF_WORD_LEN 4

static void FLASH_clearErrors(void);

void FLASH_waitBusy(void) 
{
    while (FLASH->SR & FLASH_SR_BSY1)
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

void FLASH_erasePage(uint32_t flas_page_no) 
{
   
    FLASH_waitBusy();
    FLASH_clearErrors();

    FLASH->CR |= FLASH_CR_PER; 
    FLASH->CR &= ~FLASH_CR_PNB;  // Clear PNB bits before writing new page adr
    FLASH->CR |= (flas_page_no << FLASH_CR_PNB_Pos);
    FLASH->CR |= FLASH_CR_STRT;

    FLASH_waitBusy();
    FLASH->CR &= ~FLASH_CR_PER;
} 

void FLASH_write(uint32_t addr, uint32_t const *data, size_t dataLen) 
{
    // Info in RM0454 site 60
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

        addr += BYTE_WROD_LEN; // 
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
                 FLASH_SR_OPTVERR;
                 
}