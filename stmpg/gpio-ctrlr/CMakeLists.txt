cmake_minimum_required(VERSION 3.21)

# Build the GPIO firmware using strict C23 and ARM assembly.
project(gpio_controller LANGUAGES C ASM)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Locate support shared by all STM32F407 projects.
set(
    STM32F407_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../platform/stm32f407
)

set(
    LINKER_SCRIPT
    ${STM32F407_DIR}/linker/stm32f407vg.ld
)

# Combine shared boot data with project-specific startup and application code.
add_executable(
    gpio_controller
    ${STM32F407_DIR}/startup/vector_table.S
    startup/reset_handler.S
    src/main.c
)

set_target_properties(
    gpio_controller
    PROPERTIES
        # Identify the output as an ELF firmware image.
        SUFFIX ".elf"

        # Relink whenever the STM32 memory layout changes.
        LINK_DEPENDS "${LINKER_SCRIPT}"
)

# Generate code for the STM32F407's Cortex-M4 processor.
target_compile_options(
    gpio_controller
    PRIVATE
        -mcpu=cortex-m4
        -mthumb

        # Apply freestanding diagnostics only to C source files.
        $<$<COMPILE_LANGUAGE:C>:-ffreestanding>
        $<$<COMPILE_LANGUAGE:C>:-ffunction-sections>
        $<$<COMPILE_LANGUAGE:C>:-fdata-sections>
        $<$<COMPILE_LANGUAGE:C>:-Wall>
        $<$<COMPILE_LANGUAGE:C>:-Wextra>
        $<$<COMPILE_LANGUAGE:C>:-Wpedantic>
)

target_link_options(
    gpio_controller
    PRIVATE
        -mcpu=cortex-m4
        -mthumb

        # Use our startup code and linker script instead of a hosted runtime.
        -nostdlib
        -T${LINKER_SCRIPT}

        # Remove unreachable sections without discarding the vector table.
        -Wl,--gc-sections

        # Keep the vector table as the first bytes in Flash.
        -Wl,--build-id=none

        # Record the final section and symbol placement.
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/gpio_controller.map
)

# Print the firmware's loaded section sizes after linking.
find_program(ARM_SIZE arm-none-eabi-size REQUIRED)

add_custom_command(
    TARGET gpio_controller
    POST_BUILD
    COMMAND "${ARM_SIZE}" "$<TARGET_FILE:gpio_controller>"
    VERBATIM
)
