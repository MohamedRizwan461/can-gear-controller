#include "can_protocol.h"

/* Byte layout (SWR-020):
     0    gear
     1-2  speed km/h x10, little endian
     3    throttle percent
     4    faults
     5    reserved (0)
     6    rolling counter, low nibble (SWR-021)
     7    checksum over bytes 0..6 (SWR-022)
*/

uint8_t can_checksum(const uint8_t *data, size_t len)
{
    uint8_t sum = 0u;
    if (data == 0) return 0u;
    for (size_t i = 0u; i < len; ++i) {
        sum = (uint8_t)(sum + data[i]);
    }
    return (uint8_t)(0xFFu - sum);
}

can_result_t can_encode_gear_status(const gear_status_t *in, uint8_t buf[CAN_DLC])
{
    if (in == 0 || buf == 0) return CAN_ERR_NULL;

    buf[0] = (uint8_t)in->gear;
    buf[1] = (uint8_t)(in->speed_kph_x10 & 0xFFu);
    buf[2] = (uint8_t)((in->speed_kph_x10 >> 8) & 0xFFu);
    buf[3] = in->throttle_pct;
    buf[4] = in->faults;
    buf[5] = 0u;
    buf[6] = (uint8_t)(in->counter & CAN_COUNTER_MAX);
    buf[7] = can_checksum(buf, 7u);

    return CAN_OK;
}

can_result_t can_decode_gear_status(const uint8_t buf[CAN_DLC], gear_status_t *out)
{
    if (buf == 0 || out == 0) return CAN_ERR_NULL;

    /* SWR-023 */
    if (buf[7] != can_checksum(buf, 7u)) return CAN_ERR_CHECKSUM;

    out->gear          = (gear_t)buf[0];
    out->speed_kph_x10 = (uint16_t)((uint16_t)buf[1] | ((uint16_t)buf[2] << 8));
    out->throttle_pct  = buf[3];
    out->faults        = buf[4];
    out->counter       = (uint8_t)(buf[6] & CAN_COUNTER_MAX);

    return CAN_OK;
}

void can_rx_init(can_rx_ctx_t *ctx)
{
    if (ctx == 0) return;
    ctx->last_counter = 0u;
    ctx->have_last    = false;
    ctx->last_rx_ms   = 0u;
    ctx->faults       = FAULT_NONE;
}

can_result_t can_rx_accept(can_rx_ctx_t *ctx,
                           const uint8_t buf[CAN_DLC],
                           uint32_t now_ms,
                           gear_status_t *out)
{
    if (ctx == 0 || buf == 0 || out == 0) return CAN_ERR_NULL;

    can_result_t r = can_decode_gear_status(buf, out);
    if (r == CAN_ERR_CHECKSUM) {
        ctx->faults |= (uint8_t)FAULT_CHECKSUM;
        return r;
    }
    if (r != CAN_OK) return r;

    ctx->last_rx_ms = now_ms;
    ctx->faults &= (uint8_t)~FAULT_TIMEOUT;

    /* SWR-024: counter must advance by exactly one, modulo 16. */
    if (ctx->have_last) {
        uint8_t expected = (uint8_t)((ctx->last_counter + 1u) & CAN_COUNTER_MAX);
        if (out->counter != expected) {
            ctx->last_counter = out->counter;
            return CAN_ERR_COUNTER_GAP;
        }
    }

    ctx->last_counter = out->counter;
    ctx->have_last    = true;
    return CAN_OK;
}

/* SWR-032 */
void can_rx_tick(can_rx_ctx_t *ctx, uint32_t now_ms)
{
    if (ctx == 0) return;
    if (!ctx->have_last) return;
    if ((now_ms - ctx->last_rx_ms) >= CAN_TIMEOUT_MS) {
        ctx->faults |= (uint8_t)FAULT_TIMEOUT;
    }
}
