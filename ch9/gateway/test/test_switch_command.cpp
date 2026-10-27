#include <unity.h>
#include "switch_command.h"

void test_switch_on_returns_one(void)
{
    TEST_ASSERT_EQUAL_UINT8(1, switch_command_value(true));
}

void test_switch_off_returns_zero(void)
{
    TEST_ASSERT_EQUAL_UINT8(0, switch_command_value(false));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_switch_on_returns_one);
    RUN_TEST(test_switch_off_returns_zero);
    return UNITY_END();
}
