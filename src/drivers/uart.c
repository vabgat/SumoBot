#include "drivers/uart.h"
#include "common/assert_handler.h"
#include "common/defines.h"
#include "common/ring_buffer.h"
#include <assert.h>
#include <msp430.h>
#include <stdint.h>

#define UART_BUFFER_SIZE (16)
static uint8_t            buffer[UART_BUFFER_SIZE];
static struct ring_buffer tx_buffer = {.buffer = buffer, .size = sizeof(buffer)};
/* Calculate the integer and fractional part of the divisor
 * N = (Clock source / Desired baudrate)
 * for Low-Frequency Baud Rate Mode.
 *
 * These are used to configure the desired baudrate
 *
 * For information on the corresponding transmission error
 * for common values, please refer to the table provided in
 * the family user guide (SLAU144K)*/

#define SMCLK (16000000u)
#define BRCLK (SMCLK)
#define UART_BAUD_RATE (115200u)
static_assert(UART_BAUD_RATE < (BRCLK / 3.0f),
              "Baudrate must be smaller than 1/3 of input clock in Low-frequency Mode");
#define UART_DIVISOR ((float)BRCLK / UART_BAUD_RATE)
static_assert(UART_DIVISOR < 0xFFFFu, "Sanity check divisor fits in 16-bit");

#define UART_DIVISOR_INT_16_BIT ((uint16_t)UART_DIVISOR)  // controls the prescalar
#define UART_DIVISOR_INT_LOW_BYTE (UART_DIVISOR_INT_16_BIT & 0xFF);
#define UART_DIVISOR_INT_HIGH_BYTE (UART_DIVISOR_INT_16_BIT >> 8);
#define UART_DIVISOR_FRACTIONAL (UART_DIVISOR - UART_DIVISOR_INT_16_BIT)  // controls the modulator
// prescalar brings baud rate closer to desire and modulator brings more closer to it.
#define UART_UCBRS ((uint8_t)(8 * UART_DIVISOR_FRACTIONAL))
#define UART_UCBRF (0)
#define UART_UC0S16 (0)
static_assert(UART_UCBRS < 8, "Sanity check second modulation stage value fits in 3-bit");

static inline void uart_tx_enable_interrupt(void) { UC0IE |= UCA0TXIE; }

static inline void uart_tx_disable_interrupt(void) { UC0IE &= ~UCA0TXIE; }

static inline void uart_tx_clear_interrupt(void) { IFG2 &= ~UCA0TXIFG; }

static void uart_tx_start(void) {
    if (!(ring_buffer_empty(&tx_buffer))) {
        UCA0TXBUF = ring_buffer_peek(&tx_buffer);
    }
}

INTERRUPT_FUNCTION(USCIAB0TX_VECTOR) isr_uart_tx() {
    ASSERT_INTERRUPT(!ring_buffer_empty(&tx_buffer));

    // Remove the transmitted data byte from the buffer
    ring_buffer_get(&tx_buffer);

    // Clear interrupt here to avoid accidently clearing interrupt for next transmission
    uart_tx_clear_interrupt();

    if (!ring_buffer_empty(&tx_buffer)) {
        uart_tx_start();
    }
}

static void uart_configure(void) {

    /* Reset module. It stays in reset until cleared. The module should be in reset
     *  condition while configuring according to the user guide SLAU144K. */
    UCA0CTL1 &= UCSWRST;
    // TODO: configure

    /* Use default (data word length 8 bits, 1 stop bit, no parity bits)
     * [ Start (1 bit) | Data (8 bits) | Stop (1 bit) ] */
    UCA0CTL0 = 0;

    // Set SMCLK as clock source which is Sub system clock
    UCA0CTL1 |= UCSSEL_2;

    // Set clock prescaler to the integer part of divisor N.
    UCA0BR0 = UART_DIVISOR_INT_LOW_BYTE;
    UCA0BR1 = UART_DIVISOR_INT_HIGH_BYTE;

    /* Set modulation to account for fractional part of divisor N.
     * UCA0MCTL = [ UCBRF (4 bits) | UCBRS (3 bits) | UC0516 (1 bit) ] */
    UCA0MCTL = ((UART_UCBRF << 4) + (UART_UCBRS << 1) + UART_UC0S16);

    // Clear reset to release the module for operation
    UCA0CTL1 &= ~UCSWRST;
}

static bool initialized = false;
void        uart_init(void) {
    ASSERT(!initialized);

    uart_configure();

    // Interrupt triggers when TX buffer is empty, which it is after boot, so clear it here.
    uart_tx_clear_interrupt();

    // Enable TX interrupt
    uart_tx_enable_interrupt();

    initialized = true;
}

// mpaland/printf needs this to be named _putchar
void _putchar(char c) {
    // Some terminals expect line feed (\r) after carriage return (\n) for proper new line
    if (c == '\n') {
        _putchar('\r');
    }
    // Poll if full
    while (ring_buffer_full(&tx_buffer)) {
    }

    uart_tx_disable_interrupt();
    const bool tx_ongoing = !ring_buffer_empty(&tx_buffer);
    ring_buffer_put(&tx_buffer, c);
    if (!tx_ongoing) {
        uart_tx_start();
    }
    uart_tx_enable_interrupt();
}

void uart_init_assert(void) {
    uart_tx_disable_interrupt();
    uart_configure();
}

void uart_putchar_polling(char c) {
    if (c == '\n') {
        uart_putchar_polling('\r');
    }
    while (!(IFG2 & UCA0TXIFG)) {
    }
    UCA0TXBUF = c;
}

void uart_trace_assert(const char* string) {
    int i = 0;
    while (string[i] != '\0') {
        uart_putchar_polling(string[i]);
        i++;
    }
}
