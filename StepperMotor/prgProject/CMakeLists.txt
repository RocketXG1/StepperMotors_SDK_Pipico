cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(template_motion_project C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(template_motion_project
    main.cpp

    ../FunctionBlocks/HomeSequenceFunctionBlock.cpp

    ../src/srcStepper/AdvanceStepperAxis.cpp
    ../src/srcStepper/StepperPioPulseEngine.cpp

    ../src/srcMotion/TrapezoidProfile.cpp
    ../src/srcMotion/MotionPlanner.cpp
    ../src/srcMotion/AdvanceMotionController.cpp
)

target_include_directories(template_motion_project PRIVATE
    ../FunctionBlocks
    ../lib/objStepper
    ../lib/objMotion
)

pico_generate_pio_header(template_motion_project
    ${CMAKE_CURRENT_LIST_DIR}/../src/srcStepper/stepper.pio
)

target_link_libraries(template_motion_project
    pico_stdlib
    hardware_pio
    hardware_clocks
)

pico_enable_stdio_usb(template_motion_project 1)
pico_enable_stdio_uart(template_motion_project 0)

pico_add_extra_outputs(template_motion_project)
