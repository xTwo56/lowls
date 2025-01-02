cmake_minimum_required(VERSION 3.20)

# Build a freestanding C++20 and assembly firmware image.
project(bare_boot LANGUAGES CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(
    LINKER_SCRIPT
    ${CMAKE_CURRENT_SOURCE_DIR}/linker/stm32f407vg.ld
)

# The initial firmware contains only the vector table and reset handler.
add_executable(
    bare_boot
    startup/vector_table.S
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

        # Apply C++-specific options only when C++ sources are introduced.
        $<$<COMPILE_LANGUAGE:CXX>:-ffreestanding>
        $<$<COMPILE_LANGUAGE:CXX>:-ffunction-sections>
        $<$<COMPILE_LANGUAGE:CXX>:-fdata-sections>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
        $<$<COMPILE_LANGUAGE:CXX>:-Wall>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
        $<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>
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
