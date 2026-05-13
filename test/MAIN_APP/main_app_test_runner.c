#include "unity/fixture/unity_fixture.h"

TEST_GROUP_RUNNER(main_app_init)
{
    RUN_TEST_CASE(main_app_init, GivenBootloader_WhenInitHardwareCalled_ThenAllPeripheralsInitialisedInOrder);
    RUN_TEST_CASE(main_app_init, GivenSysTickCallbackFires_WhenCalled_ThenTickIncremented);
    RUN_TEST_CASE(main_app_init, GivenBootloader_WhenDeinitHardwareCalled_ThenAllPeripheralsDeinitInOrder);
}

TEST_GROUP_RUNNER(main_app_led_key)
{
    RUN_TEST_CASE(main_app_led_key, GivenTickDivisibleBy5AndBlinkEventClear_WhenLEDUpdateCalled_ThenLEDToggled);
    RUN_TEST_CASE(main_app_led_key, GivenTickDivisibleBy5AndBlinkEventSet_WhenLEDUpdateCalled_ThenLEDNotToggled);
    RUN_TEST_CASE(main_app_led_key, GivenTickNotDivisibleBy5_WhenLEDUpdateCalled_ThenBlinkEventCleared);
    RUN_TEST_CASE(main_app_led_key, GivenKeyNotPressedAndTimerZero_WhenKeyCheckCalled_ThenTimeoutUnchanged);
    RUN_TEST_CASE(main_app_led_key, GivenPressFollowedByRelease_WhenKeyCheckCalledUntilTimerWraps_ThenTimeoutExtended);
}

TEST_GROUP_RUNNER(main_app_rx_fsm)
{
    RUN_TEST_CASE(main_app_rx_fsm, GivenComStart_WhenNonVersionByteReceived_ThenStateRemainsComStart);
    RUN_TEST_CASE(main_app_rx_fsm, GivenComStart_WhenGetVersionByteReceived_ThenStateBecomesComDone);
    RUN_TEST_CASE(main_app_rx_fsm, GivenComStart_WhenGetVersionByteReceived_ThenCmdIsGetVersion);
    RUN_TEST_CASE(main_app_rx_fsm, GivenComReady_WhenCmdStartReceived_ThenStateBecomesComNextByte);
    RUN_TEST_CASE(main_app_rx_fsm, GivenComReady_WhenCmdStartReceived_ThenPageTailEqualsHeaderSize);
    RUN_TEST_CASE(main_app_rx_fsm, GivenComReady_WhenCmdNextPageReceived_ThenStateBecomesComNextByte);
    RUN_TEST_CASE(main_app_rx_fsm, GivenComReady_WhenCmdNextPageReceived_ThenPageTailEqualsPageBufSize);
    RUN_TEST_CASE(main_app_rx_fsm, GivenComReady_WhenUnknownCmdReceived_ThenStateBecomesComDone);
    RUN_TEST_CASE(main_app_rx_fsm, GivenComNextByte_WhenByteReceived_ThenByteStoredAtBufPtr);
    RUN_TEST_CASE(main_app_rx_fsm, GivenComNextByte_WhenByteReceived_ThenPageTailDecremented);
    RUN_TEST_CASE(main_app_rx_fsm, GivenComNextByte_WhenByteReceivedButNotLast_ThenStateRemainsComNextByte);
    RUN_TEST_CASE(main_app_rx_fsm, GivenComNextByte_WhenLastByteReceived_ThenStateBecomesComDone);
    RUN_TEST_CASE(main_app_rx_fsm, GivenComDone_WhenAnyByteReceived_ThenStateRemainsComDone);
}

TEST_GROUP_RUNNER(main_app_com_fsm)
{
    RUN_TEST_CASE(main_app_com_fsm, GivenStateNotComDone_WhenComFsmUpdated_ThenNoCmdDispatched);
    RUN_TEST_CASE(main_app_com_fsm, GivenComDoneWithGetVersion_WhenComFsmUpdated_ThenVersionResponseSent);
    RUN_TEST_CASE(main_app_com_fsm, GivenComDoneWithReset_WhenComFsmUpdated_ThenResetAckSentAndSystemResetCalled);
    RUN_TEST_CASE(main_app_com_fsm, GivenComDoneWithUnknownCmd_WhenComFsmUpdated_ThenErrSent);
    RUN_TEST_CASE(main_app_com_fsm, GivenComDoneWithCmdStart_WhenComFsmUpdated_ThenDoStartDispatched);
    RUN_TEST_CASE(main_app_com_fsm, GivenComDoneWithCmdNextPage_WhenComFsmUpdated_ThenDoNextPageDispatched);
    RUN_TEST_CASE(main_app_com_fsm, GivenComDone_WhenComFsmUpdated_ThenTimeoutRefreshedByCommTimeout);
}

