#include <stddef.h>
#include <stdint.h>

/* Matches the STM32F407 GPIO register layout in memory. */
typedef struct {
  volatile uint32_t moder;     /* Offset 0x00: pin modes. */
  volatile uint32_t otyper;    /* Offset 0x04: output types. */
  volatile uint32_t ospeedr;   /* Offset 0x08: output speeds. */
  volatile uint32_t pupdr;     /* Offset 0x0C: pull resistors. */
  volatile const uint32_t idr; /* Offset 0x10: input states. */
  volatile uint32_t odr;       /* Offset 0x14: output states. */
  volatile uint32_t bsrr;      /* Offset 0x18: atomic set/reset. */
  volatile uint32_t lckr;      /* Offset 0x1C: configuration lock. */
  volatile uint32_t afrl;      /* Offset 0x20: pins 0–7 functions. */
  volatile uint32_t afrh;      /* Offset 0x24: pins 8–15 functions. */
} gpio_registers;

/* Verify that struct members match the hardware register offsets. */
_Static_assert(offsetof(gpio_registers, idr) == 0x10u,
               "GPIO IDR must be at offset 0x10");

_Static_assert(offsetof(gpio_registers, bsrr) == 0x18u,
               "GPIO BSRR must be at offset 0x18");

/* STM32F407 memory-mapped peripheral registers. */
enum {
  RCC_AHB1ENR_ADDRESS = 0x40023830u,

  GPIOA_BASE_ADDRESS = 0x40020000u,
  GPIOD_BASE_ADDRESS = 0x40020C00u,

  /* Cortex-M4 SysTick registers provide a periodic time reference. */
  SYSTICK_CTRL_ADDRESS = 0xE000E010u,
  SYSTICK_LOAD_ADDRESS = 0xE000E014u,
  SYSTICK_VAL_ADDRESS = 0xE000E018u,
};

/* RCC clock-enable bits for GPIOA and GPIOD. */
enum {
  GPIOA_CLOCK_ENABLE = 1u << 0,
  GPIOD_CLOCK_ENABLE = 1u << 3,
};

/* Discovery-board GPIO connections and register encoding. */
enum {
  USER_BUTTON_PIN = 0u,
  GREEN_LED_PIN = 12u,
  MODE_BITS_PER_PIN = 2u,
  GPIO_MODE_MASK = 0x3u,
  GPIO_INPUT_MODE = 0x0u,
  GPIO_OUTPUT_MODE = 0x1u,
  BSRR_RESET_OFFSET = 16u,
};

/* The STM32F407 starts from its 16 MHz internal oscillator. */
enum {
  PROCESSOR_CLOCK_HZ = 16000000u,
  SYSTICK_FREQUENCY_HZ = 1000u,
  SYSTICK_RELOAD_VALUE = (PROCESSOR_CLOCK_HZ / SYSTICK_FREQUENCY_HZ) - 1u,

  SYSTICK_ENABLE = 1u << 0,
  SYSTICK_PROCESSOR_CLOCK = 1u << 2,
  SYSTICK_COUNTFLAG = 1u << 16,

  DEBOUNCE_SAMPLES = 20u,
};
/* Convert a peripheral address into a volatile register pointer. */
static inline volatile uint32_t *register_at(uintptr_t address) {
  return (volatile uint32_t *)address;
}

/* Configure one GPIO pin while preserving every other pin's mode. */
static void configure_pin_mode(volatile uint32_t *moder, uint32_t pin,
                               uint32_t mode) {
  const uint32_t shift = pin * MODE_BITS_PER_PIN;
  uint32_t value = *moder;

  value &= ~(GPIO_MODE_MASK << shift);
  value |= mode << shift;

  *moder = value;
}

/* Configure SysTick to complete one countdown every millisecond. */
static void start_systick(volatile uint32_t *control, volatile uint32_t *load,
                          volatile uint32_t *value) {
  *load = SYSTICK_RELOAD_VALUE;
  *value = 0u;
  *control = SYSTICK_ENABLE | SYSTICK_PROCESSOR_CLOCK;
}

/* Wait until SysTick completes the next one-millisecond period. */
static void wait_for_systick(volatile uint32_t *control) {
  while ((*control & SYSTICK_COUNTFLAG) == 0u) {
  }
}

/* Continuously drive the LED from the user-button state. */
[[noreturn]]
void firmware_main(void) {
  volatile uint32_t *const rcc_ahb1enr = register_at(RCC_AHB1ENR_ADDRESS);

  volatile uint32_t *const systick_control = register_at(SYSTICK_CTRL_ADDRESS);

  volatile uint32_t *const systick_load = register_at(SYSTICK_LOAD_ADDRESS);

  volatile uint32_t *const systick_value = register_at(SYSTICK_VAL_ADDRESS);

  volatile gpio_registers *const gpioa =
      (volatile gpio_registers *)GPIOA_BASE_ADDRESS;

  volatile gpio_registers *const gpiod =
      (volatile gpio_registers *)GPIOD_BASE_ADDRESS;

  /* Enable both GPIO peripheral clocks without changing other clocks. */
  *rcc_ahb1enr |= GPIOA_CLOCK_ENABLE | GPIOD_CLOCK_ENABLE;

  /* Ensure the clock-enable write completes before GPIO access. */
  const uint32_t enabled_clocks = *rcc_ahb1enr;
  (void)enabled_clocks;

  configure_pin_mode(&gpioa->moder, USER_BUTTON_PIN, GPIO_INPUT_MODE);
  configure_pin_mode(&gpiod->moder, GREEN_LED_PIN, GPIO_OUTPUT_MODE);

  const uint32_t sampled_button_state =
      ((gpioa->idr & (1u << USER_BUTTON_PIN)) != 0u);

  gpiod->bsrr = 1u << GREEN_LED_PIN;

  /* Begin with both the remembered button state and LED turned off. */
  uint32_t button_was_pressed = 0u;
  uint32_t led_is_on = 0u;

  uint32_t candidate_button_state = 0u;
  uint32_t debounced_button_state = 0u;
  uint32_t stable_samples = 0u;

  /* Begin with the green LED off. */
  gpiod->bsrr = 1u << (GREEN_LED_PIN + BSRR_RESET_OFFSET);

  start_systick(systick_control, systick_load, systick_value);

  for (;;) {
    wait_for_systick(systick_control);

    /* Normalize PA0 into either zero or one. */
    const uint32_t sampled_button_state =
        ((gpioa->idr & (1u << USER_BUTTON_PIN)) != 0u);

    if (sampled_button_state != candidate_button_state) {
      /* A changed sample begins a new stability period. */
      candidate_button_state = sampled_button_state;
      stable_samples = 1u;
    } else if (stable_samples < DEBOUNCE_SAMPLES) {
      stable_samples++;
    }

    if ((stable_samples == DEBOUNCE_SAMPLES) &&
        (candidate_button_state != debounced_button_state)) {
      debounced_button_state = candidate_button_state;

      /* Toggle only after a stable released-to-pressed transition. */
      if (debounced_button_state != 0u) {
        led_is_on ^= 1u;

        if (led_is_on != 0u) {
          gpiod->bsrr = 1u << GREEN_LED_PIN;
        } else {
          gpiod->bsrr = 1u << (GREEN_LED_PIN + BSRR_RESET_OFFSET);
        }
      }
    }
  }
}
