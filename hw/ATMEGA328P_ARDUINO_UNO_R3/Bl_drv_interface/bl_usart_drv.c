/**
 * @file bl_usart_drv.c
 * @author niwciu (niwciu@gmail.com)
 * @brief
 * @version 1.0.0
 * @date 2026-05-07
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "bl_usart_drv.h"
#include "bl_hw_config.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/*
 * USART0: PD0 = RX, PD1 = TX
 *
 * 115200 baud at 16 MHz with U2X = 1:
 *
 * UBRR = round(CPU_F / (8 * BAUD)) - 1
 *      = round(16000000 / 921600) - 1
 *      = round(17.36) - 1
 *      = 17 - 1
 *      = 16
 *
 * Actual baud rate:
 * ≈ 117647 baud
 *
 * Baud error:
 * ≈ +2.1 %
 */
#define UBRR_VAL \
    ((CPU_F + (4UL * BAUD)) / (8UL * BAUD) - 1UL)

/* RX callback invoked from USART RX ISR */
static volatile data_rx_cb_t byte_rx_cb;

/**
 * @brief  Initialize USART0 peripheral for bootloader communication.
 *
 *         Configuration:
 *         - 115200 baud
 *         - 8 data bits
 *         - no parity
 *         - 1 stop bit
 *         - double-speed mode enabled (U2X)
 *
 *         NOTE:
 *         This function does NOT enable global interrupts.
 */
void init_bl_usart(void)
{
    /*
     * Ensure USART0 clock is enabled.
     *
     * Required if deinit_bl_usart_clk()
     * was called previously.
     */
    PRR &= ~(1 << PRUSART0);

    /* Set baud rate */
    UBRR0H = (uint8_t)(UBRR_VAL >> 8U);
    UBRR0L = (uint8_t)(UBRR_VAL);

    /* Enable double-speed mode */
    UCSR0A = (1 << U2X0);

    /*
     * Enable:
     * - RX complete interrupt
     * - receiver
     * - transmitter
     */
    UCSR0B =
        (1 << RXCIE0) |
        (1 << RXEN0)  |
        (1 << TXEN0);

    /*
     * Frame format:
     * - 8 data bits
     * - 1 stop bit
     * - no parity
     */
    UCSR0C =
        (1 << UCSZ01) |
        (1 << UCSZ00);
}

/**
 * @brief  Register RX callback function.
 *
 * @param  callback  Function invoked from USART RX ISR
 *                   when a byte is received.
 */
void register_data_rx_cb(data_rx_cb_t callback)
{
    uint8_t sreg = SREG;

    cli();

    byte_rx_cb = callback;

    SREG = sreg;
}

/**
 * @brief  Disable USART0 transmitter, receiver and interrupts.
 */
void deinit_bl_usart_periph(void)
{
    UCSR0B = 0U;
}

/**
 * @brief  Power down USART0 via Power Reduction Register.
 *
 *         Call after deinit_bl_usart_periph().
 */
void deinit_bl_usart_clk(void)
{
    PRR |= (1 << PRUSART0);
}

/**
 * @brief  Send a single byte.
 *
 * @param  b  Byte to transmit.
 *
 * @note   This function waits until the USART data register
 *         becomes empty and then loads the byte into the
 *         transmitter shift register.
 *
 *         It does NOT wait for full transmission completion
 *         on the TX pin.
 */
void send_byte(uint8_t b)
{
    /*
     * Guard against stuck hardware.
     *
     * This timeout is iteration-based,
     * not time-based.
     */
    uint32_t timeout = 1000000UL;

    /*
     * Clear transmission-complete flag.
     *
     * TXC0 is cleared by writing logic 1.
     */
    UCSR0A |= (1 << TXC0);

    /* Wait until transmit data register becomes empty */
    while (!(UCSR0A & (1 << UDRE0)))
    {
        if (--timeout == 0UL)
        {
            return;
        }
    }

    /* Load byte into USART data register */
    UDR0 = b;
}

/**
 * @brief  Send multiple bytes.
 *
 * @param  d  Pointer to data buffer.
 * @param  l  Number of bytes to send.
 */
void send_byte_table(const uint8_t *d, size_t l)
{
    size_t i;

    for (i = 0U; i < l; ++i)
    {
        send_byte(d[i]);
    }
}

/**
 * @brief  Check whether transmission has fully completed.
 *
 * @retval true   Transmission complete.
 * @retval false  Transmission still in progress.
 */
bool byte_transmission_complete(void)
{
    return ((UCSR0A & (1 << TXC0)) != 0U);
}

/**
 * @brief  USART0 RX complete interrupt handler.
 *
 *         Reads received byte from UDR0 and forwards it
 *         to the registered callback function.
 */
ISR(USART_RX_vect)
{
    uint8_t received = UDR0;

    if (byte_rx_cb != NULL)
    {
        byte_rx_cb(received);
    }
}