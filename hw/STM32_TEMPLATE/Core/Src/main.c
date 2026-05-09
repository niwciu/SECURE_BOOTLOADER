/**
 * @file main.c
 * @brief Platform entry point – delegates immediately to the bootloader core.
 *
 *        TODO: replace the MCU header include below with the correct header
 *        for your target (e.g. "stm32l4xx.h", "stm32h7xx.h").
 */

/* TODO: replace with the correct CMSIS device header for your MCU */
#include "stm32xxxxxx.h"
#include "main_app.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

static void core_init(void);

int main(void)
{
    /* TODO: add any MCU-specific core setup here (e.g. clock config, FPU
     * enable, VTOR relocation) that must happen before the bootloader runs. */
    core_init();

    main_app();

    for (;;)
    {
    }
    return 0;
}

static void core_init(void)
{
    /* TODO: implement clock / core initialisation if needed, or leave empty */
}
