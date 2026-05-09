/**
 * @file bl_status_led_drv.c
 * @brief Status LED driver stub – implement for your target MCU.
 *
 *        TODO: configure a GPIO output pin connected to the bootloader
 *        status LED.  The LED is toggled by toggle_bl_led() every 500 ms
 *        (called from the main loop via update_bl_status_LED()).
 */

#include "bl_status_led_drv.h"
/* TODO: replace with the correct CMSIS device header */
#include "stm32xxxxxx.h"

void init_bl_status_led(void)
{
    /* TODO:
     *  1. Enable GPIO clock via RCC
     *  2. Configure the pin as push-pull output (MODER = 01, OTYPER = 0)
     *  3. Set initial state (LED off) */
}

void deinit_bl_status_led_port(void)
{
    /* TODO: reset the GPIO pin to its reset state (input, no pull, LED off) */
}

void deinit_bl_status_led_clk(void)
{
    /* TODO: gate off the GPIO clock in RCC (only if no other pin on the same
     * port is still in use) */
}

void toggle_bl_led(void)
{
    /* TODO: toggle the LED GPIO output.
     * Example using BSRR (atomic, preferred):
     *   GPIOA->BSRR = (GPIOA->ODR & GPIO_ODR_OD5)
     *                 ? GPIO_BSRR_BR5   // turn off
     *                 : GPIO_BSRR_BS5;  // turn on
     * Or using ODR XOR (single register):
     *   GPIOA->ODR ^= GPIO_ODR_OD5; */
}
