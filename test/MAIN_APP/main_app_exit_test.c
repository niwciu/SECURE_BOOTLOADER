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
extern volatile comm_state_t com_state;
extern uint32_t timeout;
void check_bl_exit_condition(void);

TEST_GROUP(main_app_exit);

TEST_SETUP(main_app_exit)
{
    mock_bl_usart_drv_Init();
    mock_bl_flash_drv_Init();
    mock_core_drv_Init();
    mock_bl_key_drv_Init();
    mock_bl_status_led_drv_Init();
    mock_crc_api_Init();
    mock_aes_Init();

    _100ms_Tick = 0;
    timeout = START_TIMEOUT;
    com_state = COM_READY;
}

TEST_TEAR_DOWN(main_app_exit)
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
 * check_bl_exit_condition
 * ============================================================ */

TEST(main_app_exit, GivenTickBelowTimeout_WhenExitCheckCalled_ThenNoAction)
{
    _100ms_Tick = START_TIMEOUT - 1;
    timeout = START_TIMEOUT;

    /* No mock calls expected */
    check_bl_exit_condition();
}

TEST(main_app_exit, GivenTickAtTimeoutAndValidApp_WhenExitCheckCalled_ThenDeinitAndJumpToApp)
{
    _100ms_Tick = START_TIMEOUT;
    timeout = START_TIMEOUT;
    com_state = COM_READY;

    CORE_is_valid_app_ExpectAndReturn(true);
    /* deinit_hardware() call sequence */
    deinit_bl_usart_periph_Expect();
    deinit_sys_tick_Expect();
    deinit_bl_status_led_port_Expect();
    deinit_bl_key_Expect();
    deinit_bl_usart_clk_Expect();
    deinit_bl_status_led_clk_Expect();
    deinit_bl_key_clk_Expect();
    jump_to_app_Expect();

    check_bl_exit_condition();
}

TEST(main_app_exit, GivenTickAtTimeoutAndInvalidApp_WhenExitCheckCalled_ThenSystemReset)
{
    _100ms_Tick = START_TIMEOUT;
    timeout = START_TIMEOUT;
    com_state = COM_READY;

    CORE_is_valid_app_ExpectAndReturn(false);
    send_byte_Expect((uint8_t)CMD_OK | (uint8_t)CMD_RESET);
    byte_transmission_complete_ExpectAndReturn(true);
    system_reset_Expect();

    check_bl_exit_condition();
}

TEST(main_app_exit, GivenTickAtTimeoutAndMidReceive_WhenExitCheckCalled_ThenSystemResetWithoutValidityCheck)
{
    _100ms_Tick = START_TIMEOUT;
    timeout = START_TIMEOUT;
    com_state = COM_NEXT_BYTE; /* short-circuits CORE_is_valid_app() */

    /* CORE_is_valid_app must NOT be called */
    send_byte_Expect((uint8_t)CMD_OK | (uint8_t)CMD_RESET);
    byte_transmission_complete_ExpectAndReturn(true);
    system_reset_Expect();

    check_bl_exit_condition();
}

TEST(main_app_exit, GivenTickExceedsTimeout_WhenExitCheckCalled_ThenConditionAlsoTriggered)
{
    _100ms_Tick = START_TIMEOUT + 3;
    timeout = START_TIMEOUT;
    com_state = COM_READY;

    CORE_is_valid_app_ExpectAndReturn(false);
    send_byte_Expect((uint8_t)CMD_OK | (uint8_t)CMD_RESET);
    byte_transmission_complete_ExpectAndReturn(true);
    system_reset_Expect();

    check_bl_exit_condition();
}
