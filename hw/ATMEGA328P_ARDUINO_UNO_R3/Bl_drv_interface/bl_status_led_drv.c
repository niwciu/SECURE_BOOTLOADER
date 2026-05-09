/**
 * @file bl_status_led_drv.c
 * @author niwciu (niwciu@gmail.com)
 * @brief
 * @version 1.0.0
 * @date 2026-05-07
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "bl_status_led_drv.h"
#include <avr/io.h>

/*
 * Status LED: PB5 (Arduino Uno built-in LED, digital pin 13)
 * LED is active HIGH (lit when PB5 is driven high).
 */

void init_bl_status_led(void)
{
    /* Configure PB5 as output */
    DDRB  |=  (1 << PB5);
    /* LED off initially */
    PORTB &= ~(1 << PB5);
}

void deinit_bl_status_led_port(void)
{
    /* Return PB5 to input (high-Z) and clear output latch */
    DDRB  &= ~(1 << PB5);
    PORTB &= ~(1 << PB5);
}

/**
 * @brief  No-op on AVR: there is no per-GPIO clock gate.
 *         Provided to satisfy the common driver interface.
 */
void deinit_bl_status_led_clk(void)
{
    /* AVR has no GPIO clock gating — nothing to do */
}

void toggle_bl_led(void)
{
    /* Writing 1 to the PIN register toggles the corresponding PORT bit */
    PINB = (1 << PB5);
}
