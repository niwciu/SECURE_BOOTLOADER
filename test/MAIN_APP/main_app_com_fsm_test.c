#include "unity/fixture/unity_fixture.h"
#include <string.h>
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
extern volatile cmd_t cmd;
extern volatile header_t header;
extern uint32_t timeout;
extern uint32_t page_pos;
extern uint32_t page_rem;
void update_com_fsm(void);

TEST_GROUP(main_app_com_fsm);

TEST_SETUP(main_app_com_fsm)
{
    mock_bl_usart_drv_Init();
    mock_bl_flash_drv_Init();
    mock_core_drv_Init();
    mock_bl_key_drv_Init();
    mock_bl_status_led_drv_Init();
    mock_crc_api_Init();
    mock_aes_Init();

    _100ms_Tick = 0;
    com_state = COM_READY;
    cmd = CMD_GET_VERSION;
    timeout = START_TIMEOUT;
    page_pos = 0;
    page_rem = 0;
    memset((void *)&header, 0, sizeof(header));
}

TEST_TEAR_DOWN(main_app_com_fsm)
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
 * update_com_fsm — no dispatch when not COM_DONE
 * ============================================================ */

TEST(main_app_com_fsm, GivenStateNotComDone_WhenComFsmUpdated_ThenNoCmdDispatched)
{
    com_state = COM_READY;

    /* No mock expectations — send_byte must NOT be called */
    update_com_fsm();
}

/* ============================================================
 * CMD_GET_VERSION dispatch
 * ============================================================ */

TEST(main_app_com_fsm, GivenComDoneWithGetVersion_WhenComFsmUpdated_ThenVersionResponseSent)
{
    com_state = COM_DONE;
    cmd = CMD_GET_VERSION;

    send_byte_Expect((uint8_t)CMD_OK | (uint8_t)CMD_GET_VERSION);
    send_byte_table_ExpectAnyArgs(); /* protocol version  */
    send_byte_table_ExpectAnyArgs(); /* device ID         */
    send_byte_table_ExpectAnyArgs(); /* flash page size   */

    update_com_fsm();

    TEST_ASSERT_EQUAL(COM_READY, com_state);
}

/* ============================================================
 * CMD_RESET dispatch  (also tests do_reset internals)
 * ============================================================ */

TEST(main_app_com_fsm, GivenComDoneWithReset_WhenComFsmUpdated_ThenResetAckSentAndSystemResetCalled)
{
    com_state = COM_DONE;
    cmd = CMD_RESET;

    send_byte_Expect((uint8_t)CMD_OK | (uint8_t)CMD_RESET);
    byte_transmission_complete_ExpectAndReturn(true);
    system_reset_Expect();

    update_com_fsm();
}

/* ============================================================
 * Unknown command in dispatch switch
 * ============================================================ */

TEST(main_app_com_fsm, GivenComDoneWithUnknownCmd_WhenComFsmUpdated_ThenErrSent)
{
    com_state = COM_DONE;
    cmd = CMD_OK; /* not handled in the dispatch switch */

    send_byte_Expect((uint8_t)CMD_ERR);

    update_com_fsm();

    TEST_ASSERT_EQUAL(COM_READY, com_state);
}

/* ============================================================
 * CMD_START dispatch
 * ============================================================ */

TEST(main_app_com_fsm, GivenComDoneWithCmdStart_WhenComFsmUpdated_ThenDoStartDispatched)
{
    header.protocol_version = PROTOCOL_VERSION;
    header.product_ID_MSB = (uint32_t)((uint64_t)DEVICE_ID >> 32);
    header.product_ID_LSB = (uint32_t)((uint64_t)DEVICE_ID & 0xFFFFFFFFUL);
    header.flash_page_size = FLASH_PAGE_SIZE;
    header.page_count = 1;

    com_state = COM_DONE;
    cmd = CMD_START;

    FLASH_unlock_Ignore();
    FLASH_erasePage_Ignore();
    FLASH_lock_Ignore();
    AES_CBC_init_ExpectAnyArgs();
    CRC_init_ExpectAndReturn(0xFFFFFFFFUL);
    send_byte_Expect((uint8_t)CMD_OK | (uint8_t)CMD_START);

    update_com_fsm();

    TEST_ASSERT_EQUAL(COM_READY, (int)com_state);
}

/* ============================================================
 * CMD_NEXT_PAGE dispatch
 * ============================================================ */

TEST(main_app_com_fsm, GivenComDoneWithCmdNextPage_WhenComFsmUpdated_ThenDoNextPageDispatched)
{
    /* page_rem == 0 triggers the guard in do_next_page → immediate ERR reply */
    page_pos = APP_START;
    page_rem = 0;
    com_state = COM_DONE;
    cmd = CMD_NEXT_PAGE;

    send_byte_Expect((uint8_t)CMD_ERR | (uint8_t)CMD_NEXT_PAGE);

    update_com_fsm();

    TEST_ASSERT_EQUAL(COM_READY, (int)com_state);
}

/* ============================================================
 * Timeout refresh on each dispatch
 * ============================================================ */

TEST(main_app_com_fsm, GivenComDone_WhenComFsmUpdated_ThenTimeoutRefreshedByCommTimeout)
{
    com_state = COM_DONE;
    cmd = CMD_GET_VERSION;
    _100ms_Tick = 7;

    send_byte_Expect((uint8_t)CMD_OK | (uint8_t)CMD_GET_VERSION);
    send_byte_table_ExpectAnyArgs();
    send_byte_table_ExpectAnyArgs();
    send_byte_table_ExpectAnyArgs();

    update_com_fsm();

    TEST_ASSERT_EQUAL(7 + COMM_TIMEOUT, timeout);
}
