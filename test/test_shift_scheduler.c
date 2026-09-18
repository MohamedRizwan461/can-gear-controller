#include "unity.h"
#include "shift_scheduler.h"

void setUp(void)    { }
void tearDown(void) { }

/* SWR-010 */
void test_SWR010_target_gear_from_speed(void)
{
    /* Below the D1 upshift point we stay in D1. */
    TEST_ASSERT_EQUAL(GEAR_D1, shift_compute_target(GEAR_D1, 100u, 30u));
    /* At the threshold we upshift. */
    TEST_ASSERT_EQUAL(GEAR_D2, shift_compute_target(GEAR_D1, 120u, 30u));
}

/* SWR-011 - the band that stops gear hunting. */
void test_SWR011_hysteresis_prevents_hunting(void)
{
    uint16_t up_from_d1   = shift_upshift_threshold_x10(GEAR_D1);
    uint16_t down_from_d2 = shift_downshift_threshold_x10(GEAR_D2);

    TEST_ASSERT_TRUE(up_from_d1 > down_from_d2);
    TEST_ASSERT_TRUE((up_from_d1 - down_from_d2) >= SHIFT_HYSTERESIS_X10);

    /* Sitting inside the band, neither gear wants to change. */
    uint16_t inside = (uint16_t)(down_from_d2 + 5u);
    TEST_ASSERT_EQUAL(GEAR_D2, shift_compute_target(GEAR_D2, inside, 30u));
    TEST_ASSERT_EQUAL(GEAR_D1, shift_compute_target(GEAR_D1, inside - 20u, 30u));
}

/* SWR-012 */
void test_SWR012_no_upshift_at_high_throttle(void)
{
    TEST_ASSERT_EQUAL(GEAR_D2, shift_compute_target(GEAR_D1, 500u, 50u));
    TEST_ASSERT_EQUAL(GEAR_D1, shift_compute_target(GEAR_D1, 500u, 95u));
}

/* SWR-013 */
void test_SWR013_no_change_when_stable(void)
{
    TEST_ASSERT_EQUAL(GEAR_D3, shift_compute_target(GEAR_D3, 250u, 40u));
    /* Non-forward gears are never scheduled. */
    TEST_ASSERT_EQUAL(GEAR_PARK,    shift_compute_target(GEAR_PARK, 0u, 0u));
    TEST_ASSERT_EQUAL(GEAR_NEUTRAL, shift_compute_target(GEAR_NEUTRAL, 300u, 20u));
    TEST_ASSERT_EQUAL(GEAR_REVERSE, shift_compute_target(GEAR_REVERSE, 10u, 20u));
}

void test_top_gear_does_not_upshift(void)
{
    TEST_ASSERT_EQUAL(GEAR_D8, shift_compute_target(GEAR_D8, 5000u, 10u));
}

void test_downshift_at_low_speed(void)
{
    TEST_ASSERT_EQUAL(GEAR_D2, shift_compute_target(GEAR_D3, 0u, 10u));
}