TEST_GROUP_RUNNER(main_app_start_cmd)
{
    RUN_TEST_CASE(main_app_start_cmd, GivenWrongProtocolVersion_WhenStartCmdExecuted_ThenErrStartSent);
    RUN_TEST_CASE(main_app_start_cmd, GivenWrongProductIdMSB_WhenStartCmdExecuted_ThenErrStartSent);
    RUN_TEST_CASE(main_app_start_cmd, GivenWrongProductIdLSB_WhenStartCmdExecuted_ThenErrStartSent);
    RUN_TEST_CASE(main_app_start_cmd, GivenWrongFlashPageSize_WhenStartCmdExecuted_ThenErrStartSent);
    RUN_TEST_CASE(main_app_start_cmd, GivenPageCountZero_WhenStartCmdExecuted_ThenErrStartSent);
    RUN_TEST_CASE(main_app_start_cmd, GivenPageCountExceedsAppRegion_WhenStartCmdExecuted_ThenErrStartSent);
    RUN_TEST_CASE(main_app_start_cmd, GivenAppVersionBelowPrevVersion_WhenStartCmdExecuted_ThenErrStartSent);
    RUN_TEST_CASE(main_app_start_cmd, GivenValidHeader_WhenStartCmdExecuted_ThenFlashUnlockedAndLocked);
    RUN_TEST_CASE(main_app_start_cmd, GivenValidHeader_WhenStartCmdExecuted_ThenEveryAppPageErased);
    RUN_TEST_CASE(main_app_start_cmd, GivenValidHeader_WhenStartCmdExecuted_ThenAesInitCalledWithHeaderIV);
    RUN_TEST_CASE(main_app_start_cmd, GivenValidHeader_WhenStartCmdExecuted_ThenCRCInitialisedAndPagePosSet);
    RUN_TEST_CASE(main_app_start_cmd, GivenValidHeader_WhenStartCmdExecuted_ThenOkStartSent);
}

TEST_GROUP_RUNNER(main_app_page_cmd)
{
    RUN_TEST_CASE(main_app_page_cmd, GivenPagePosBeforeAppStart_WhenNextPageCalled_ThenErrNextPageSent);
    RUN_TEST_CASE(main_app_page_cmd, GivenPagePosAtOrAfterAppEnd_WhenNextPageCalled_ThenErrNextPageSent);
    RUN_TEST_CASE(main_app_page_cmd, GivenPageRemZero_WhenNextPageCalled_ThenErrNextPageSent);
    RUN_TEST_CASE(main_app_page_cmd, GivenFirstPageNotLast_WhenNextPageCalled_ThenFirstEightBytesBufferedNotWritten);
    RUN_TEST_CASE(main_app_page_cmd, GivenFirstPageNotLast_WhenNextPageCalled_ThenOkNextPageSent);
    RUN_TEST_CASE(main_app_page_cmd, GivenFirstPageNotLast_WhenNextPageCalled_ThenPagePosAdvanced);
    RUN_TEST_CASE(main_app_page_cmd, GivenLastPageWithCRCMismatch_WhenNextPageCalled_ThenErrNextPageSent);
    RUN_TEST_CASE(main_app_page_cmd, GivenLastPageWithCRCMatch_WhenNextPageCalled_ThenFirstBytesWrittenToFlash);
    RUN_TEST_CASE(main_app_page_cmd, GivenLastPageWithCRCMatch_WhenNextPageCalled_ThenOkNextPageSent);
}

TEST_GROUP_RUNNER(main_app_exit)
{
    RUN_TEST_CASE(main_app_exit, GivenTickBelowTimeout_WhenExitCheckCalled_ThenNoAction);
    RUN_TEST_CASE(main_app_exit, GivenTickAtTimeoutAndValidApp_WhenExitCheckCalled_ThenDeinitAndJumpToApp);
    RUN_TEST_CASE(main_app_exit, GivenTickAtTimeoutAndInvalidApp_WhenExitCheckCalled_ThenSystemReset);
    RUN_TEST_CASE(main_app_exit, GivenTickAtTimeoutAndMidReceive_WhenExitCheckCalled_ThenSystemResetWithoutValidityCheck);
    RUN_TEST_CASE(main_app_exit, GivenTickExceedsTimeout_WhenExitCheckCalled_ThenConditionAlsoTriggered);
}
