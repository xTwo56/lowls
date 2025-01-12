.syntax unified
.cpu cortex-m4
.thumb

/* Keep Bare Boot's reset behaviour separate from the shared vector table. */
.section .text.Reset_Handler, "ax", %progbits
.align 2

.global Reset_Handler
.type Reset_Handler, %function
.thumb_func

Reset_Handler:
    /* Stop at the first instruction reached after reset. */
    b .

.size Reset_Handler, . - Reset_Handler
