# CAN-Based Transmission Gear-State Controller

Embedded gear-state controller for a simulated multi-speed transmission, running
FreeRTOS on an STM32 and exchanging J1939-style status and command frames over CAN
between two nodes.

Built to production-embedded practice: safety-critical shift interlocks, message
integrity (rolling counter + checksum), written software requirements with a
traceability matrix, host-runnable unit tests, and CI on every push.

---

## Why this architecture

All decision logic is **pure C with no hardware dependency** - `gear_state`,
`shift_scheduler` and `can_protocol` take values in and return values out. The
STM32 HAL sits behind a thin interface (`hal/can_hal.h`) that `main.c` wires up.

That split is what makes the unit tests runnable on a normal PC, which in turn is
what makes continuous integration possible. Hardware-dependent code is isolated to
`hal/` and `main.c`; everything that can carry a bug is tested.

```
    +-------------------+       +--------------------+
    |  main.c           |       |  FreeRTOS tasks    |
    |  (wiring only)    |-----> |  control / comms   |
    +-------------------+       +--------------------+
              |                           |
              v                           v
    +-------------------+       +--------------------+
    |  hal/can_hal.h    |       |  PURE LOGIC        |
    |  (interface)      |       |  gear_state        |
    +-------------------+       |  shift_scheduler   |
              |                 |  can_protocol      |
              v                 +--------------------+
    +-------------------+                 |
    |  can_hal_stm32.c  |                 | tested on host
    +-------------------+                 v
                                 +--------------------+
                                 |  Unity + CTest     |
                                 |  GitHub Actions CI |
                                 +--------------------+
```

---

## Hardware

| Item | Approx cost | Purpose |
|---|---|---|
| 2x STM32 Nucleo-F446RE (or F103 Blue Pill) | ~$20 ea | The two CAN nodes |
| 2x CAN transceiver (TJA1050 / MCP2551 / SN65HVD230) | ~$5 ea | CAN PHY |
| 2x 120 ohm resistors | pennies | Bus termination, one at each end |
| USB logic analyzer (8-ch, Saleae-clone) | ~$15 | Capture CAN_TX/CAN_RX timing |
| ST-Link V2 (on-board on Nucleo) | included | SWD/JTAG flashing and debug |

Node A runs the shift scheduler and broadcasts gear status.
Node B acts as the transmission ECU: validates requests, applies interlocks,
reports actual gear back.

---

## Build and test on your PC (no hardware needed)

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Unity is fetched automatically by CMake. This is exactly what CI runs.

## Build firmware for the STM32

Firmware build uses the ARM GNU toolchain and STM32Cube HAL, kept separate from the
host test build:

```bash
cmake -S . -B build-arm -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake
cmake --build build-arm
```

---

## Layout

```
src/            pure logic - fully unit tested
  gear_state.*        gear state machine + safety interlocks
  shift_scheduler.*   target-gear selection with hysteresis
  can_protocol.*      J1939-style frame encode/decode, counter + checksum
src/hal/        hardware interface + STM32 implementation
src/app/        FreeRTOS task skeleton (main.c)
test/           Unity unit tests, one file per module
docs/           software requirements + traceability matrix
.github/        CI workflow
```

## Requirements and traceability

See `docs/requirements.md`. Every requirement has an ID (SWR-xxx) and the
traceability matrix maps each one to the test case that verifies it.
