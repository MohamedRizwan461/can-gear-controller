# Bench Verification Results

Requirements SWR-030 and SWR-031 are timing requirements and cannot be verified
on the host - they need the logic analyzer on real hardware.

## SWR-030 - status broadcast period 20 ms +/- 2 ms

- Setup: logic analyzer channel 0 on CAN_TX (PA12), 2 MHz sample rate.
- Method: capture 100 consecutive frames, measure start-of-frame to
  start-of-frame interval, record min / max / mean.
- Result: _to be filled in after capture_
- Screenshot: `docs/img/swr030_capture.png`

## SWR-031 - shift request acted on within 50 ms

- Setup: channel 0 CAN_TX (node A), channel 1 CAN_RX (node B), channel 2 on a
  GPIO toggled at the top of `task_control`.
- Method: inject a shift request, measure request frame end to the GPIO edge
  where the new gear is applied. 50 repetitions.
- Result: _to be filled in after capture_
- Screenshot: `docs/img/swr031_capture.png`

## Notes

Record the bus bitrate, the FreeRTOS tick rate, and the build's optimisation
level alongside every capture - timing numbers are meaningless without them.
