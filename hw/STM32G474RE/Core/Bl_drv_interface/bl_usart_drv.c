/**
 * @file usart_drv.c
 * @author niwciu (niwciu@gmail.com)
 * @brief
 * @version 1.0.0
 * @date 2026-05-05
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "bl_usart_drv.h"
#include "stm32g474xx.h"
#include "bl_hw_config.h"

#define USART_ICR_CLEAR_IRQ_Msk 0x00123BFFUL

static data_rx_cb_t byte_rx_cb;

void init_bl_usart(void)
{
    // USART2 on PA2(TX)/PA3(RX) - NUCLEO-G474RE ST-LINK VCP, AF7
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
    RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOAEN;

    GPIOA->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
    GPIOA->MODER |=  (GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1);
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3);
    GPIOA->PUPDR |=   GPIO_PUPDR_PUPD2_0 | GPIO_PUPDR_PUPD3_0;

    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2 | GPIO_AFRL_AFSEL3);
    GPIOA->AFR[0] |=  (GPIO_AFRL_AFSEL2_0 | GPIO_AFRL_AFSEL2_1 | GPIO_AFRL_AFSEL2_2 |
                       GPIO_AFRL_AFSEL3_0 | GPIO_AFRL_AFSEL3_1 | GPIO_AFRL_AFSEL3_2);

    USART2->BRR = CPU_F / BAUD;
    USART2->CR1 = USART_CR1_RXNEIE_RXFNEIE | USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;

    USART2->ICR = USART_ICR_CLEAR_IRQ_Msk;
    NVIC_ClearPendingIRQ(USART2_IRQn);

    NVIC_EnableIRQ(USART2_IRQn);
}

void register_data_rx_cb(data_rx_cb_t callback)
{
    if (callback != NULL)
    {
        byte_rx_cb = callback;
    }
}

void deinit_bl_usart_periph(void)
{
    NVIC_DisableIRQ(USART2_IRQn);

    GPIOA->MODER |=  (GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3);
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2 | GPIO_AFRL_AFSEL3);

    USART2->BRR = 0;
    USART2->CR1 &= ~(USART_CR1_RXNEIE_RXFNEIE | USART_CR1_RE | USART_CR1_TE | USART_CR1_UE);
}

/**
 * @brief  use this function after de init all peripherials in case if some periph use same port
 */
void deinit_bl_usart_clk(void)
{
    RCC->APB1ENR1 &= ~RCC_APB1ENR1_USART2EN;
    RCC->AHB2ENR  &= ~RCC_AHB2ENR_GPIOAEN;
}

void send_byte_table(const uint8_t *d, size_t l)
{
    for (size_t i = 0; i < l; ++i)
        send_byte(d[i]);
}

void send_byte(uint8_t b)
{
    uint32_t timeout = 1000000UL;

    while (!(USART2->ISR & USART_ISR_TXE_TXFNF))
    {
        if (--timeout == 0)
        {
            return;
        }
    }
    USART2->TDR = b;
}

bool byte_transmission_complete(void)
{
    return (USART2->ISR & USART_ISR_TC);
}

void USART2_IRQHandler(void)
{
    uint32_t isr = USART2->ISR;

    if (isr & (USART_ISR_ORE |
               USART_ISR_FE  |
               USART_ISR_NE  |
               USART_ISR_PE))
    {
        USART2->ICR = USART_ICR_ORECF |
                      USART_ICR_FECF  |
                      USART_ICR_NECF  |
                      USART_ICR_PECF;

        if (isr & USART_ISR_RXNE_RXFNE)
        {
            (void)USART2->RDR;
        }

        return;
    }

    if (isr & USART_ISR_RXNE_RXFNE)
    {
        uint8_t c = (uint8_t)USART2->RDR;
        if (byte_rx_cb != NULL)
        {
            byte_rx_cb(c);
        }
    }
}
