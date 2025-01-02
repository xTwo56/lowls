.syntax unified
.cpu cortex-m4
.thumb

.section .isr_vector, "a", %progbits
.align 2

.global vector_table
.type vector_table, %object

vector_table:
    .word _estack
    .word Reset_Handler

.size vector_table, . - vector_table

.section .text.Reset_Handler, "ax", %progbits
.align 2

.global Reset_Handler
.type Reset_Handler, %function
.thumb_func

Reset_Handler:
    b .

.size Reset_Handler, . - Reset_Handler
