/**
 * @file bl_key_drv.h
 * @author niwciu (niwciu@gmail.com)
 * @brief 
 * @version 1.0.0
 * @date 2026-05-05
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef _BL_KEY_DRV_H_
#define _BL_KEY_DRV_H_

 #include <stdbool.h>

void init_bl_key(void);
void deinit_bl_key(void);
void deinit_bl_key_clk(void);
bool bl_key_pressed(void);

#endif /* _BL_KEY_DRV_H_ */