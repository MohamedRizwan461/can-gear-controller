#ifndef GEAR_STATE_H
#define GEAR_STATE_H

#include <stdbool.h>
#include <stdint.h>

/* Road speed is carried throughout as km/h x 10 (fixed point, 0.1 km/h). */
#define SPEED_REVERSE_LIMIT_X10  50u   /* SWR-003: 5.0 km/h */
#define SPEED_PARK_LIMIT_X10     20u   /* SWR-004: 2.0 km/h */

typedef enum {
    GEAR_PARK = 0,
    GEAR_REVERSE,
    GEAR_NEUTRAL,
    GEAR_D1,
    GEAR_D2,
    GEAR_D3,
    GEAR_D4,
    GEAR_D5,
    GEAR_D6,
    GEAR_D7,
    GEAR_D8,
    GEAR_COUNT,
    GEAR_INVALID = 0xFF
} gear_t;

typedef enum {
    FAULT_NONE            = 0u,
    FAULT_INVALID_REQUEST = (1u << 0),
    FAULT_TIMEOUT         = (1u << 1),
    FAULT_CHECKSUM        = (1u << 2)
} gear_fault_t;

typedef struct {
    gear_t   current;
    uint16_t speed_kph_x10;
    uint8_t  throttle_pct;
    bool     brake_applied;
    uint8_t  faults;        /* bitmask of gear_fault_t */
} gear_ctx_t;

void  gear_ctx_init(gear_ctx_t *ctx);
bool  gear_is_forward(gear_t g);
bool  gear_transition_allowed(const gear_ctx_t *ctx, gear_t requested);
gear_t gear_apply_request(gear_ctx_t *ctx, gear_t requested);
const char *gear_name(gear_t g);

#endif /* GEAR_STATE_H */
