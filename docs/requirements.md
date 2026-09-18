# Software Requirements - Gear-State Controller

Document ID: SRS-GSC-001
Status: Baseline

Each requirement is verifiable and maps to at least one automated test.
Requirement IDs are referenced in test names so traceability survives refactoring.

---

## 1. Gear state machine

| ID | Requirement | Rationale |
|---|---|---|
| SWR-001 | The controller shall support gears PARK, REVERSE, NEUTRAL and forward gears D1 through D8. | Defines the state space. |
| SWR-002 | The controller shall reject a transition from PARK to any forward or reverse gear unless the brake is applied. | Prevents unintended vehicle motion. |
| SWR-003 | The controller shall reject a transition to REVERSE when road speed exceeds 5.0 km/h. | Prevents driveline damage. |
| SWR-004 | The controller shall reject a transition to PARK when road speed exceeds 2.0 km/h. | Prevents parking-pawl damage. |
| SWR-005 | The controller shall permit a transition to NEUTRAL from any gear at any speed. | Neutral is the safe state. |
| SWR-006 | On rejecting a transition the controller shall retain the current gear and set the fault flag FAULT_INVALID_REQUEST. | No silent failure. |
| SWR-007 | Forward gear changes shall be limited to one gear per request. | Prevents skip-shift driveline shock. |

## 2. Shift scheduling

| ID | Requirement | Rationale |
|---|---|---|
| SWR-010 | The scheduler shall select a target forward gear from road speed and throttle position. | Core function. |
| SWR-011 | The scheduler shall apply hysteresis of at least 3.0 km/h between the upshift and downshift thresholds of any gear pair. | Prevents gear hunting. |
| SWR-012 | The scheduler shall not command an upshift when throttle exceeds 85 percent. | Preserves acceleration authority. |
| SWR-013 | The scheduler shall return the current gear unchanged when no shift condition is met. | Deterministic output. |

## 3. CAN message integrity

| ID | Requirement | Rationale |
|---|---|---|
| SWR-020 | Gear status shall be transmitted in an 8-byte frame on CAN ID 0x18F00500. | J1939-style PDU. |
| SWR-021 | Every transmitted frame shall carry a rolling counter incrementing 0-15 and wrapping. | Detects stale or lost frames. |
| SWR-022 | Every transmitted frame shall carry a checksum over the preceding 7 bytes. | Detects corruption. |
| SWR-023 | The receiver shall reject a frame whose checksum does not match. | Data integrity. |
| SWR-024 | The receiver shall flag a frame whose rolling counter does not advance by exactly 1 from the previous accepted frame. | Detects dropped frames. |
| SWR-025 | Encode followed by decode shall reproduce the original signal values exactly. | Round-trip integrity. |

## 4. Timing

| ID | Requirement | Rationale |
|---|---|---|
| SWR-030 | Gear status shall be broadcast every 20 ms +/- 2 ms. | Bus scheduling. |
| SWR-031 | A received shift request shall be acted upon within 50 ms. | Driver responsiveness. |
| SWR-032 | If no valid status frame is received for 250 ms the receiver shall enter FAULT_TIMEOUT. | Fail-safe on bus loss. |

---

## Traceability matrix

| Requirement | Verified by | File |
|---|---|---|
| SWR-001 | test_gear_all_gears_are_valid | test/test_gear_state.c |
| SWR-002 | test_SWR002_park_to_drive_requires_brake | test/test_gear_state.c |
| SWR-003 | test_SWR003_reverse_blocked_above_5kph | test/test_gear_state.c |
| SWR-004 | test_SWR004_park_blocked_above_2kph | test/test_gear_state.c |
| SWR-005 | test_SWR005_neutral_always_allowed | test/test_gear_state.c |
| SWR-006 | test_SWR006_rejected_request_sets_fault | test/test_gear_state.c |
| SWR-007 | test_SWR007_no_skip_shift | test/test_gear_state.c |
| SWR-010 | test_SWR010_target_gear_from_speed | test/test_shift_scheduler.c |
| SWR-011 | test_SWR011_hysteresis_prevents_hunting | test/test_shift_scheduler.c |
| SWR-012 | test_SWR012_no_upshift_at_high_throttle | test/test_shift_scheduler.c |
| SWR-013 | test_SWR013_no_change_when_stable | test/test_shift_scheduler.c |
| SWR-020 | test_SWR020_frame_is_eight_bytes | test/test_can_protocol.c |
| SWR-021 | test_SWR021_counter_wraps_at_15 | test/test_can_protocol.c |
| SWR-022 | test_SWR022_checksum_is_present | test/test_can_protocol.c |
| SWR-023 | test_SWR023_bad_checksum_rejected | test/test_can_protocol.c |
| SWR-024 | test_SWR024_counter_gap_flagged | test/test_can_protocol.c |
| SWR-025 | test_SWR025_encode_decode_roundtrip | test/test_can_protocol.c |
| SWR-030 | Bench: logic-analyzer capture of CAN_TX period | docs/bench_results.md |
| SWR-031 | Bench: request-to-actuation latency capture | docs/bench_results.md |
| SWR-032 | test_SWR032_timeout_sets_fault | test/test_can_protocol.c |
