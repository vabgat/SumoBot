#include "common/assert_handler.h"
#include "common/defines.h"
#include "drivers/uart.h"
#include "external/printf/printf.h"
#include <msp430.h>

/* The TI compiler provides intrinsic support for calling a specific opcode, which means
 * you can write __op_code(0x4343) to trigger a software breakpoint (when LAUNCHPAD FET
 * debugger is attached). MSP430-GCC does not have this intrinsic, but 0x4343 corresponds
 * to assembly instruction "CLR.B R3". */
#define BREAKPOINT __asm volatile("CLR.B R3");

// Text + Program counter + Null termination
#define ASSERT_STRING_MAX_SIZE (15u + 6u + 1u)

#define GPIO_OUTPUT_LOW(port, bit)                                                                 \
    do {                                                                                           \
        P##port##SEL &= ~(BIT##bit);                                                               \
        P##port##SEL2 &= ~(BIT##bit);                                                              \
        P##port##DIR |= BIT##bit;                                                                  \
        P##port##REN &= ~(BIT##bit);                                                               \
        P##port##OUT &= ~(BIT##bit);                                                               \
    } while (0)

static void assert_trace(uint16_t program_counter) {
    // UART Tx
    P1SEL |= BIT1;
    P1SEL2 |= BIT1;
    uart_init_assert();
    char assert_string[ASSERT_STRING_MAX_SIZE];
    snprintf(assert_string, sizeof(assert_string), "ASSERT 0x%x\n", program_counter);
    uart_trace_assert(assert_string);
}

static void assert_blink_led(void) {
    // Configure TEST LED pin on LAUNCHPAD
    GPIO_OUTPUT_LOW(1, 0);

    // Confgure TEST LED pin on NSUMO
    GPIO_OUTPUT_LOW(2, 6);

    while (1) {
        // Blink LED on both targets in case the wrong target was flashed
        P1OUT ^= BIT0;
        P2OUT ^= BIT6;
        BUSY_WAIT_ms(250);
    };
}

static void assert_stop_motors(void) {
    GPIO_OUTPUT_LOW(1, 6);  // Left PWM (Launchpad)
    GPIO_OUTPUT_LOW(2, 1);  // Left CC1 (Launchpad)
    GPIO_OUTPUT_LOW(2, 2);  // Left CC2 (Launchpad)
    GPIO_OUTPUT_LOW(2, 4);  // Left CC2 (NSUMO)
    GPIO_OUTPUT_LOW(2, 5);  // Left CC1 (NSUMO)
    GPIO_OUTPUT_LOW(2, 7);  // Right CC2 (NSUMO)
    GPIO_OUTPUT_LOW(3, 7);  // Right CC1 (NSUMO)
    GPIO_OUTPUT_LOW(3, 5);  // Left PWM (NSUMO)
    GPIO_OUTPUT_LOW(3, 6);  // Right PWM (NSUMO)
}
/* Minimize code dependency in this function to reduce the risk of accidently calling
 * a function with an assert in it, which would cause the assert_handler to be called
 * recursively until stack overflow. */
void assert_handler(uint16_t program_counter) {
    assert_stop_motors();
    BREAKPOINT
    assert_trace(program_counter);
    assert_blink_led();
}