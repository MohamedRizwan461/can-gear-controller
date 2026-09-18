#ifndef CAN_HAL_H
#define CAN_HAL_H

#include <stdbool.h>
#include <stdint.h>

/* Thin seam between pure logic and silicon. Swap the implementation to
   retarget; the tested modules above never include this header. */

typedef struct {
    uint32_t id;
    uint8_t  dlc;
    uint8_t  data[8];
} can_frame_t;

bool     can_hal_init(uint32_t bitrate_bps);
bool     can_hal_send(const can_frame_t *frame);
bool     can_hal_recv(can_frame_t *frame);   /* non-blocking, false if empty */
uint32_t can_hal_millis(void);

#endif /* CAN_HAL_H */
