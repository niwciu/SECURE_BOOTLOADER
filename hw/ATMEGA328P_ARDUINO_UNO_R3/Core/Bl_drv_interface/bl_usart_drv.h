/**
 * @file bl_usart_drv.h
 * @author niwciu (niwciu@gmail.com)
 * @brief
 * @version 1.0.0
 * @date 2026-05-07
 *
 * @copyright Copyright (c) 2026
 *
 */
#ifndef _BL_USART_DRV_H_
#define _BL_USART_DRV_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef void (*data_rx_cb_t)(uint8_t data);

void init_bl_usart(void);
void register_data_rx_cb(data_rx_cb_t callback);
void deinit_bl_usart_periph(void);
void deinit_bl_usart_clk(void);
void send_byte_table(const uint8_t *d, size_t l);
void send_byte(uint8_t b);
bool byte_transmission_complete(void);

#endif /* _BL_USART_DRV_H_ */
