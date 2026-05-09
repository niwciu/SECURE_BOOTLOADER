/**
 * @file core_drv.c
 * @author niwciu (niwciu@gmail.com)
 * @brief
 * @version 1.0.0
 * @date 2026-05-07
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "core_drv.h"
#include "bl_hw_config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <stddef.h>

/* ATmega328P reset vector: 2-word JMP instruction. Word 0 = opcode 0x940C,
 * word 1 = target word-address. Erased flash reads 0xFFFF → not 0x940C → false. */
#define AVR_JMP_OPCODE 0x940CU

/*
 * Timer1 CTC mode, 100 ms tick at 16 MHz:
 *   Prescaler = 1024
 *   OCR1A = (CPU_F / 1024 / 10) - 1 = (16000000 / 1024 / 10) - 1 = 1562 - 1 = 1561
 */

static SysTick_cb_t SysTick_cb;

void init_sys_tick(void)
{
    /* CTC mode: WGM12=1 in TCCR1B */
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); /* CTC, prescaler 1024 */

    /* Compare match value for 100 ms */
    OCR1A = (uint16_t)((CPU_F / 1024UL / 10UL) - 1UL);

    /* Enable Output Compare A Match interrupt */
    TIMSK1 = (1 << OCIE1A);

    /* Enable global interrupts */
    sei();
}

void deinit_sys_tick(void)
{
    /* Stop the timer */
    TCCR1B = 0;
    /* Disable the compare-match interrupt */
    TIMSK1 &= ~(1 << OCIE1A);
    /* Reset counter */
    TCNT1 = 0;
}

void register_sysTick_cb(SysTick_cb_t callback)
{
    if (callback != NULL)
    {
        SysTick_cb = callback;
    }
}

void jump_to_app(void)
{
    /* 1. Globally disable interrupts to ensure safe transition */
    cli();

    /* 2. Reset the Interrupt Vector Selection
        This moves the interrupt vector table back to the start of FLASH (0x0000).
        Required because the CPU won't do this automatically on a software jump. */
    MCUCR = (1 << IVCE);  /* Enable change of Interrupt Vectors */
    MCUCR = 0;            /* Move vectors to Application Section (0x0000) */

    /* 3. Perform the jump to the application reset vector (address 0x0000) */
    typedef void (*app_ptr_t)(void);
    app_ptr_t start_application = (app_ptr_t)0x0000;
    start_application();
}

void system_reset(void)
{
    /* Enable watchdog with shortest timeout, then spin to trigger reset */
    wdt_enable(WDTO_15MS);
    while (1)
    {
    }
}

ISR(TIMER1_COMPA_vect)
{
    if (SysTick_cb != NULL)
    {
        SysTick_cb();
    }
}

bool CORE_is_valid_app(void)
{
    /* Word 0 at APP_START must be the JMP opcode */
    if (pgm_read_word(APP_START) != AVR_JMP_OPCODE)
        return false;

    /* Word 1 is the reset handler word-address; convert to byte-address */
    uint32_t byte_addr = (uint32_t)pgm_read_word(APP_START + 2U) * 2U;
    if (byte_addr < (uint32_t)APP_START || byte_addr >= (uint32_t)APP_END)
        return false;

    return true;
}
