#include "unity/fixture/unity_fixture.h"
#include "mock_bl_usart_drv.h"
#include "mock_bl_flash_drv.h"
#include "mock_core_drv.h"
#include "mock_bl_key_drv.h"
#include "mock_bl_status_led_drv.h"
#include "mock_crc_api.h"
#include "mock_aes.h"

#include "main_app.h"

/* Internals under test */
extern volatile uint32_t _100ms_Tick;
void update_SysTick_tim(void);
void init_hardware(void);
void deinit_hardware(void);

TEST_GROUP(main_app_init);

TEST_SETUP(main_app_init)
{
    mock_bl_usart_drv_Init();
    mock_bl_flash_drv_Init();
    mock_core_drv_Init();
    mock_bl_key_drv_Init();
    mock_bl_status_led_drv_Init();
    mock_crc_api_Init();
    mock_aes_Init();

    _100ms_Tick = 0;
}

TEST_TEAR_DOWN(main_app_init)
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
 * init_hardware
 * ============================================================ */

TEST(main_app_init, GivenBootloader_WhenInitHardwareCalled_ThenAllPeripheralsInitialisedInOrder)
{
    init_bl_key_Expect();
    init_bl_status_led_Expect();
    register_data_rx_cb_ExpectAnyArgs();
    init_bl_usart_Expect();
    register_sysTick_cb_ExpectAnyArgs();
    init_sys_tick_Expect();
    CRC_hw_init_Expect();

    init_hardware();
}

/* ============================================================
 * update_SysTick_tim
 * ============================================================ */

TEST(main_app_init, GivenSysTickCallbackFires_WhenCalled_ThenTickIncremented)
{
    _100ms_Tick = 5;
    update_SysTick_tim();
    TEST_ASSERT_EQUAL(6, (int)_100ms_Tick);
}

/* ============================================================
 * deinit_hardware
 * ============================================================ */

TEST(main_app_init, GivenBootloader_WhenDeinitHardwareCalled_ThenAllPeripheralsDeinitInOrder)
{
    deinit_bl_usart_periph_Expect();
    deinit_sys_tick_Expect();
    deinit_bl_status_led_port_Expect();
    deinit_bl_key_Expect();
    deinit_bl_usart_clk_Expect();
    deinit_bl_status_led_clk_Expect();
    deinit_bl_key_clk_Expect();

    deinit_hardware();
}
