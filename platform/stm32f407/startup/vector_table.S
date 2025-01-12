.syntax unified
.cpu cortex-m4
.thumb

/* These symbols are defined outside this assembly file. */
.extern _estack
.extern Reset_Handler

/* Store the initial stack pointer and reset-handler address in Flash. */
.section .isr_vector, "a", %progbits
.align 2

.global vector_table
.type vector_table, %object

vector_table:
    .word _estack
    .word Reset_Handler

.size vector_table, . - vector_table
