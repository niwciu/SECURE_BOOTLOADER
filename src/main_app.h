/**
 * @file main_app.h
 * @author niwciu (niwciu@gmail.com)
 * @brief Platform-independent secure bootloader entry point.
 * @version 1.0.0
 * @date 2026-05-05
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef MAIN_APP_H_
#define MAIN_APP_H_

/**
 * @brief  Bootloader entry point called from the platform hw/main.c.
 *
 *         Initialises all hardware and enters the bootloader main loop.
 *         Under normal operation the function never returns.
 *
 * @return Never returns; declared @c int to match hw/main.c convention.
 */
int main_app(void);

#endif /* MAIN_APP_H_ */
