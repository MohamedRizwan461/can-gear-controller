#include "gear_state.h"

void gear_ctx_init(gear_ctx_t *ctx)
{
    if (ctx == 0) return;
    ctx->current       = GEAR_PARK;
    ctx->speed_kph_x10 = 0u;
    ctx->throttle_pct  = 0u;
    ctx->brake_applied = false;
    ctx->faults        = FAULT_NONE;
}

bool gear_is_forward(gear_t g)
{
    return (g >= GEAR_D1) && (g <= GEAR_D8);
}

/* SWR-002..SWR-007 */
bool gear_transition_allowed(const gear_ctx_t *ctx, gear_t requested)
{
    if (ctx == 0) return false;
    if (requested >= GEAR_COUNT) return false;
    if (requested == ctx->current) return true;

    /* SWR-005: neutral is always reachable. */
    if (requested == GEAR_NEUTRAL) return true;

    /* SWR-004: parking pawl protection. */
    if (requested == GEAR_PARK) {
        return ctx->speed_kph_x10 <= SPEED_PARK_LIMIT_X10;
    }

    /* SWR-003: reverse only at very low speed. */
    if (requested == GEAR_REVERSE) {
        if (ctx->speed_kph_x10 > SPEED_REVERSE_LIMIT_X10) return false;
        /* SWR-002: leaving PARK needs the brake. */
        if (ctx->current == GEAR_PARK && !ctx->brake_applied) return false;
        return true;
    }

    if (gear_is_forward(requested)) {
        /* SWR-002: leaving PARK needs the brake. */
        if (ctx->current == GEAR_PARK) {
            if (!ctx->brake_applied) return false;
            return requested == GEAR_D1;   /* pull away in first */
        }
        /* From reverse or neutral, engage first gear only. */
        if (ctx->current == GEAR_REVERSE || ctx->current == GEAR_NEUTRAL) {
            return requested == GEAR_D1;
        }
        /* SWR-007: single-step shifts between forward gears. */
        int delta = (int)requested - (int)ctx->current;
        return (delta == 1) || (delta == -1);
    }

    return false;
}

/* SWR-006: a rejected request leaves the gear untouched and raises a fault. */
gear_t gear_apply_request(gear_ctx_t *ctx, gear_t requested)
{
    if (ctx == 0) return GEAR_INVALID;

    if (gear_transition_allowed(ctx, requested)) {
        ctx->current = requested;
    } else {
        ctx->faults |= (uint8_t)FAULT_INVALID_REQUEST;
    }
    return ctx->current;
}

const char *gear_name(gear_t g)
{
    switch (g) {
        case GEAR_PARK:    return "P";
        case GEAR_REVERSE: return "R";
        case GEAR_NEUTRAL: return "N";
        case GEAR_D1:      return "D1";
        case GEAR_D2:      return "D2";
        case GEAR_D3:      return "D3";
        case GEAR_D4:      return "D4";
        case GEAR_D5:      return "D5";
        case GEAR_D6:      return "D6";
        case GEAR_D7:      return "D7";
        case GEAR_D8:      return "D8";
        default:           return "??";
    }
}
