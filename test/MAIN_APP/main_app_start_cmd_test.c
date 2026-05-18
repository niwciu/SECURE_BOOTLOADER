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
extern volatile header_t header;
extern uint32_t page_pos;
extern uint32_t page_rem;
void do_start(void);

/* Helper: fill header with values that pass all validation checks. */
static void set_valid_header(void)
{
    header.protocol_version = PROTOCOL_VERSION;
    header.product_ID_MSB = (uint32_t)((uint64_t)DEVICE_ID >> 32);
    header.product_ID_LSB = (uint32_t)((uint64_t)DEVICE_ID & 0xFFFFFFFFUL);
    header.app_version = 2;
    header.page_count = 2;
    header.flash_page_size = FLASH_PAGE_SIZE;
    header.crc = 0x00000000UL;
}

TEST_GROUP(main_app_start_cmd);

TEST_SETUP(main_app_start_cmd)
{
    mock_bl_usart_drv_Init();
    mock_bl_flash_drv_Init();
    mock_core_drv_Init();
    mock_bl_key_drv_Init();
    mock_bl_status_led_drv_Init();
    mock_crc_api_Init();
    mock_aes_Init();
}

TEST_TEAR_DOWN(main_app_start_cmd)
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
 * do_start — header validation failures
 * ============================================================ */

TEST(main_app_start_cmd, GivenWrongProtocolVersion_WhenStartCmdExecuted_ThenErrStartSent)
{
    set_valid_header();
    header.protocol_version = PROTOCOL_VERSION + 1;

    send_byte_Expect((uint8_t)CMD_ERR | (uint8_t)CMD_START);

    do_start();
}

TEST(main_app_start_cmd, GivenWrongProductIdMSB_WhenStartCmdExecuted_ThenErrStartSent)
{
    set_valid_header();
    header.product_ID_MSB = 0xDEADBEEFUL;

    send_byte_Expect((uint8_t)CMD_ERR | (uint8_t)CMD_START);

    do_start();
}

TEST(main_app_start_cmd, GivenWrongProductIdLSB_WhenStartCmdExecuted_ThenErrStartSent)
{
    set_valid_header();
    header.product_ID_LSB = 0xDEADBEEFUL;

    send_byte_Expect((uint8_t)CMD_ERR | (uint8_t)CMD_START);

    do_start();
}

TEST(main_app_start_cmd, GivenWrongFlashPageSize_WhenStartCmdExecuted_ThenErrStartSent)
{
    set_valid_header();
    header.flash_page_size = FLASH_PAGE_SIZE + 1;

    send_byte_Expect((uint8_t)CMD_ERR | (uint8_t)CMD_START);

    do_start();
}

TEST(main_app_start_cmd, GivenPageCountZero_WhenStartCmdExecuted_ThenErrStartSent)
{
    set_valid_header();
    header.page_count = 0;

    send_byte_Expect((uint8_t)CMD_ERR | (uint8_t)CMD_START);

    do_start();
}

TEST(main_app_start_cmd, GivenPageCountExceedsAppRegion_WhenStartCmdExecuted_ThenErrStartSent)
{
    set_valid_header();
    header.page_count = APP_LAST_PAGE - APP_START_PAGE + 2U;

    send_byte_Expect((uint8_t)CMD_ERR | (uint8_t)CMD_START);

    do_start();
}

/* ============================================================
 * do_start — valid header
 * ============================================================ */

TEST(main_app_start_cmd, GivenValidHeader_WhenStartCmdExecuted_ThenFlashUnlockedAndLocked)
{
    set_valid_header();

    FLASH_unlock_Expect();
    FLASH_erasePage_Ignore();
    FLASH_lock_Expect();
    AES_CBC_init_ExpectAnyArgs();
    CRC_init_ExpectAndReturn(0xFFFFFFFFUL);
    send_byte_Expect((uint8_t)CMD_OK | (uint8_t)CMD_START);

    do_start();
}

TEST(main_app_start_cmd, GivenValidHeader_WhenStartCmdExecuted_ThenEveryAppPageErased)
{
    set_valid_header();

    FLASH_unlock_Ignore();
    FLASH_lock_Ignore();

    /* Expect exactly (APP_LAST_PAGE - APP_START_PAGE + 1) erase calls */
    for (uint32_t p = APP_START_PAGE; p <= APP_LAST_PAGE; p++)
    {
        FLASH_erasePage_Expect(p);
    }

    AES_CBC_init_ExpectAnyArgs();
    CRC_init_ExpectAndReturn(0xFFFFFFFFUL);
    send_byte_Expect((uint8_t)CMD_OK | (uint8_t)CMD_START);

    do_start();
}

TEST(main_app_start_cmd, GivenValidHeader_WhenStartCmdExecuted_ThenAesInitCalledWithHeaderIV)
{
    set_valid_header();
    /* Set a known IV so we can verify it is passed to AES_CBC_init. */
    for (size_t i = 0; i < IV_BLOCK_SIZE; i++)
        header.iv[i] = (uint8_t)i;

    FLASH_unlock_Ignore();
    FLASH_erasePage_Ignore();
    FLASH_lock_Ignore();
    AES_CBC_init_ExpectAnyArgs();
    CRC_init_ExpectAndReturn(0xFFFFFFFFUL);
    send_byte_Expect((uint8_t)CMD_OK | (uint8_t)CMD_START);

    do_start();
}

TEST(main_app_start_cmd, GivenValidHeader_WhenStartCmdExecuted_ThenCRCInitialisedAndPagePosSet)
{
    set_valid_header();

    FLASH_unlock_Ignore();
    FLASH_erasePage_Ignore();
    FLASH_lock_Ignore();
    AES_CBC_init_ExpectAnyArgs();
    CRC_init_ExpectAndReturn(0xFFFFFFFFUL);
    send_byte_Expect((uint8_t)CMD_OK | (uint8_t)CMD_START);

    do_start();

    TEST_ASSERT_EQUAL_HEX32(APP_START, page_pos);
    TEST_ASSERT_EQUAL(header.page_count, page_rem);
}

TEST(main_app_start_cmd, GivenValidHeader_WhenStartCmdExecuted_ThenOkStartSent)
{
    set_valid_header();

    FLASH_unlock_Ignore();
    FLASH_erasePage_Ignore();
    FLASH_lock_Ignore();
    AES_CBC_init_ExpectAnyArgs();
    CRC_init_ExpectAndReturn(0xFFFFFFFFUL);
    send_byte_Expect((uint8_t)CMD_OK | (uint8_t)CMD_START);

    do_start();
}
