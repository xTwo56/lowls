/* Begin execution through the Cortex-M reset handler. */
ENTRY(Reset_Handler)

MEMORY
{
    /* STM32F407VG non-volatile program memory. */
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 1M

    /* Main SRAM region used for the stack and writable objects. */
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

/* Cortex-M loads this address as its initial stack pointer. */
_estack = ORIGIN(SRAM) + LENGTH(SRAM);

SECTIONS
{
    /* The vector table must begin at the start of Flash. */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    /* Executable instructions and read-only constants remain in Flash. */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(4);
    } > FLASH

    /*
     * Initialized writable objects run from SRAM, while their initial
     * values are stored as part of the firmware image in Flash.
     */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data.*)
        . = ALIGN(4);
        _edata = .;
    } > SRAM AT > FLASH

    /* Reset_Handler uses this as the Flash source for the .data copy. */
    _sidata = LOADADDR(.data);

    /* Zero-initialized writable objects reserve SRAM but no Flash bytes. */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > SRAM
}
