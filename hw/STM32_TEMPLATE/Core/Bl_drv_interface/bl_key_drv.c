/**
 * @file bl_key_drv.c
 * @brief Push-button driver stub – implement for your target MCU.
 *
 *        TODO: configure a GPIO input pin connected to a push-button.
 *        Decide on active-high or active-low and adjust bl_key_pressed()
 *        accordingly.  No interrupt is required – the main loop polls
 *        bl_key_pressed() on every iteration.
 */

#include "bl_key_drv.h"
/* TODO: replace with the correct CMSIS device header */
#include "stm32xxxxxx.h"

void init_bl_key(void)
{
    /* TODO:
     *  1. Enable GPIO clock via RCC
     *  2. Configure the pin as input (MODER = 00)
     *  3. Enable internal pull-up or pull-down as required by your circuit */
}

void deinit_bl_key(void)
{
    /* TODO: reset the GPIO pin to its reset state (input, no pull) */
}

void deinit_bl_key_clk(void)
{
    /* TODO: gate off the GPIO clock in RCC (only if no other pin on the same
     * port is still in use) */
}

bool bl_key_pressed(void)
{
    /* TODO: return true when the button GPIO pin reads as pressed.
     * Example (active-low, pin PA0):
     *   return !(GPIOA->IDR & GPIO_IDR_ID0); */
    return false;
}
