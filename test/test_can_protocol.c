#include <string.h>
#include "unity.h"
#include "can_protocol.h"

static gear_status_t st;
static uint8_t buf[CAN_DLC];
static can_rx_ctx_t rx;

void setUp(void)
{
    memset(&st, 0, sizeof(st));
    memset(buf, 0, sizeof(buf));
    can_rx_init(&rx);

    st.gear          = GEAR_D3;
    st.speed_kph_x10 = 1234u;
    st.throttle_pct  = 42u;
    st.faults        = FAULT_NONE;
    st.counter       = 0u;
}

void tearDown(void) { }

/* SWR-020 */
void test_SWR020_frame_is_eight_bytes(void)
{
    TEST_ASSERT_EQUAL(8u, CAN_DLC);
    TEST_ASSERT_EQUAL(CAN_OK, can_encode_gear_status(&st, buf));
    TEST_ASSERT_EQUAL_HEX32(0x18F00500u, CAN_ID_GEAR_STATUS);
}

/* SWR-021 */
void test_SWR021_counter_wraps_at_15(void)
{
    st.counter = 15u;
    TEST_ASSERT_EQUAL(CAN_OK, can_encode_gear_status(&st, buf));
    TEST_ASSERT_EQUAL(15u, buf[6] & CAN_COUNTER_MAX);

    st.counter = 16u;   /* must be masked back to 0 */
    TEST_ASSERT_EQUAL(CAN_OK, can_encode_gear_status(&st, buf));
    TEST_ASSERT_EQUAL(0u, buf[6] & CAN_COUNTER_MAX);
}

/* SWR-022 */
void test_SWR022_checksum_is_present(void)
{
    TEST_ASSERT_EQUAL(CAN_OK, can_encode_gear_status(&st, buf));
    TEST_ASSERT_EQUAL(can_checksum(buf, 7u), buf[7]);
}

/* SWR-023 */
void test_SWR023_bad_checksum_rejected(void)
{
    gear_status_t out;
    TEST_ASSERT_EQUAL(CAN_OK, can_encode_gear_status(&st, buf));

    buf[7] = (uint8_t)(buf[7] ^ 0xFFu);
    TEST_ASSERT_EQUAL(CAN_ERR_CHECKSUM, can_decode_gear_status(buf, &out));

    TEST_ASSERT_EQUAL(CAN_ERR_CHECKSUM, can_rx_accept(&rx, buf, 100u, &out));
    TEST_ASSERT_BITS_HIGH(FAULT_CHECKSUM, rx.faults);
}

/* SWR-024 */
void test_SWR024_counter_gap_flagged(void)
{
    gear_status_t out;

    st.counter = 3u;
    TEST_ASSERT_EQUAL(CAN_OK, can_encode_gear_status(&st, buf));
    TEST_ASSERT_EQUAL(CAN_OK, can_rx_accept(&rx, buf, 10u, &out));

    st.counter = 4u;    /* correct successor */
    TEST_ASSERT_EQUAL(CAN_OK, can_encode_gear_status(&st, buf));
    TEST_ASSERT_EQUAL(CAN_OK, can_rx_accept(&rx, buf, 30u, &out));

    st.counter = 7u;    /* two frames missing */
    TEST_ASSERT_EQUAL(CAN_OK, can_encode_gear_status(&st, buf));
    TEST_ASSERT_EQUAL(CAN_ERR_COUNTER_GAP, can_rx_accept(&rx, buf, 50u, &out));
}

void test_counter_wrap_is_not_a_gap(void)
{
    gear_status_t out;

    st.counter = 15u;
    TEST_ASSERT_EQUAL(CAN_OK, can_encode_gear_status(&st, buf));
    TEST_ASSERT_EQUAL(CAN_OK, can_rx_accept(&rx, buf, 10u, &out));

    st.counter = 0u;    /* wraps to zero - legal */
    TEST_ASSERT_EQUAL(CAN_OK, can_encode_gear_status(&st, buf));
    TEST_ASSERT_EQUAL(CAN_OK, can_rx_accept(&rx, buf, 30u, &out));
}

/* SWR-025 */
void test_SWR025_encode_decode_roundtrip(void)
{
    gear_status_t out;
    memset(&out, 0, sizeof(out));

    TEST_ASSERT_EQUAL(CAN_OK, can_encode_gear_status(&st, buf));
    TEST_ASSERT_EQUAL(CAN_OK, can_decode_gear_status(buf, &out));

    TEST_ASSERT_EQUAL(st.gear,          out.gear);
    TEST_ASSERT_EQUAL(st.speed_kph_x10, out.speed_kph_x10);
    TEST_ASSERT_EQUAL(st.throttle_pct,  out.throttle_pct);
    TEST_ASSERT_EQUAL(st.faults,        out.faults);
    TEST_ASSERT_EQUAL(st.counter,       out.counter);
}

void test_speed_roundtrip_at_boundaries(void)
{
    gear_status_t out;
    const uint16_t cases[] = { 0u, 1u, 255u, 256u, 1000u, 65535u };

    for (unsigned i = 0; i < sizeof(cases)/sizeof(cases[0]); ++i) {
        st.speed_kph_x10 = cases[i];
        TEST_ASSERT_EQUAL(CAN_OK, can_encode_gear_status(&st, buf));
        TEST_ASSERT_EQUAL(CAN_OK, can_decode_gear_status(buf, &out));
        TEST_ASSERT_EQUAL(cases[i], out.speed_kph_x10);
    }
}

/* SWR-032 */
void test_SWR032_timeout_sets_fault(void)
{
    gear_status_t out;

    TEST_ASSERT_EQUAL(CAN_OK, can_encode_gear_status(&st, buf));
    TEST_ASSERT_EQUAL(CAN_OK, can_rx_accept(&rx, buf, 1000u, &out));

    can_rx_tick(&rx, 1000u + CAN_TIMEOUT_MS - 1u);
    TEST_ASSERT_BITS_LOW(FAULT_TIMEOUT, rx.faults);

    can_rx_tick(&rx, 1000u + CAN_TIMEOUT_MS);
    TEST_ASSERT_BITS_HIGH(FAULT_TIMEOUT, rx.faults);
}

void test_null_arguments_are_rejected(void)
{
    gear_status_t out;
    TEST_ASSERT_EQUAL(CAN_ERR_NULL, can_encode_gear_status(NULL, buf));
    TEST_ASSERT_EQUAL(CAN_ERR_NULL, can_decode_gear_status(NULL, &out));
    TEST_ASSERT_EQUAL(CAN_ERR_NULL, can_decode_gear_status(buf, NULL));
}
