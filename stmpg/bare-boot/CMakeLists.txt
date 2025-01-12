cmake_minimum_required(VERSION 3.21)

# Build freestanding STM32 firmware with ISO C23 and ARM assembly.
project(bare_boot LANGUAGES C ASM)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Locate hardware support shared by all STM32F407 projects.
set(
    STM32F407_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../../platform/stm32f407
)

# All current projects use the STM32F407VG memory layout.
set(
    LINKER_SCRIPT
    ${STM32F407_DIR}/linker/stm32f407vg.ld
)

# Combine shared STM32 startup data with project-specific reset behaviour.
add_executable(
    bare_boot
    ${STM32F407_DIR}/startup/vector_table.S
    startup/reset_handler.S
)

set_target_properties(
    bare_boot
    PROPERTIES
        # Identify the output explicitly as an ELF firmware image.
        SUFFIX ".elf"

        # Relink whenever the memory layout changes.
        LINK_DEPENDS "${LINKER_SCRIPT}"
)

# Generate Cortex-M4 Thumb instructions.
target_compile_options(
    bare_boot
    PRIVATE
        -mcpu=cortex-m4
        -mthumb

        # Apply freestanding C diagnostics only to C source files.
        $<$<COMPILE_LANGUAGE:C>:-ffreestanding>
        $<$<COMPILE_LANGUAGE:C>:-ffunction-sections>
        $<$<COMPILE_LANGUAGE:C>:-fdata-sections>
        $<$<COMPILE_LANGUAGE:C>:-Wall>
        $<$<COMPILE_LANGUAGE:C>:-Wextra>
        $<$<COMPILE_LANGUAGE:C>:-Wpedantic>
)

target_link_options(
    bare_boot
    PRIVATE
        # Keep the linker configured for Cortex-M4 Thumb code.
        -mcpu=cortex-m4
        -mthumb

        # Use our startup code and memory layout instead of a standard runtime.
        -nostdlib
        -T${LINKER_SCRIPT}

        # Remove unused sections while preserving explicitly kept sections.
        -Wl,--gc-sections

        # Prevent metadata from appearing before the vector table.
        -Wl,--build-id=none

        # Record the final placement of sections and symbols.
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/bare_boot.map
)

# Display the firmware section sizes after every successful link.
find_program(ARM_SIZE arm-none-eabi-size REQUIRED)

add_custom_command(
    TARGET bare_boot
    POST_BUILD
    COMMAND "${ARM_SIZE}" "$<TARGET_FILE:bare_boot>"
    VERBATIM
)
