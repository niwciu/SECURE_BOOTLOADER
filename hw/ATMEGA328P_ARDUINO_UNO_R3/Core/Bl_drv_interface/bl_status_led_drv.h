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
#ifndef _BL_STATUS_LED_DRV_H_
#define _BL_STATUS_LED_DRV_H_

void init_bl_status_led(void);
void deinit_bl_status_led_port(void);
void deinit_bl_status_led_clk(void);
void toogle_bl_led(void);

#endif /* _BL_STATUS_LED_DRV_H_ */
