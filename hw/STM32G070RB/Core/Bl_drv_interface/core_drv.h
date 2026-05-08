/**
 * @file core_drv.h
 * @author niwciu (niwciu@gmail.com)
 * @brief
 * @version 1.0.0
 * @date 2026-05-05
 *
 * @copyright Copyright (c) 2026
 *
 */
#ifndef _CORE_DRV_H_
#define _CORE_DRV_H_

#include <stdbool.h>

typedef void (*SysTick_cb_t)(void);

void init_sys_tick(void);
void deinit_sys_tick(void);
void deinit_nvic(void);
void register_sysTick_cb(SysTick_cb_t callback);
void jump_to_app(void);
void system_reset(void);
bool CORE_is_valid_app(void);

#endif /* _CORE_DRV_H_*/