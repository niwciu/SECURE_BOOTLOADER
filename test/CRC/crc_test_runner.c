#include "unity/fixture/unity_fixture.h"

TEST_GROUP_RUNNER(CRC)
{
    /* rev_u32 */
    RUN_TEST_CASE(CRC, GivenAllZerosInput_WhenRevU32Called_ThenReturnsZero);
    RUN_TEST_CASE(CRC, GivenAllOnesInput_WhenRevU32Called_ThenReturnsAllOnes);
    RUN_TEST_CASE(CRC, GivenOnlyMSBSet_WhenRevU32Called_ThenOnlyLSBSet);
    RUN_TEST_CASE(CRC, GivenOnlyLSBSet_WhenRevU32Called_ThenOnlyMSBSet);
    RUN_TEST_CASE(CRC, GivenAlternatingBitPattern_WhenRevU32Called_ThenBitsFlipped);
    RUN_TEST_CASE(CRC, GivenKnownPattern_WhenRevU32Called_ThenAllBitsAreReversed);

    /* CRC_hw_init */
    RUN_TEST_CASE(CRC, GivenSWCRCTarget_WhenCRCHwInitCalled_ThenFunctionCompletesWithoutError);

    /* CRC_init */
    RUN_TEST_CASE(CRC, GivenCRCModule_WhenCRCInitCalled_ThenReturnsInitValue);

    /* CRC_add_byte */
    RUN_TEST_CASE(CRC, GivenInitState_WhenAddingByteWithNoBitsSet_ThenAccumulatorMatchesTabEquivalent);
    RUN_TEST_CASE(CRC, GivenInitState_WhenAddingByteWithAllBitsSet_ThenAccumulatorMatchesTabEquivalent);
    RUN_TEST_CASE(CRC, GivenInitState_WhenAddingByteWithMixedBits_ThenAccumulatorMatchesTabEquivalent);

    /* CRC_add_byte_tab */
    RUN_TEST_CASE(CRC, GivenInitState_WhenEmptyBufferProcessed_ThenCRCRemainsUnchanged);
    RUN_TEST_CASE(CRC, GivenInitState_WhenSingleByteBuffer_ThenMatchesCRCAddByteForSameByte);
    RUN_TEST_CASE(CRC, GivenInitState_WhenStandardCheckString_ThenCRCMatchesIEEE802_3CheckValue);
    RUN_TEST_CASE(CRC, GivenInitState_WhenDataProcessedInTwoChunks_ThenMatchesSingleCallResult);

    /* CRC_result */
    RUN_TEST_CASE(CRC, GivenNoDataAdded_WhenCRCResultCalled_ThenReturnsZero);
    RUN_TEST_CASE(CRC, GivenStandardCheckString_WhenCRCResultCalled_ThenReturnsIEEE802_3CheckValue);
}
