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
extern volatile comm_state_t com_state;
extern volatile cmd_t cmd;
extern volatile uint8_t *buf_ptr;
extern volatile uint32_t page_tail;
void byte_rx_cb(uint8_t data);

TEST_GROUP(main_app_rx_fsm);

TEST_SETUP(main_app_rx_fsm)
{
    mock_bl_usart_drv_Init();
    mock_bl_flash_drv_Init();
    mock_core_drv_Init();
    mock_bl_key_drv_Init();
    mock_bl_status_led_drv_Init();
    mock_crc_api_Init();
    mock_aes_Init();

    com_state = COM_START;
    cmd = CMD_GET_VERSION;
    buf_ptr = NULL;
    page_tail = 0;
}

TEST_TEAR_DOWN(main_app_rx_fsm)
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
 * COM_START state
 * ============================================================ */

TEST(main_app_rx_fsm, GivenComStart_WhenNonVersionByteReceived_ThenStateRemainsComStart)
{
    com_state = COM_START;

    byte_rx_cb(0xFF);

    TEST_ASSERT_EQUAL(COM_START, com_state);
}

TEST(main_app_rx_fsm, GivenComStart_WhenGetVersionByteReceived_ThenStateBecomesComDone)
{
    com_state = COM_START;

    byte_rx_cb((uint8_t)CMD_GET_VERSION);

    TEST_ASSERT_EQUAL(COM_DONE, com_state);
}

TEST(main_app_rx_fsm, GivenComStart_WhenGetVersionByteReceived_ThenCmdIsGetVersion)
{
    com_state = COM_START;

    byte_rx_cb((uint8_t)CMD_GET_VERSION);

    TEST_ASSERT_EQUAL(CMD_GET_VERSION, cmd);
}

/* ============================================================
 * COM_READY state
 * ============================================================ */

TEST(main_app_rx_fsm, GivenComReady_WhenCmdStartReceived_ThenStateBecomesComNextByte)
{
    com_state = COM_READY;

    byte_rx_cb((uint8_t)CMD_START);

    TEST_ASSERT_EQUAL(COM_NEXT_BYTE, com_state);
}

TEST(main_app_rx_fsm, GivenComReady_WhenCmdStartReceived_ThenPageTailEqualsHeaderSize)
{
    com_state = COM_READY;

    byte_rx_cb((uint8_t)CMD_START);

    TEST_ASSERT_EQUAL(sizeof(header_t), page_tail);
}

TEST(main_app_rx_fsm, GivenComReady_WhenCmdNextPageReceived_ThenStateBecomesComNextByte)
{
    com_state = COM_READY;

    byte_rx_cb((uint8_t)CMD_NEXT_PAGE);

    TEST_ASSERT_EQUAL(COM_NEXT_BYTE, com_state);
}

TEST(main_app_rx_fsm, GivenComReady_WhenCmdNextPageReceived_ThenPageTailEqualsPageBufSize)
{
    com_state = COM_READY;

    byte_rx_cb((uint8_t)CMD_NEXT_PAGE);

    TEST_ASSERT_EQUAL(FLASH_PAGE_SIZE, page_tail);
}

TEST(main_app_rx_fsm, GivenComReady_WhenUnknownCmdReceived_ThenStateBecomesComDone)
{
    com_state = COM_READY;

    byte_rx_cb(0xEE);

    TEST_ASSERT_EQUAL(COM_DONE, com_state);
}

/* ============================================================
 * COM_NEXT_BYTE state
 * ============================================================ */

TEST(main_app_rx_fsm, GivenComNextByte_WhenByteReceived_ThenByteStoredAtBufPtr)
{
    uint8_t dst[4] = {0};
    com_state = COM_NEXT_BYTE;
    buf_ptr = dst;
    page_tail = 4;

    byte_rx_cb(0xAB);

    TEST_ASSERT_EQUAL_HEX8(0xAB, dst[0]);
}

TEST(main_app_rx_fsm, GivenComNextByte_WhenByteReceived_ThenPageTailDecremented)
{
    uint8_t dst[4] = {0};
    com_state = COM_NEXT_BYTE;
    buf_ptr = dst;
    page_tail = 4;

    byte_rx_cb(0xAB);

    TEST_ASSERT_EQUAL(3, (int)page_tail);
}

TEST(main_app_rx_fsm, GivenComNextByte_WhenByteReceivedButNotLast_ThenStateRemainsComNextByte)
{
    uint8_t dst[4] = {0};
    com_state = COM_NEXT_BYTE;
    buf_ptr = dst;
    page_tail = 4;

    byte_rx_cb(0xAB);

    TEST_ASSERT_EQUAL(COM_NEXT_BYTE, com_state);
}

TEST(main_app_rx_fsm, GivenComNextByte_WhenLastByteReceived_ThenStateBecomesComDone)
{
    uint8_t dst[1] = {0};
    com_state = COM_NEXT_BYTE;
    buf_ptr = dst;
    page_tail = 1;

    byte_rx_cb(0xCC);

    TEST_ASSERT_EQUAL(COM_DONE, com_state);
}

/* ============================================================
 * COM_DONE state
 * ============================================================ */

TEST(main_app_rx_fsm, GivenComDone_WhenAnyByteReceived_ThenStateRemainsComDone)
{
    com_state = COM_DONE;

    byte_rx_cb(0x01);

    TEST_ASSERT_EQUAL(COM_DONE, com_state);
}
