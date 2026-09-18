#ifndef CAN_PROTOCOL_H
#define CAN_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "gear_state.h"

#define CAN_ID_GEAR_STATUS   0x18F00500u  /* SWR-020, J1939-style 29-bit */
#define CAN_ID_SHIFT_REQUEST 0x18F00600u
#define CAN_DLC              8u
#define CAN_COUNTER_MAX      15u          /* SWR-021 */
#define CAN_TIMEOUT_MS       250u         /* SWR-032 */

typedef struct {
    gear_t   gear;
    uint16_t speed_kph_x10;
    uint8_t  throttle_pct;
    uint8_t  faults;
    uint8_t  counter;    /* 0..15, rolling */
} gear_status_t;

typedef enum {
    CAN_OK = 0,
    CAN_ERR_NULL,
    CAN_ERR_CHECKSUM,     /* SWR-023 */
    CAN_ERR_COUNTER_GAP   /* SWR-024 */
} can_result_t;

/* Receiver state for counter and timeout monitoring. */
typedef struct {
    uint8_t  last_counter;
    bool     have_last;
    uint32_t last_rx_ms;
    uint8_t  faults;
} can_rx_ctx_t;

uint8_t      can_checksum(const uint8_t *data, size_t len);
can_result_t can_encode_gear_status(const gear_status_t *in, uint8_t buf[CAN_DLC]);
can_result_t can_decode_gear_status(const uint8_t buf[CAN_DLC], gear_status_t *out);

void         can_rx_init(can_rx_ctx_t *ctx);
can_result_t can_rx_accept(can_rx_ctx_t *ctx,
                           const uint8_t buf[CAN_DLC],
                           uint32_t now_ms,
                           gear_status_t *out);
void         can_rx_tick(can_rx_ctx_t *ctx, uint32_t now_ms);  /* SWR-032 */

#endif /* CAN_PROTOCOL_H */
