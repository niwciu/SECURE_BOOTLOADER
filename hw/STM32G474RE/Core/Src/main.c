/**
 * @file main.c
 * @author niwciu (niwciu@gmail.com)
 * @brief Platform entry point for STM32G474RE – delegates to bootloader core.
 * @version 1.0.0
 * @date 2026-05-05
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "stm32g474xx.h"
#include "main_app.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

int main(void)
{
    main_app();
    return 0;
}
