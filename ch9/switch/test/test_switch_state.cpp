#include <unity.h>
#include "switch_state.h"

void setUp(void) {}
void tearDown(void) {}

void test_switch_off(void)
{
    TEST_ASSERT_FALSE(switch_state_update(false));
}

void test_switch_on(void)
{
    TEST_ASSERT_TRUE(switch_state_update(true));
}

void test_switch_can_turn_off_after_on(void)
{
    bool state = switch_state_update(true);
    state = switch_state_update(false);
    TEST_ASSERT_FALSE(state);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_switch_off);
    RUN_TEST(test_switch_on);
    RUN_TEST(test_switch_can_turn_off_after_on);
    return UNITY_END();
}
