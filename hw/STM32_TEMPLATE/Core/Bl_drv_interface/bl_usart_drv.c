/**
 * @file bl_usart_drv.c
 * @brief USART driver stub – implement for your target MCU.
 *
 *        TODO: configure a USART peripheral for:
 *          - 8N1 framing at BAUD (defined in bl_hw_config.h)
 *          - RX interrupt that calls the registered callback for each byte
 *          - Blocking TX (polling TXE/TC flags)
 *
 *        GPIO pin and USART instance are target-specific — pick any USART
 *        whose RX/TX pins are routed to the board connector you intend to use.
 */

#include "bl_usart_drv.h"
/* TODO: replace with the correct CMSIS device header */
#include "stm32xxxxxx.h"
#include "bl_hw_config.h"

static data_rx_cb_t rx_callback = NULL;

void register_data_rx_cb(data_rx_cb_t callback)
{
    rx_callback = callback;
}

void init_bl_usart(void)
{
    /* TODO:
     *  1. Enable GPIO and USART clocks via RCC
     *  2. Configure GPIO pins as alternate function (TX/RX)
     *  3. Set baud rate: BRR = CPU_F / BAUD  (for oversampling-16)
     *  4. Enable RXNE interrupt (RXNEIE bit in CR1)
     *  5. Enable NVIC for the USART IRQ
     *  6. Enable USART (UE bit), then TE and RE bits */
}

void deinit_bl_usart_periph(void)
{
    /* TODO: disable USART (clear UE, TE, RE bits) and RXNE interrupt */
}

void deinit_bl_usart_clk(void)
{
    /* TODO: gate off the USART clock in RCC */
}

void send_byte(uint8_t b)
{
    /* TODO: wait for TXE (transmit data register empty), then write to TDR/DR */
    (void)b;
}

void send_byte_table(const uint8_t *d, size_t l)
{
    while (l--)
    {
        send_byte(*d++);
    }
}

bool byte_transmission_complete(void)
{
    /* TODO: return true when TC (transmission complete) flag is set in SR/ISR */
    return false;
}

/* USART RX interrupt handler */
/* TODO: rename to match your MCU's USART IRQ handler name, e.g.:
 *   USART1_IRQHandler / USART2_IRQHandler / LPUART1_IRQHandler
 * The name must match the symbol in the startup file vector table. */
void USARTx_IRQHandler(void)
{
    /* TODO: check RXNE flag in SR/ISR, read the received byte from RDR/DR,
     * then call the registered callback. */
    if (rx_callback != NULL)
    {
        /* rx_callback(received_byte); */
    }
}
