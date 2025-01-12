.syntax unified
.cpu cortex-m4
.thumb

/* The linker script and project startup code provide these symbols. */
.extern _estack
.extern Reset_Handler

/* Unimplemented exceptions and interrupts stop here for debugging. */
.section .text.Default_Handler, "ax", %progbits
.align 2

.global Default_Handler
.type Default_Handler, %function
.thumb_func

Default_Handler:
1:
    b 1b

.size Default_Handler, . - Default_Handler

/*
 * Create a weak handler that applications may replace with a strong
 * function of the same name.
 */
.macro default_handler handler_name
    .weak \handler_name
    .thumb_set \handler_name, Default_Handler
.endm

/* Cortex-M exception handlers default to the infinite loop. */
default_handler NMI_Handler
default_handler HardFault_Handler
default_handler MemManage_Handler
default_handler BusFault_Handler
default_handler UsageFault_Handler
default_handler SVC_Handler
default_handler DebugMon_Handler
default_handler PendSV_Handler
default_handler SysTick_Handler

/* STM32F407 interrupt handlers through TIM2. */
default_handler WWDG_IRQHandler
default_handler PVD_IRQHandler
default_handler TAMP_STAMP_IRQHandler
default_handler RTC_WKUP_IRQHandler
default_handler FLASH_IRQHandler
default_handler RCC_IRQHandler
default_handler EXTI0_IRQHandler
default_handler EXTI1_IRQHandler
default_handler EXTI2_IRQHandler
default_handler EXTI3_IRQHandler
default_handler EXTI4_IRQHandler
default_handler DMA1_Stream0_IRQHandler
default_handler DMA1_Stream1_IRQHandler
default_handler DMA1_Stream2_IRQHandler
default_handler DMA1_Stream3_IRQHandler
default_handler DMA1_Stream4_IRQHandler
default_handler DMA1_Stream5_IRQHandler
default_handler DMA1_Stream6_IRQHandler
default_handler ADC_IRQHandler
default_handler CAN1_TX_IRQHandler
default_handler CAN1_RX0_IRQHandler
default_handler CAN1_RX1_IRQHandler
default_handler CAN1_SCE_IRQHandler
default_handler EXTI9_5_IRQHandler
default_handler TIM1_BRK_TIM9_IRQHandler
default_handler TIM1_UP_TIM10_IRQHandler
default_handler TIM1_TRG_COM_TIM11_IRQHandler
default_handler TIM1_CC_IRQHandler
default_handler TIM2_IRQHandler

/* Hardware uses each entry's fixed position to select a handler. */
.section .isr_vector, "a", %progbits
.align 2

.global vector_table
.type vector_table, %object

vector_table:
    /* Cortex-M core vectors: indexes 0–15. */
    .word _estack
    .word Reset_Handler
    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVC_Handler
    .word DebugMon_Handler
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler

    /* STM32 external interrupts begin at vector index 16. */
    .word WWDG_IRQHandler
    .word PVD_IRQHandler
    .word TAMP_STAMP_IRQHandler
    .word RTC_WKUP_IRQHandler
    .word FLASH_IRQHandler
    .word RCC_IRQHandler
    .word EXTI0_IRQHandler
    .word EXTI1_IRQHandler
    .word EXTI2_IRQHandler
    .word EXTI3_IRQHandler
    .word EXTI4_IRQHandler
    .word DMA1_Stream0_IRQHandler
    .word DMA1_Stream1_IRQHandler
    .word DMA1_Stream2_IRQHandler
    .word DMA1_Stream3_IRQHandler
    .word DMA1_Stream4_IRQHandler
    .word DMA1_Stream5_IRQHandler
    .word DMA1_Stream6_IRQHandler
    .word ADC_IRQHandler
    .word CAN1_TX_IRQHandler
    .word CAN1_RX0_IRQHandler
    .word CAN1_RX1_IRQHandler
    .word CAN1_SCE_IRQHandler
    .word EXTI9_5_IRQHandler
    .word TIM1_BRK_TIM9_IRQHandler
    .word TIM1_UP_TIM10_IRQHandler
    .word TIM1_TRG_COM_TIM11_IRQHandler
    .word TIM1_CC_IRQHandler
    .word TIM2_IRQHandler

.size vector_table, . - vector_table
