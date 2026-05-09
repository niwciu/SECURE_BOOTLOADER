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
extern volatile header_t header;
extern volatile uint8_t page_buf[FLASH_PAGE_SIZE];
extern uint32_t page_pos;
extern uint32_t page_rem;
extern uint32_t app_begin_word[2];
extern uint32_t crc;
void do_start(void);
void do_next_page(void);

/* Simulate a successful do_start so page_pos / page_rem / crc are set. */
static void start_transfer(uint32_t page_count, uint32_t expected_crc)
{
    header.protocol_version = PROTOCOL_VERSION;
    header.product_ID_MSB = (uint32_t)((uint64_t)DEVICE_ID >> 32);
    header.product_ID_LSB = (uint32_t)((uint64_t)DEVICE_ID & 0xFFFFFFFFUL);
    header.app_version = 1;
    header.page_count = page_count;
    header.flash_page_size = FLASH_PAGE_SIZE;
    header.crc = expected_crc;

    FLASH_unlock_Ignore();
    FLASH_erasePage_Ignore();
    FLASH_lock_Ignore();
    AES_CBC_init_ExpectAnyArgs();
    CRC_init_ExpectAndReturn(0xFFFFFFFFUL);
    send_byte_Expect((uint8_t)CMD_OK | (uint8_t)CMD_START);

    do_start();

    /* Reset mock state — do_start consumed the unlock/lock expectations */
    mock_bl_flash_drv_Verify();
    mock_bl_flash_drv_Destroy();
    mock_bl_flash_drv_Init();
    mock_crc_api_Verify();
    mock_crc_api_Destroy();
    mock_crc_api_Init();
    mock_bl_usart_drv_Verify();
    mock_bl_usart_drv_Destroy();
    mock_bl_usart_drv_Init();
    mock_aes_Verify();
    mock_aes_Destroy();
    mock_aes_Init();
}

TEST_GROUP(main_app_page_cmd);

TEST_SETUP(main_app_page_cmd)
{
    mock_bl_usart_drv_Init();
    mock_bl_flash_drv_Init();
    mock_core_drv_Init();
    mock_bl_key_drv_Init();
    mock_bl_status_led_drv_Init();
    mock_crc_api_Init();
    mock_aes_Init();

    page_pos = 0;
    page_rem = 0;
    crc = 0;
    app_begin_word[0] = 0;
    app_begin_word[1] = 0;
    memset((void *)page_buf, 0, FLASH_PAGE_SIZE);
}

TEST_TEAR_DOWN(main_app_page_cmd)
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
 * do_next_page — guard conditions
 * ============================================================ */

TEST(main_app_page_cmd, GivenPagePosBeforeAppStart_WhenNextPageCalled_ThenErrNextPageSent)
{
    page_pos = APP_START - 1;
    page_rem = 1;

    send_byte_Expect((uint8_t)CMD_ERR | (uint8_t)CMD_NEXT_PAGE);

    do_next_page();
}

TEST(main_app_page_cmd, GivenPagePosAtOrAfterAppEnd_WhenNextPageCalled_ThenErrNextPageSent)
{
    page_pos = APP_END;
    page_rem = 1;

    send_byte_Expect((uint8_t)CMD_ERR | (uint8_t)CMD_NEXT_PAGE);

    do_next_page();
}

TEST(main_app_page_cmd, GivenPageRemZero_WhenNextPageCalled_ThenErrNextPageSent)
{
    page_pos = APP_START;
    page_rem = 0;

    send_byte_Expect((uint8_t)CMD_ERR | (uint8_t)CMD_NEXT_PAGE);

    do_next_page();
}

/* ============================================================
 * do_next_page — first page (not last)
 * ============================================================ */

TEST(main_app_page_cmd, GivenFirstPageNotLast_WhenNextPageCalled_ThenFirstEightBytesBufferedNotWritten)
{
    /* Two-page transfer; this is page 1 of 2. */
    start_transfer(2, 0xDEADBEEF);

    /* Fill page_buf with known data: first 8 bytes (2 words) must NOT be
     * written to flash on this call. */
    for (size_t i = 0; i < FLASH_PAGE_SIZE; i++)
        ((uint8_t *)page_buf)[i] = (uint8_t)(i + 1);

    AES_CBC_decrypt_buffer_ExpectAnyArgs();
    CRC_add_byte_tab_ExpectAnyArgsAndReturn(0x11111111UL);
    FLASH_unlock_Expect();
    /* Only 7 double-words written (first word skipped) */
    FLASH_write_ExpectAnyArgs();
    FLASH_lock_Expect();
    send_byte_Expect((uint8_t)CMD_OK | (uint8_t)CMD_NEXT_PAGE);

    do_next_page();

    /* First 8 bytes captured in app_begin_word, not yet at flash */
    TEST_ASSERT_EQUAL_HEX32(0x04030201UL, app_begin_word[0]);
    TEST_ASSERT_EQUAL_HEX32(0x08070605UL, app_begin_word[1]);
}

