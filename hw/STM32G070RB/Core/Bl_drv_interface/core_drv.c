/**
 * @file core_drv.c
 * @author niwciu (niwciu@gmail.com)
 * @brief
 * @version 1.0.0
 * @date 2026-05-05
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "core_drv.h"
#include "stm32g070xx.h"
#include "bl_hw_config.h"
#include <memory.h>
#include <stddef.h>

#define ICER_DISABLE_ALL_Msk 0xFFFFFFFFUL
#define ICPR_CLEAR_ALL_Msk 0xFFFFFFFFUL

typedef void (*app_entry_t)(void);

static SysTick_cb_t SysTick_cb;

void init_sys_tick(void)
{
    SysTick->LOAD = CPU_F / 10 - 1; // 100 ms
    SysTick->VAL = 0;
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void deinit_sys_tick(void)
{
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
}

void register_sysTick_cb(SysTick_cb_t callback)
{
    if (callback != NULL)
    {
        SysTick_cb = callback;
    }
}

void jump_to_app(void)
{
    volatile uint32_t *app_begin = (volatile uint32_t *)APP_START;
    uint32_t stack = *app_begin;
    uint32_t start = *(app_begin + 1);

    __disable_irq();

    NVIC->ICER[0] = ICER_DISABLE_ALL_Msk;
    NVIC->ICPR[0] = ICPR_CLEAR_ALL_Msk;
    /* for different cortex-m */
    // for (int i = 0; i < (sizeof(NVIC->ICER)/sizeof(NVIC->ICER[0])); i++)
    // {
    //     NVIC->ICER[i] = 0xFFFFFFFF;
    //     NVIC->ICPR[i] = 0xFFFFFFFF;
    // }
    SCB->VTOR = APP_START;

    __DSB();
    __ISB();

    __set_MSP(stack);

    __DSB();
    __ISB();

    app_entry_t app = (app_entry_t)start;
    app();
}

void system_reset(void)
{
    NVIC_SystemReset();
}

void SysTick_Handler(void)
{
    SysTick_cb();
}

bool CORE_is_valid_app(void)
{
    const uint32_t stack = *(const uint32_t *)APP_START;
    const uint32_t reset = *(const uint32_t *)(APP_START + 4U);

    if (stack < (uint32_t)SRAM_START || stack > (uint32_t)SRAM_END)
        return false;
    if (stack & 0x3U)
        return false;
    if ((reset & 1U) == 0U)
        return false;
    if (reset < (uint32_t)APP_START || reset >= (uint32_t)APP_END)
        return false;

    return true;
}
