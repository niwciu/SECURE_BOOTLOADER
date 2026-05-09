/**
 * @file bl_status_led_drv.h
 * @author niwciu (niwciu@gmail.com)
 * @brief
 * @version 1.0.0
 * @date 2026-05-07
 *
 * @copyright Copyright (c) 2026
 *
 */
#ifndef BL_STATUS_LED_DRV_H_
#define BL_STATUS_LED_DRV_H_

void init_bl_status_led(void);
void deinit_bl_status_led_port(void);
void deinit_bl_status_led_clk(void);
void toggle_bl_led(void);

#endif /* BL_STATUS_LED_DRV_H_ */
