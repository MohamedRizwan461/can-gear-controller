cmake_minimum_required(VERSION 3.16)
project(can_gear_controller C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Strict warnings apply to OUR code only. Third-party sources fetched by
# CMake (Unity) must not inherit -Werror or an upstream warning breaks CI.
if(MSVC)
  set(PROJECT_WARNING_FLAGS /W4)
else()
  set(PROJECT_WARNING_FLAGS -Wall -Wextra -Wpedantic -Werror)
endif()

# Pure logic only - no HAL, no FreeRTOS, so it builds and runs on any host.
add_library(gear_logic STATIC
    src/gear_state.c
    src/shift_scheduler.c
    src/can_protocol.c
)
target_include_directories(gear_logic PUBLIC src)
target_compile_options(gear_logic PRIVATE ${PROJECT_WARNING_FLAGS})

enable_testing()
add_subdirectory(test)
