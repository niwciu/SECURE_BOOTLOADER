/**
 * @file bl_status_led_drv.c
 * @author niwciu (niwciu@gmail.com)
 * @brief 
 * @version 1.0.0
 * @date 2026-05-05
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "bl_status_led_drv.h"
#include "stm32g474xx.h"

void init_bl_status_led(void)
{
    // PA5 - NUCLEO-G474RE LD2 green LED
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    // output mode => MODE5_0=1, MODE5_1=0
    GPIOA->MODER &= ~GPIO_MODER_MODE5;
    GPIOA->MODER |= GPIO_MODER_MODE5_0;
}

void deinit_bl_status_led_port(void)
{
    GPIOA->MODER |= GPIO_MODER_MODE5;
}

/**
 * @brief  use this function after de init all peripherials in case if some periph use same port
 */
void deinit_bl_status_led_clk(void)
{
    RCC->AHB2ENR &= ~RCC_AHB2ENR_GPIOAEN;
}

void toggle_bl_led(void)
{
    GPIOA->ODR ^= (GPIO_ODR_OD5);
}