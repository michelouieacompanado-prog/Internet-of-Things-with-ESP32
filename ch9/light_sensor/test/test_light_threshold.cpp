#include <unity.h>
#include "light_threshold.h"

void setUp(void) {}
void tearDown(void) {}

void test_zero_lux_is_low(void)
{
    TEST_ASSERT_TRUE(is_low_light(0));
}

void test_29_lux_is_low(void)
{
    TEST_ASSERT_TRUE(is_low_light(29));
}

void test_30_lux_is_not_low(void)
{
    TEST_ASSERT_FALSE(is_low_light(30));
}

void test_31_lux_is_not_low(void)
{
    TEST_ASSERT_FALSE(is_low_light(31));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_zero_lux_is_low);
    RUN_TEST(test_29_lux_is_low);
    RUN_TEST(test_30_lux_is_not_low);
    RUN_TEST(test_31_lux_is_not_low);
    return UNITY_END();
}
