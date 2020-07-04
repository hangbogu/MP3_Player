#include "gpio_isr.h"
#include "lpc40xx.h"
#include "stdint.h"
#include "stdio.h"

int check_pin_port0(void) {
  for (int i = 0; i < 32; i++) {
    if (LPC_GPIOINT->IO0IntStatR & (1 << i) || LPC_GPIOINT->IO0IntStatF & (1 << i)) {
      return i;
    }
  }
  return 0;
}

int check_pin_port2(void) {
  for (int i = 0; i < 32; i++) {
    if (LPC_GPIOINT->IO2IntStatR & (1 << i) || LPC_GPIOINT->IO2IntStatF & (1 << i)) {
      return i;
    }
  }
  return 0;
}

// Note: You may want another separate array for falling vs. rising edge callbacks
static function_pointer_t gpio0_callbacks[32];
static function_pointer_t gpio2_callbacks[32];

void gpio__attach_interrupt(uint32_t port, uint32_t pin, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  // 1) Store the callback based on the pin at gpio0_callbacks
  switch (port) {
    {
    case 0:
      gpio0_callbacks[pin] = callback;
      if (interrupt_type == GPIO_INTR__FALLING_EDGE) {
        LPC_GPIOINT->IO0IntEnF |= (1 << pin);
      } else if (interrupt_type == GPIO_INTR__RISING_EDGE) {
        LPC_GPIOINT->IO0IntEnR |= (1 << pin);
      }
      break;
    case 2:
      gpio2_callbacks[pin] = callback;
      if (interrupt_type == GPIO_INTR__FALLING_EDGE) {
        LPC_GPIOINT->IO2IntEnF |= (1 << pin);
      } else if (interrupt_type == GPIO_INTR__RISING_EDGE) {
        LPC_GPIOINT->IO2IntEnR |= (1 << pin);
      }
      break;
    default:
      break;
    }
    // 2) Configure GPIO 0 pin for rising or falling edge
  }
}

// We wrote some of the implementation for you
void gpio__interrupt_dispatcher(void) {
  // Check which pin generated the interrupt
  const int pin_that_generated_interrupt_port0 = check_pin_port0();
  const int pin_that_generated_interrupt_port2 = check_pin_port2();

  if (pin_that_generated_interrupt_port0) {

    function_pointer_t attached_user_handler_port0 = gpio0_callbacks[pin_that_generated_interrupt_port0];

    // Invoke the user registered callback, and then clear the interrupt
    attached_user_handler_port0();
    LPC_GPIOINT->IO0IntClr |= (1 << pin_that_generated_interrupt_port0);
  }

  if (pin_that_generated_interrupt_port2) {

    function_pointer_t attached_user_handler_port2 = gpio2_callbacks[pin_that_generated_interrupt_port2];

    // Invoke the user registered callback, and then clear the interrupt
    attached_user_handler_port2();
    LPC_GPIOINT->IO2IntClr |= (1 << pin_that_generated_interrupt_port2);
  }
}
