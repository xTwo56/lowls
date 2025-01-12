.syntax unified
.cpu cortex-m4
.thumb

/* The C application provides this entry-point symbol. */
.extern firmware_main

/* Enter the C application after the processor loads the initial stack. */
.section .text.Reset_Handler, "ax", %progbits
.align 2

.global Reset_Handler
.type Reset_Handler, %function
.thumb_func

Reset_Handler:
    bl firmware_main

/* Remain here if the non-returning C entry point unexpectedly returns. */
1:
    b 1b

.size Reset_Handler, . - Reset_Handler