TEST(main_app_page_cmd, GivenFirstPageNotLast_WhenNextPageCalled_ThenOkNextPageSent)
{
    start_transfer(2, 0xDEADBEEF);

    AES_CBC_decrypt_buffer_ExpectAnyArgs();
    CRC_add_byte_tab_ExpectAnyArgsAndReturn(0x11111111UL);
    FLASH_unlock_Expect();
    FLASH_write_ExpectAnyArgs();
    FLASH_lock_Expect();
    send_byte_Expect((uint8_t)CMD_OK | (uint8_t)CMD_NEXT_PAGE);

    do_next_page();
}

TEST(main_app_page_cmd, GivenFirstPageNotLast_WhenNextPageCalled_ThenPagePosAdvanced)
{
    start_transfer(2, 0xDEADBEEF);

    AES_CBC_decrypt_buffer_ExpectAnyArgs();
    CRC_add_byte_tab_ExpectAnyArgsAndReturn(0x11111111UL);
    FLASH_unlock_Expect();
    FLASH_write_ExpectAnyArgs();
    FLASH_lock_Expect();
    send_byte_ExpectAnyArgs();

    do_next_page();

    TEST_ASSERT_EQUAL_HEX32(APP_START + FLASH_PAGE_SIZE, page_pos);
}

/* ============================================================
 * do_next_page — last page, CRC mismatch
 * ============================================================ */

TEST(main_app_page_cmd, GivenLastPageWithCRCMismatch_WhenNextPageCalled_ThenErrNextPageSent)
{
    start_transfer(1, 0xCAFEBABEUL);

    AES_CBC_decrypt_buffer_ExpectAnyArgs();
    CRC_add_byte_tab_ExpectAnyArgsAndReturn(0x55555555UL);
    FLASH_unlock_Expect();
    FLASH_write_ExpectAnyArgs();
    CRC_result_ExpectAndReturn(0x55555555UL, 0x00000000UL); /* != header.crc */
    FLASH_lock_Expect();
    send_byte_Expect((uint8_t)CMD_ERR | (uint8_t)CMD_NEXT_PAGE);

    do_next_page();
}

/* ============================================================
 * do_next_page — last page, CRC match
 * ============================================================ */

TEST(main_app_page_cmd, GivenLastPageWithCRCMatch_WhenNextPageCalled_ThenFirstBytesWrittenToFlash)
{
    const uint32_t good_crc = 0xCBF43926UL;
    start_transfer(1, good_crc);

    AES_CBC_decrypt_buffer_ExpectAnyArgs();
    CRC_add_byte_tab_ExpectAnyArgsAndReturn(0xAAAAAAAAUL);
    FLASH_unlock_Expect();
    FLASH_write_ExpectAnyArgs();                        /* page body      */
    CRC_result_ExpectAndReturn(0xAAAAAAAAUL, good_crc); /* CRC matches     */
    FLASH_write_Expect(APP_START, app_begin_word, 1);   /* first 8 bytes  */
    FLASH_lock_Expect();
    send_byte_Expect((uint8_t)CMD_OK | (uint8_t)CMD_NEXT_PAGE);

    do_next_page();
}

TEST(main_app_page_cmd, GivenLastPageWithCRCMatch_WhenNextPageCalled_ThenOkNextPageSent)
{
    const uint32_t good_crc = 0xCBF43926UL;
    start_transfer(1, good_crc);

    AES_CBC_decrypt_buffer_ExpectAnyArgs();
    CRC_add_byte_tab_ExpectAnyArgsAndReturn(0xAAAAAAAAUL);
    FLASH_unlock_Expect();
    FLASH_write_ExpectAnyArgs();
    CRC_result_ExpectAndReturn(0xAAAAAAAAUL, good_crc);
    FLASH_write_ExpectAnyArgs();
    FLASH_lock_Expect();
    send_byte_Expect((uint8_t)CMD_OK | (uint8_t)CMD_NEXT_PAGE);

    do_next_page();
}
