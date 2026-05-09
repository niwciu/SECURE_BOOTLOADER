/**
 * @file core_drv.c
 * @brief Core / SysTick / application-jump driver stub – implement for your
 *        target MCU.
 *
 *        TODO: implement the functions below.  Key points:
 *
 *        init_sys_tick()  – configure SysTick for a 100 ms interrupt period.
 *                           The ISR must call the registered callback.
 *
 *        jump_to_app()    – deinitialise the NVIC, set MSP from the
 *                           application vector table, then branch to the
 *                           application reset handler.
 *
 *        CORE_is_valid_app() – decide whether a valid application image is
 *                           present.  A common heuristic: check that the first
 *                           word (initial MSP) falls inside SRAM and the
 *                           second word (reset vector) falls inside the
 *                           application flash region.
 */

#include "core_drv.h"
/* TODO: replace with the correct CMSIS device header */
#include "stm32xxxxxx.h"
#include "bl_hw_config.h"

#include <stdbool.h>
#include <stdint.h>

static SysTick_cb_t systick_callback = NULL;

void register_sysTick_cb(SysTick_cb_t callback)
{
    systick_callback = callback;
}

void init_sys_tick(void)
{
    /* TODO: configure SysTick for a 100 ms period.
     * Example using CMSIS SysTick_Config():
     *   SysTick_Config(CPU_F / 10);   // CPU_F / 10 = 100 ms at given CPU_F
     * Then enable SysTick IRQ in NVIC (usually already done by SysTick_Config). */
}

void deinit_sys_tick(void)
{
    /* TODO: disable SysTick counter and its interrupt */
    SysTick->CTRL = 0;
}

void deinit_nvic(void)
{
    /* TODO: disable all NVIC interrupts and clear all pending flags so the
     * application starts with a clean interrupt state.
     * Iterate over NVIC->ICER[] and NVIC->ICPR[] registers. */
}

void jump_to_app(void)
{
    /* TODO: transfer control to the application.
     * Typical sequence:
     *   1. Disable all interrupts (__disable_irq())
     *   2. Deinitialise NVIC
     *   3. Set VTOR to APP_START (if the MCU has VTOR; not all Cortex-M0 do)
     *   4. Load new MSP from APP_START[0]
     *   5. Branch to reset handler at APP_START[1]
     *
     * Example:
     *   typedef void (*app_entry_t)(void);
     *   __disable_irq();
     *   deinit_nvic();
     *   SCB->VTOR = APP_START;
     *   __set_MSP(*(volatile uint32_t *)APP_START);
     *   app_entry_t app = (app_entry_t)(*(volatile uint32_t *)(APP_START + 4));
     *   app();
     */
}

void system_reset(void)
{
    /* CMSIS provides this as a portable inline – usually safe to keep as-is */
    NVIC_SystemReset();
}

bool CORE_is_valid_app(void)
{
    /* TODO: verify the application image at APP_START.
     * Minimum check: MSP must point into SRAM and reset vector into app flash.
     *
     * Example:
     *   uint32_t msp    = *(volatile uint32_t *) APP_START;
     *   uint32_t reset  = *(volatile uint32_t *)(APP_START + 4);
     *   return (msp    >= SRAM_START && msp    <= SRAM_END) &&
     *          (reset  >= APP_START  && reset  <  APP_END);
     */
    return false;
}

/* SysTick interrupt handler */
void SysTick_Handler(void)
{
    if (systick_callback != NULL)
    {
        systick_callback();
    }
}
