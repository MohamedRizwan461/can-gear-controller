/* FreeRTOS wiring for the gear-state controller.
   This file is glue only - every decision it delegates is unit tested. */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "can_hal.h"
#include "can_protocol.h"
#include "gear_state.h"
#include "shift_scheduler.h"

#define STATUS_PERIOD_MS   20u   /* SWR-030 */
#define CONTROL_PERIOD_MS  10u
#define MONITOR_PERIOD_MS  50u

static gear_ctx_t   g_gear;
static can_rx_ctx_t g_rx;
static QueueHandle_t g_rx_queue;

/* Control task: runs the scheduler and the interlocks. */
static void task_control(void *arg)
{
    (void)arg;
    TickType_t last = xTaskGetTickCount();

    for (;;) {
        gear_t target = shift_compute_target(g_gear.current,
                                             g_gear.speed_kph_x10,
                                             g_gear.throttle_pct);
        if (target != g_gear.current) {
            (void)gear_apply_request(&g_gear, target);   /* SWR-031 */
        }
        vTaskDelayUntil(&last, pdMS_TO_TICKS(CONTROL_PERIOD_MS));
    }
}

/* Comms task: broadcasts status on a fixed period. */
static void task_can_tx(void *arg)
{
    (void)arg;
    TickType_t last = xTaskGetTickCount();
    uint8_t counter = 0u;

    for (;;) {
        gear_status_t st = {
            .gear          = g_gear.current,
            .speed_kph_x10 = g_gear.speed_kph_x10,
            .throttle_pct  = g_gear.throttle_pct,
            .faults        = g_gear.faults,
            .counter       = counter
        };

        can_frame_t f = { .id = CAN_ID_GEAR_STATUS, .dlc = CAN_DLC };
        if (can_encode_gear_status(&st, f.data) == CAN_OK) {
            (void)can_hal_send(&f);
        }

        counter = (uint8_t)((counter + 1u) & CAN_COUNTER_MAX);  /* SWR-021 */
        vTaskDelayUntil(&last, pdMS_TO_TICKS(STATUS_PERIOD_MS));
    }
}

/* Comms task: drains the RX mailbox. */
static void task_can_rx(void *arg)
{
    (void)arg;
    can_frame_t f;

    for (;;) {
        while (can_hal_recv(&f)) {
            if (f.id == CAN_ID_SHIFT_REQUEST) {
                gear_status_t in;
                if (can_rx_accept(&g_rx, f.data, can_hal_millis(), &in) == CAN_OK) {
                    (void)xQueueSend(g_rx_queue, &in, 0);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

/* Monitor task: bus-loss watchdog. */
static void task_monitor(void *arg)
{
    (void)arg;
    TickType_t last = xTaskGetTickCount();

    for (;;) {
        can_rx_tick(&g_rx, can_hal_millis());   /* SWR-032 */
        g_gear.faults |= g_rx.faults;
        vTaskDelayUntil(&last, pdMS_TO_TICKS(MONITOR_PERIOD_MS));
    }
}

int main(void)
{
    gear_ctx_init(&g_gear);
    can_rx_init(&g_rx);
    (void)can_hal_init(500000u);

    g_rx_queue = xQueueCreate(8, sizeof(gear_status_t));

    xTaskCreate(task_control, "control", 256, NULL, 3, NULL);
    xTaskCreate(task_can_tx,  "can_tx",  256, NULL, 2, NULL);
    xTaskCreate(task_can_rx,  "can_rx",  256, NULL, 2, NULL);
    xTaskCreate(task_monitor, "monitor", 192, NULL, 1, NULL);

    vTaskStartScheduler();
    for (;;) { }   /* scheduler should never return */
}
