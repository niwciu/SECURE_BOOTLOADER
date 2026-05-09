/**
 * @file bl_key_drv.c
 * @author niwciu (niwciu@gmail.com)
 * @brief
 * @version 1.0.0
 * @date 2026-05-05
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "bl_key_drv.h"
#include "stm32g474xx.h"

void init_bl_key(void)
{
    // PC13 - NUCLEO-G474RE B1 USER button, active LOW
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

    // input mode => MODE13_0=0, MODE13_1=0
    GPIOC->MODER &= ~GPIO_MODER_MODE13;
}

void deinit_bl_key(void)
{
    GPIOC->MODER |= GPIO_MODER_MODE13;
}

/**
 * @brief  use this function after de init all peripherials in case if some periph use same port
 */
void deinit_bl_key_clk(void)
{
    RCC->AHB2ENR &= ~RCC_AHB2ENR_GPIOCEN;
}


bool bl_key_pressed(void)
{
    bool key_press_status = false;
    if((GPIOC->IDR & GPIO_IDR_ID13_Msk) == GPIO_IDR_ID13_Msk)
    {
        key_press_status = true;
    }
    return key_press_status;
}