#include <stdint.h>

/* STM32F407 peripheral and Cortex-M4 NVIC register addresses. */
enum {
  RCC_AHB1ENR_ADDRESS = 0x40023830u,
  RCC_APB1ENR_ADDRESS = 0x40023840u,

  GPIOD_MODER_ADDRESS = 0x40020C00u,
  GPIOD_ODR_ADDRESS = 0x40020C14u,

  TIM2_CR1_ADDRESS = 0x40000000u,
  TIM2_DIER_ADDRESS = 0x4000000Cu,
  TIM2_SR_ADDRESS = 0x40000010u,
  TIM2_EGR_ADDRESS = 0x40000014u,
  TIM2_CNT_ADDRESS = 0x40000024u,
  TIM2_PSC_ADDRESS = 0x40000028u,
  TIM2_ARR_ADDRESS = 0x4000002Cu,

  NVIC_ISER0_ADDRESS = 0xE000E100u,
};

/* Peripheral clock-enable bits. */
enum {
  GPIOD_CLOCK_ENABLE = 1u << 3,
  TIM2_CLOCK_ENABLE = 1u << 0,
};

/* GPIO configuration for the Discovery board's green LED. */
enum {
  GREEN_LED_PIN = 12u,
  GREEN_LED_MASK = 1u << GREEN_LED_PIN,
  MODE_BITS_PER_PIN = 2u,
  GPIO_MODE_MASK = 0x3u,
  GPIO_OUTPUT_MODE = 0x1u,
};

/* TIM2 and NVIC control bits. */
enum {
  TIM2_COUNTER_ENABLE = 1u << 0,
  TIM2_UPDATE_INTERRUPT_ENABLE = 1u << 0,
  TIM2_UPDATE_FLAG = 1u << 0,
  TIM2_GENERATE_UPDATE = 1u << 0,

  TIM2_INTERRUPT_NUMBER = 28u,
  TIM2_NVIC_ENABLE = 1u << TIM2_INTERRUPT_NUMBER,
};

/*
 * Divide the 16 MHz timer clock by 16,000 to obtain a 1 kHz counter.
 * Five hundred counter periods then produce one interrupt every 500 ms.
 */
enum {
  TIM2_PRESCALER_VALUE = 16000u - 1u,
  TIM2_RELOAD_VALUE = 500u - 1u,
};

/* Convert a memory-mapped address into a volatile register pointer. */
static inline volatile uint32_t *register_at(uintptr_t address) {
  return (volatile uint32_t *)address;
}

/* Configure PD12 as a general-purpose output. */
static void configure_green_led(void) {
  volatile uint32_t *const gpiod_moder = register_at(GPIOD_MODER_ADDRESS);

  const uint32_t shift = GREEN_LED_PIN * MODE_BITS_PER_PIN;
  uint32_t mode = *gpiod_moder;

  mode &= ~(GPIO_MODE_MASK << shift);
  mode |= GPIO_OUTPUT_MODE << shift;

  *gpiod_moder = mode;
}

/* Handle one TIM2 update event and toggle the green LED. */
void TIM2_IRQHandler(void) {
  volatile uint32_t *const tim2_status = register_at(TIM2_SR_ADDRESS);

  volatile uint32_t *const gpiod_output = register_at(GPIOD_ODR_ADDRESS);

  if ((*tim2_status & TIM2_UPDATE_FLAG) != 0u) {
    /* Clear the handled event so it does not immediately interrupt again. */
    *tim2_status &= ~TIM2_UPDATE_FLAG;

    *gpiod_output ^= GREEN_LED_MASK;
  }
}

/* Configure TIM2 and sleep while hardware handles periodic interrupts. */
[[noreturn]]
void firmware_main(void) {
  volatile uint32_t *const rcc_ahb1enr = register_at(RCC_AHB1ENR_ADDRESS);

  volatile uint32_t *const rcc_apb1enr = register_at(RCC_APB1ENR_ADDRESS);

  volatile uint32_t *const gpiod_output = register_at(GPIOD_ODR_ADDRESS);

  volatile uint32_t *const tim2_control = register_at(TIM2_CR1_ADDRESS);

  volatile uint32_t *const tim2_interrupt_enable =
      register_at(TIM2_DIER_ADDRESS);

  volatile uint32_t *const tim2_status = register_at(TIM2_SR_ADDRESS);

  volatile uint32_t *const tim2_event = register_at(TIM2_EGR_ADDRESS);

  volatile uint32_t *const tim2_counter = register_at(TIM2_CNT_ADDRESS);

  volatile uint32_t *const tim2_prescaler = register_at(TIM2_PSC_ADDRESS);

  volatile uint32_t *const tim2_reload = register_at(TIM2_ARR_ADDRESS);

  volatile uint32_t *const nvic_enable = register_at(NVIC_ISER0_ADDRESS);

  /* Supply clocks to GPIOD and TIM2. */
  *rcc_ahb1enr |= GPIOD_CLOCK_ENABLE;
  *rcc_apb1enr |= TIM2_CLOCK_ENABLE;

  /* Complete both clock writes before accessing the peripherals. */
  const uint32_t enabled_gpio_clocks = *rcc_ahb1enr;
  const uint32_t enabled_timer_clocks = *rcc_apb1enr;
  (void)enabled_gpio_clocks;
  (void)enabled_timer_clocks;

  configure_green_led();
  *gpiod_output &= ~GREEN_LED_MASK;

  /* Configure TIM2 while its counter is stopped. */
  *tim2_control = 0u;
  *tim2_prescaler = TIM2_PRESCALER_VALUE;
  *tim2_reload = TIM2_RELOAD_VALUE;
  *tim2_counter = 0u;

  /* Load the prescaler immediately, then discard the generated event. */
  *tim2_event = TIM2_GENERATE_UPDATE;
  *tim2_status = 0u;

  /* Allow TIM2 update events to reach the Cortex-M4. */
  *tim2_interrupt_enable = TIM2_UPDATE_INTERRUPT_ENABLE;
  *nvic_enable = TIM2_NVIC_ENABLE;

  /* Start TIM2 after its interrupt route is ready. */
  *tim2_control = TIM2_COUNTER_ENABLE;

  for (;;) {
    /* Suspend instruction execution until an interrupt arrives. */
    __asm__ volatile("wfi");
  }
}
