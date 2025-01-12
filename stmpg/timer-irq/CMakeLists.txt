cmake_minimum_required(VERSION 3.21)

# Build freestanding C23 firmware with ARM assembly startup code.
project(timer_irq LANGUAGES C ASM)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Reuse the STM32F407 linker script and interrupt vector table.
set(
    STM32F407_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../platform/stm32f407
)

add_executable(
    timer_irq
    ${STM32F407_DIR}/startup/vector_table.S
    startup/reset_handler.S
    src/main.c
)

# Preserve ELF metadata for inspection and debugging.
set_target_properties(
    timer_irq
    PROPERTIES
        SUFFIX ".elf"
)

target_compile_options(
    timer_irq
    PRIVATE
        -mcpu=cortex-m4
        -mthumb

        # Apply C-specific freestanding and diagnostic options.
        $<$<COMPILE_LANGUAGE:C>:-ffreestanding>
        $<$<COMPILE_LANGUAGE:C>:-ffunction-sections>
        $<$<COMPILE_LANGUAGE:C>:-fdata-sections>
        $<$<COMPILE_LANGUAGE:C>:-Wall>
        $<$<COMPILE_LANGUAGE:C>:-Wextra>
        $<$<COMPILE_LANGUAGE:C>:-Wpedantic>
)

target_link_options(
    timer_irq
    PRIVATE
        -mcpu=cortex-m4
        -mthumb
        -nostdlib
        -T${STM32F407_DIR}/linker/stm32f407vg.ld
        -Wl,--gc-sections
        -Wl,--build-id=none
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/timer_irq.map
)

# Print Flash and SRAM section sizes after every successful link.
find_program(ARM_SIZE arm-none-eabi-size REQUIRED)

add_custom_command(
    TARGET timer_irq
    POST_BUILD
    COMMAND ${ARM_SIZE} $<TARGET_FILE:timer_irq>
    VERBATIM
)
