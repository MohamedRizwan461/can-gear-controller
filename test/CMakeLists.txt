include(FetchContent)

FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.6.0
)
FetchContent_MakeAvailable(unity)

# Unity's generate_test_runner needs Ruby; instead each test file gets a
# tiny generated runner so CI needs nothing but a C compiler and CMake.
function(add_unity_test name source)
    set(runner "${CMAKE_CURRENT_BINARY_DIR}/${name}_runner.c")
    add_custom_command(
        OUTPUT ${runner}
        COMMAND ${CMAKE_COMMAND}
                -DSRC=${CMAKE_CURRENT_SOURCE_DIR}/${source}
                -DOUT=${runner}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/make_runner.cmake
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${source}
                ${CMAKE_CURRENT_SOURCE_DIR}/make_runner.cmake
        COMMENT "Generating runner for ${source}"
    )
    add_executable(${name} ${source} ${runner})
    target_link_libraries(${name} PRIVATE gear_logic unity)
    target_compile_options(${name} PRIVATE ${PROJECT_WARNING_FLAGS})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_unity_test(test_gear_state      test_gear_state.c)
add_unity_test(test_shift_scheduler test_shift_scheduler.c)
add_unity_test(test_can_protocol    test_can_protocol.c)
