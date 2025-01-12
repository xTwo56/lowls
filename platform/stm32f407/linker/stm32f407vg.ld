    /*
     * Writable initialized objects execute from SRAM, while their initial
     * values are stored in the firmware image in Flash.
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

    /* Address in Flash from which Reset_Handler copies .data. */
    _sidata = LOADADDR(.data);

    /*
     * Zero-initialized objects occupy SRAM but require no stored bytes
     * in the firmware image.
     */
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
