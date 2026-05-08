/**
 * @file bl_key_drv.c
 * @author niwciu (niwciu@gmail.com)
 * @brief
 * @version 1.0.0
 * @date 2026-05-07
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "bl_key_drv.h"
#include <avr/io.h>
#include <stdbool.h>

/*
 * Button: PD2 (Arduino Uno digital pin 2, INT0)
 * Active LOW with internal pull-up enabled.
 */

void init_bl_key(void)
{
    /* Configure PD2 as input */
    // DDRD  &= ~(1 << PD2);
    /* Enable internal pull-up on PD2 */
    // PORTD |=  (1 << PD2);
}

void deinit_bl_key(void)
{
    /* Disable internal pull-up (tri-state input) */
    // PORTD &= ~(1 << PD2);
}

/**
 * @brief  No-op on AVR: there is no per-GPIO clock gate.
 *         Provided to satisfy the common driver interface.
 */
void deinit_bl_key_clk(void)
{
    /* AVR has no GPIO clock gating — nothing to do */
}

bool bl_key_pressed(void)
{
    /* Button is active LOW; pressed when PD2 reads 0 */
    // return !(PIND & (1 << PD2));
    return false;
}
