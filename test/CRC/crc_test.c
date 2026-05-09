#include "unity/fixture/unity_fixture.h"
#include "crc_api.h"

TEST_GROUP(CRC);

TEST_SETUP(CRC)
{
}

TEST_TEAR_DOWN(CRC)
{
}

/* ============================================================
 * rev_u32
 * ============================================================ */

TEST(CRC, GivenAllZerosInput_WhenRevU32Called_ThenReturnsZero)
{
    TEST_ASSERT_EQUAL_HEX32(0x00000000UL, rev_u32(0x00000000UL));
}

TEST(CRC, GivenAllOnesInput_WhenRevU32Called_ThenReturnsAllOnes)
{
    TEST_ASSERT_EQUAL_HEX32(0xFFFFFFFFUL, rev_u32(0xFFFFFFFFUL));
}

TEST(CRC, GivenOnlyMSBSet_WhenRevU32Called_ThenOnlyLSBSet)
{
    TEST_ASSERT_EQUAL_HEX32(0x00000001UL, rev_u32(0x80000000UL));
}

TEST(CRC, GivenOnlyLSBSet_WhenRevU32Called_ThenOnlyMSBSet)
{
    TEST_ASSERT_EQUAL_HEX32(0x80000000UL, rev_u32(0x00000001UL));
}

TEST(CRC, GivenAlternatingBitPattern_WhenRevU32Called_ThenBitsFlipped)
{
    TEST_ASSERT_EQUAL_HEX32(0x55555555UL, rev_u32(0xAAAAAAAAUL));
}

TEST(CRC, GivenKnownPattern_WhenRevU32Called_ThenAllBitsAreReversed)
{
    /* 0x12345678 bit-reversed = 0x1E6A2C48 */
    TEST_ASSERT_EQUAL_HEX32(0x1E6A2C48UL, rev_u32(0x12345678UL));
}

/* ============================================================
 * CRC_hw_init
 * ============================================================ */

TEST(CRC, GivenSWCRCTarget_WhenCRCHwInitCalled_ThenFunctionCompletesWithoutError)
{
    CRC_hw_init(); /* no-op on SW targets – must not crash */
}

/* ============================================================
 * CRC_init
 * ============================================================ */

TEST(CRC, GivenCRCModule_WhenCRCInitCalled_ThenReturnsInitValue)
{
    TEST_ASSERT_EQUAL_HEX32(0xFFFFFFFFUL, CRC_init());
}

/* ============================================================
 * CRC_add_byte
 * ============================================================ */

TEST(CRC, GivenInitState_WhenAddingByteWithNoBitsSet_ThenAccumulatorMatchesTabEquivalent)
{
    /* 0x00: "byte & bit" branch never taken.
     * Initial crc MSB is set, so "if (msb)" is taken on first iterations,
     * then cleared by shifts – both taken/not-taken of "if (msb)" are covered. */
    uint32_t crc = CRC_init();
    uint8_t data[] = {0x00};
    uint32_t from_byte = CRC_result(CRC_add_byte(crc, 0x00));
    uint32_t from_tab = CRC_result(CRC_add_byte_tab(crc, data, 1));

    TEST_ASSERT_EQUAL_HEX32(from_tab, from_byte);
}

TEST(CRC, GivenInitState_WhenAddingByteWithAllBitsSet_ThenAccumulatorMatchesTabEquivalent)
{
    /* 0xFF: "byte & bit" branch taken for every bit.
     * First iteration: msb XOR'd to 0, so "if (msb)" not taken – covers that branch too. */
    uint32_t crc = CRC_init();
    uint8_t data[] = {0xFF};
    uint32_t from_byte = CRC_result(CRC_add_byte(crc, 0xFF));
    uint32_t from_tab = CRC_result(CRC_add_byte_tab(crc, data, 1));

    TEST_ASSERT_EQUAL_HEX32(from_tab, from_byte);
}

TEST(CRC, GivenInitState_WhenAddingByteWithMixedBits_ThenAccumulatorMatchesTabEquivalent)
{
    /* 0xA5 = 1010 0101: exercises both taken and not-taken of "byte & bit". */
    uint32_t crc = CRC_init();
    uint8_t data[] = {0xA5};
    uint32_t from_byte = CRC_result(CRC_add_byte(crc, 0xA5));
    uint32_t from_tab = CRC_result(CRC_add_byte_tab(crc, data, 1));

    TEST_ASSERT_EQUAL_HEX32(from_tab, from_byte);
}

/* ============================================================
 * CRC_add_byte_tab
 * ============================================================ */

TEST(CRC, GivenInitState_WhenEmptyBufferProcessed_ThenCRCRemainsUnchanged)
{
    /* length = 0: while-loop not entered */
    uint32_t crc = CRC_init();
    uint8_t data[] = {0x00};
    uint32_t result = CRC_add_byte_tab(crc, data, 0);

    TEST_ASSERT_EQUAL_HEX32(crc, result);
}

TEST(CRC, GivenInitState_WhenSingleByteBuffer_ThenMatchesCRCAddByteForSameByte)
{
    uint32_t crc = CRC_init();
    uint8_t data[] = {0x42};
    uint32_t from_tab = CRC_add_byte_tab(crc, data, 1);
    uint32_t from_single = CRC_add_byte(crc, 0x42);

    TEST_ASSERT_EQUAL_HEX32(from_single, from_tab);
}

TEST(CRC, GivenInitState_WhenStandardCheckString_ThenCRCMatchesIEEE802_3CheckValue)
{
    /* CRC-32/ISO-HDLC standard check value for "123456789" is 0xCBF43926. */
    uint8_t check[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    uint32_t crc = CRC_init();
    crc = CRC_add_byte_tab(crc, check, sizeof(check));

    TEST_ASSERT_EQUAL_HEX32(0xCBF43926UL, CRC_result(crc));
}

TEST(CRC, GivenInitState_WhenDataProcessedInTwoChunks_ThenMatchesSingleCallResult)
{
    uint8_t data[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

    uint32_t crc_single = CRC_init();
    crc_single = CRC_add_byte_tab(crc_single, data, sizeof(data));

    uint32_t crc_split = CRC_init();
    crc_split = CRC_add_byte_tab(crc_split, data, 4);
    crc_split = CRC_add_byte_tab(crc_split, data + 4, 5);

    TEST_ASSERT_EQUAL_HEX32(CRC_result(crc_single), CRC_result(crc_split));
}

/* ============================================================
 * CRC_result
 * ============================================================ */

TEST(CRC, GivenNoDataAdded_WhenCRCResultCalled_ThenReturnsZero)
{
    /* rev_u32(0xFFFFFFFF) ^ 0xFFFFFFFF = 0xFFFFFFFF ^ 0xFFFFFFFF = 0x00000000 */
    TEST_ASSERT_EQUAL_HEX32(0x00000000UL, CRC_result(CRC_init()));
}

TEST(CRC, GivenStandardCheckString_WhenCRCResultCalled_ThenReturnsIEEE802_3CheckValue)
{
    uint8_t check[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    uint32_t crc = CRC_init();
    crc = CRC_add_byte_tab(crc, check, sizeof(check));

    TEST_ASSERT_EQUAL_HEX32(0xCBF43926UL, CRC_result(crc));
}
