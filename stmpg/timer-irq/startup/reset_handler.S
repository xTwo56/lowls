.syntax unified
.cpu cortex-m4
.thumb

/* The linker script defines the boundaries of the C data sections. */
.extern _sidata
.extern _sdata
.extern _edata
.extern _sbss
.extern _ebss

/* The C application provides this entry point. */
.extern firmware_main

.section .text.Reset_Handler, "ax", %progbits
.align 2

.global Reset_Handler
.type Reset_Handler, %function
.thumb_func

Reset_Handler:
    /*
     * Copy initialized writable objects from their Flash load addresses
     * to their runtime locations in SRAM.
     */
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata

1:
    cmp r1, r2
    bcs 2f
    ldr r3, [r0], #4
    str r3, [r1], #4
    b 1b

    /* Clear every byte reserved for zero-initialized objects. */
2:
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0

3:
    cmp r1, r2
    bcs 4f
    str r3, [r1], #4
    b 3b

    /* Enter C only after its required memory state is established. */
4:
    bl firmware_main

    /* Stop here if the non-returning C entry point returns unexpectedly. */
5:
    b 5b

.size Reset_Handler, . - Reset_Handler
