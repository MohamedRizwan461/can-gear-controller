#include "shift_scheduler.h"

/* Upshift speed for leaving gear D1..D7, in km/h x 10. */
static const uint16_t k_upshift_x10[8] = {
    120u,  /* D1 -> D2 at 12.0 km/h */
    220u,  /* D2 -> D3 */
    340u,  /* D3 -> D4 */
    460u,  /* D4 -> D5 */
    600u,  /* D5 -> D6 */
    760u,  /* D6 -> D7 */
    920u,  /* D7 -> D8 */
    0u     /* D8 is top gear */
};

uint16_t shift_upshift_threshold_x10(gear_t from)
{
    if (!gear_is_forward(from)) return 0u;
    return k_upshift_x10[from - GEAR_D1];
}

/* SWR-011: downshift point sits one full hysteresis band below the
   upshift point of the gear beneath it, so the two never coincide. */
uint16_t shift_downshift_threshold_x10(gear_t from)
{
    if (!gear_is_forward(from) || from == GEAR_D1) return 0u;
    uint16_t lower_up = k_upshift_x10[(from - GEAR_D1) - 1u];
    return (lower_up > SHIFT_HYSTERESIS_X10)
             ? (uint16_t)(lower_up - SHIFT_HYSTERESIS_X10)
             : 0u;
}

gear_t shift_compute_target(gear_t current,
                            uint16_t speed_kph_x10,
                            uint8_t throttle_pct)
{
    /* SWR-013: only forward gears are scheduled. */
    if (!gear_is_forward(current)) return current;

    /* SWR-012: hold gear under heavy throttle demand. */
    if (throttle_pct <= THROTTLE_UPSHIFT_INHIBIT_PCT && current < GEAR_D8) {
        uint16_t up = shift_upshift_threshold_x10(current);
        if (up != 0u && speed_kph_x10 >= up) {
            return (gear_t)(current + 1);
        }
    }

    if (current > GEAR_D1) {
        uint16_t down = shift_downshift_threshold_x10(current);
        if (speed_kph_x10 < down) {
            return (gear_t)(current - 1);
        }
    }

    return current;  /* SWR-013 */
}
