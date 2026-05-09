#include "unity/fixture/unity_fixture.h"
#include "mock_bl_usart_drv.h"
#include "mock_bl_flash_drv.h"
#include "mock_core_drv.h"
#include "mock_bl_key_drv.h"
#include "mock_bl_status_led_drv.h"
#include "mock_crc_api.h"
#include "mock_aes.h"

#include "main_app.h"
#include "main_app_priv.h"

/* Internals under test */
extern volatile uint32_t _100ms_Tick;
extern uint32_t timeout;
extern uint8_t blink_event;
extern uint8_t key_timer;
void update_bl_status_led(void);
void bl_key_check(void);

TEST_GROUP(main_app_led_key);

TEST_SETUP(main_app_led_key)
{
    mock_bl_usart_drv_Init();
    mock_bl_flash_drv_Init();
    mock_core_drv_Init();
    mock_bl_key_drv_Init();
    mock_bl_status_led_drv_Init();
    mock_crc_api_Init();
    mock_aes_Init();

    _100ms_Tick = 0;
    blink_event = 0;
    timeout = START_TIMEOUT;
    key_timer = 0;
}

TEST_TEAR_DOWN(main_app_led_key)
{
    mock_bl_usart_drv_Verify();
    mock_bl_flash_drv_Verify();
    mock_core_drv_Verify();
    mock_bl_key_drv_Verify();
    mock_bl_status_led_drv_Verify();
    mock_crc_api_Verify();
    mock_aes_Verify();

    mock_bl_usart_drv_Destroy();
    mock_bl_flash_drv_Destroy();
    mock_core_drv_Destroy();
    mock_bl_key_drv_Destroy();
    mock_bl_status_led_drv_Destroy();
    mock_crc_api_Destroy();
    mock_aes_Destroy();
}

/* ============================================================
 * update_bl_status_led
 * ============================================================ */

TEST(main_app_led_key, GivenTickDivisibleBy5AndBlinkEventClear_WhenLEDUpdateCalled_ThenLEDToggled)
{
    _100ms_Tick = 5;
    blink_event = 0;

    toggle_bl_led_Expect();

    update_bl_status_led();

    TEST_ASSERT_EQUAL(1, blink_event);
}

TEST(main_app_led_key, GivenTickDivisibleBy5AndBlinkEventSet_WhenLEDUpdateCalled_ThenLEDNotToggled)
{
    _100ms_Tick = 5;
    blink_event = 1;

    /* toggle_bl_led must NOT be called — no Expect set up */
    update_bl_status_led();

    TEST_ASSERT_EQUAL(1, blink_event);
}

TEST(main_app_led_key, GivenTickNotDivisibleBy5_WhenLEDUpdateCalled_ThenBlinkEventCleared)
{
    _100ms_Tick = 3;
    blink_event = 1;

    update_bl_status_led();

    TEST_ASSERT_EQUAL(0, blink_event);
}

/* ============================================================
 * bl_key_check
 * ============================================================ */

TEST(main_app_led_key, GivenKeyNotPressedAndTimerZero_WhenKeyCheckCalled_ThenTimeoutUnchanged)
{
    _100ms_Tick = 10;
    timeout = START_TIMEOUT;

    bl_key_pressed_ExpectAndReturn(false);

    bl_key_check();

    TEST_ASSERT_EQUAL(START_TIMEOUT, timeout);
    TEST_ASSERT_EQUAL(10, (int)_100ms_Tick);
}

TEST(main_app_led_key, GivenPressFollowedByRelease_WhenKeyCheckCalledUntilTimerWraps_ThenTimeoutExtended)
{
    _100ms_Tick = 99;
    timeout = START_TIMEOUT;

    /* ---- 1 press call (key_timer: 0 → UINT8_MAX - F_KEY_DELAY = 250) ---- */
    bl_key_pressed_ExpectAndReturn(true);

    /* ---- F_KEY_DELAY - 1 = 4 non-triggering release calls ---------------  */
    for (int i = 0; i < (int)F_KEY_DELAY - 1; i++)
    {
        bl_key_pressed_ExpectAndReturn(false);
    }

    /* ---- Final release: timer increments to UINT8_MAX → action fires ---- */
    bl_key_pressed_ExpectAndReturn(false);

    bl_key_check(); /* press */
    for (int i = 0; i < (int)F_KEY_DELAY; i++)
        bl_key_check(); /* releases */

    TEST_ASSERT_EQUAL(0, (int)_100ms_Tick);
    TEST_ASSERT_EQUAL(PUSH_BUTTON_TIMEOUT, timeout);
}
