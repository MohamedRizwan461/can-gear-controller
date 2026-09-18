#ifndef SHIFT_SCHEDULER_H
#define SHIFT_SCHEDULER_H

#include "gear_state.h"

#define THROTTLE_UPSHIFT_INHIBIT_PCT  85u  /* SWR-012 */
#define SHIFT_HYSTERESIS_X10          30u  /* SWR-011: 3.0 km/h */

/* SWR-010/011/012/013 - pure function, no state, no hardware. */
gear_t shift_compute_target(gear_t current,
                            uint16_t speed_kph_x10,
                            uint8_t throttle_pct);

uint16_t shift_upshift_threshold_x10(gear_t from);
uint16_t shift_downshift_threshold_x10(gear_t from);

#endif /* SHIFT_SCHEDULER_H */
