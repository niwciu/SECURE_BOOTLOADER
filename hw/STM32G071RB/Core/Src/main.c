/**
 * @file main.c
 * @author niwciu (niwciu@gmail.com)
 * @brief Platform entry point for STM32G071RB – delegates to bootloader core.
 * @version 1.0.0
 * @date 2026-05-05
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "stm32g071xx.h"
#include "main_app.h"

int main(void)
{
    main_app();
    return 0;
}
