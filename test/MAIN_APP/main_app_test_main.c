#include "unity/fixture/unity_fixture.h"

static void run_all_tests(void);

int main(int argc, const char **argv)
{
    int test_resoult = UnityMain(argc, argv, run_all_tests);

    return test_resoult;
}

static void run_all_tests(void)
{
    RUN_TEST_GROUP(main_app_init);
    RUN_TEST_GROUP(main_app_led_key);
    RUN_TEST_GROUP(main_app_rx_fsm);
    RUN_TEST_GROUP(main_app_com_fsm);
    RUN_TEST_GROUP(main_app_start_cmd);
    RUN_TEST_GROUP(main_app_page_cmd);
    RUN_TEST_GROUP(main_app_exit);
}
