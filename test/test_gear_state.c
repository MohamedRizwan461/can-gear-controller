#include "unity.h"
#include "gear_state.h"

static gear_ctx_t ctx;

void setUp(void)    { gear_ctx_init(&ctx); }
void tearDown(void) { }

/* SWR-001 */
void test_gear_all_gears_are_valid(void)
{
    TEST_ASSERT_EQUAL_STRING("P",  gear_name(GEAR_PARK));
    TEST_ASSERT_EQUAL_STRING("R",  gear_name(GEAR_REVERSE));
    TEST_ASSERT_EQUAL_STRING("N",  gear_name(GEAR_NEUTRAL));
    TEST_ASSERT_EQUAL_STRING("D8", gear_name(GEAR_D8));
    TEST_ASSERT_TRUE(gear_is_forward(GEAR_D1));
    TEST_ASSERT_TRUE(gear_is_forward(GEAR_D8));
    TEST_ASSERT_FALSE(gear_is_forward(GEAR_NEUTRAL));
}

/* SWR-002 */
void test_SWR002_park_to_drive_requires_brake(void)
{
    ctx.current = GEAR_PARK;
    ctx.brake_applied = false;
    TEST_ASSERT_FALSE(gear_transition_allowed(&ctx, GEAR_D1));

    ctx.brake_applied = true;
    TEST_ASSERT_TRUE(gear_transition_allowed(&ctx, GEAR_D1));
}

/* SWR-003 */
void test_SWR003_reverse_blocked_above_5kph(void)
{
    ctx.current = GEAR_NEUTRAL;
    ctx.brake_applied = true;

    ctx.speed_kph_x10 = 50u;    /* exactly 5.0 km/h - still allowed */
    TEST_ASSERT_TRUE(gear_transition_allowed(&ctx, GEAR_REVERSE));

    ctx.speed_kph_x10 = 51u;    /* just over the limit */
    TEST_ASSERT_FALSE(gear_transition_allowed(&ctx, GEAR_REVERSE));
}

/* SWR-004 */
void test_SWR004_park_blocked_above_2kph(void)
{
    ctx.current = GEAR_D1;

    ctx.speed_kph_x10 = 20u;
    TEST_ASSERT_TRUE(gear_transition_allowed(&ctx, GEAR_PARK));

    ctx.speed_kph_x10 = 21u;
    TEST_ASSERT_FALSE(gear_transition_allowed(&ctx, GEAR_PARK));
}

/* SWR-005 */
void test_SWR005_neutral_always_allowed(void)
{
    ctx.speed_kph_x10 = 2000u;   /* 200 km/h */
    ctx.brake_applied = false;

    ctx.current = GEAR_D8;
    TEST_ASSERT_TRUE(gear_transition_allowed(&ctx, GEAR_NEUTRAL));

    ctx.current = GEAR_REVERSE;
    TEST_ASSERT_TRUE(gear_transition_allowed(&ctx, GEAR_NEUTRAL));
}

/* SWR-006 */
void test_SWR006_rejected_request_sets_fault(void)
{
    ctx.current = GEAR_PARK;
    ctx.brake_applied = false;

    gear_t after = gear_apply_request(&ctx, GEAR_D1);

    TEST_ASSERT_EQUAL(GEAR_PARK, after);
    TEST_ASSERT_EQUAL(GEAR_PARK, ctx.current);
    TEST_ASSERT_BITS_HIGH(FAULT_INVALID_REQUEST, ctx.faults);
}

/* SWR-007 */
void test_SWR007_no_skip_shift(void)
{
    ctx.current = GEAR_D2;
    TEST_ASSERT_TRUE(gear_transition_allowed(&ctx, GEAR_D3));
    TEST_ASSERT_TRUE(gear_transition_allowed(&ctx, GEAR_D1));
    TEST_ASSERT_FALSE(gear_transition_allowed(&ctx, GEAR_D4));
    TEST_ASSERT_FALSE(gear_transition_allowed(&ctx, GEAR_D8));
}

void test_out_of_range_request_is_rejected(void)
{
    ctx.current = GEAR_D1;
    TEST_ASSERT_FALSE(gear_transition_allowed(&ctx, (gear_t)99));
    TEST_ASSERT_FALSE(gear_transition_allowed(&ctx, GEAR_INVALID));
}
