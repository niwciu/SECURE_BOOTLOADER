/**
 * @file main.c
 * @author niwciu (niwciu@gmail.com)
 * @brief
 * @version 1.0.0
 * @date 2026-05-07
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <avr/io.h>
#include <avr/wdt.h>
#include "main_app.h"
#include <avr/interrupt.h>

int main(void)
{
    

    // relocate interrupt vectors to the begining of bootloader memory section
    MCUCR = (1<<IVCE);
    MCUCR = (1<<IVSEL);
    /* Clear WDT reset flag and disable watchdog early.
     * After a WDT-triggered reset (system_reset()), the MCU re-enters the
     * bootloader with WDRF set and WDT still enabled. Clearing MCUSR and
     * calling wdt_disable() must happen before the first WDT timeout. */
    MCUSR = 0;
    wdt_disable();

    return main_app();
}
