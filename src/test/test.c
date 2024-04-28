#include "common/assert_handler.h"
#include "common/defines.h"
#include "drivers/io.h"
#include "drivers/led.h"
#include "drivers/mcu_init.h"
#include "drivers/uart.h"
#include "drivers/ir_remote.h"
#include "drivers/pwm.h"
#include <msp430.h>
#include "common/trace.h"

SUPPRESS_UNUSED
static void test_setup(void) { 
    mcu_init(); 
}

SUPPRESS_UNUSED
static void test_assert(void)
{
    test_setup();
    ASSERT(0);
}

SUPPRESS_UNUSED
static void test_blink_led(void) {
    test_setup();
    led_init();
    led_state_e led_state = LED_STATE_OFF;
    while (1) {
        led_state = (led_state == LED_STATE_OFF) ? LED_STATE_ON : LED_STATE_OFF;
        led_set(LED_TEST, led_state);
        BUSY_WAIT_ms(250);
    }
}


// Configure all pins as output and then toggle them in a loop. Verify with a logic analyzer.
SUPPRESS_UNUSED
static void test_launchpad_io_pins_output(void)
{
    test_setup();
    const struct io_config output_config = { .select = IO_SELECT_GPIO,
                                            .resistor = IO_RESISTOR_DISABLED,
                                            .dir = IO_DIR_OUTPUT,
                                            .out = IO_OUT_LOW };

    // Configure all pins as output
    for (io_generic_e io = IO_10; io <= IO_27; io++)
    {
        io_configure(io, &output_config);
    }
    while (1) {
        for (io_generic_e io = IO_10; io <= IO_27; io++) {
            io_set_out(io, IO_OUT_HIGH);
            BUSY_WAIT_ms(10);
            io_set_out(io, IO_OUT_LOW);
        }
    }
}

/* Configure all pins except one (pin 1.0) as input with internal pull-up resistors. Configure
 * the exception (pin 1.0) as output to control an LED. Verify by pulling each pin down in
 * increasing order with an external pull-down resistor. LED state changes when the right pin is
 * pulled down. Once all pins have been verified OK, the LED blinks repeatedly.
 *
 * Note, the pins are configured with internal pull-up resistors (instead of pull-down) because
 * some pins on the LAUNCHPAD are already pulled up by external circuitry */
SUPPRESS_UNUSED
static void test_launchpad_io_pins_input(void) {
    test_setup();
    led_init();
    const struct io_config input_config = {
        .select   = IO_SELECT_GPIO,
        .resistor = IO_RESISTOR_ENABLED,
        .dir      = IO_DIR_INPUT,
        .out      = IO_OUT_HIGH  // pull-up
    };

    // Configure all pins as input
    for (io_generic_e io = IO_10; io <= IO_27; io++) {
        io_configure(io, &input_config);
    }

    for (io_generic_e io = IO_10; io <= IO_27; io++) {
        if (io == (io_generic_e)IO_TEST_LED) {
            continue;
        }
        led_set(LED_TEST, LED_STATE_ON);
        // Wait for user to pull pin low
        while (io_get_input(io) == IO_IN_HIGH) {
            BUSY_WAIT_ms(100);  // 100 ms
        }
        led_set(LED_TEST, LED_STATE_OFF);
        // Wait for user to disconnect
        while (io_get_input(io) == IO_IN_LOW) {
            BUSY_WAIT_ms(100);  // 100 ms
        }
    }

    // Blink LED when test is done
    while (1) {
        led_set(LED_TEST, LED_STATE_ON);
        BUSY_WAIT_ms(500);
        led_set(LED_TEST, LED_STATE_OFF);
        BUSY_WAIT_ms(2000);
    }
}

SUPPRESS_UNUSED
static void io_11_isr(void)
{
    led_set(LED_TEST, LED_STATE_ON);
}

SUPPRESS_UNUSED
static void io_20_isr(void)
{
    led_set(LED_TEST, LED_STATE_OFF);
}

SUPPRESS_UNUSED
static void test_io_interrupt(void)
{
    test_setup();
    const struct io_config input_config = {
        .select = IO_SELECT_GPIO,
        .resistor = IO_RESISTOR_DISABLED,
        .dir = IO_DIR_INPUT,
        .out = IO_OUT_HIGH // pull-up
    };
    io_configure(IO_11, &input_config);
    io_configure(IO_20, &input_config);
    led_init();
    led_set(LED_TEST, LED_STATE_OFF);
    io_configure_interrupt(IO_11, IO_TRIGGER_FALLING, io_11_isr);
    io_configure_interrupt(IO_20, IO_TRIGGER_FALLING, io_20_isr);
    io_enable_interrupt(IO_11);
    io_enable_interrupt(IO_20);
    while(1);
}

SUPPRESS_UNUSED
static void test_uart(void)
{
    test_setup();
    uart_init();
    while(1){
        _putchar('U');
        _putchar('A');
        _putchar('R');
        _putchar('T');
        _putchar(' ');
        _putchar('W');
        _putchar('O');
        _putchar('R');
        _putchar('K');
        _putchar('S');
        _putchar('\n');
        BUSY_WAIT_ms(1000);
    }
}

SUPPRESS_UNUSED
static void test_trace(void)
{
    test_setup();
    trace_init();
    while(1){
        TRACE("UART working! %d",2024);
        BUSY_WAIT_ms(1000);
    }
}

SUPPRESS_UNUSED
static void test_ir_remote(void)
{
    test_setup();
    trace_init();
    ir_remote_init();
    while(1){
        TRACE("Command %s", ir_remote_cmd_to_string(ir_remote_get_cmd()));
        BUSY_WAIT_ms(250);
    }
}

SUPPRESS_UNUSED
static void test_pwm(void)
{
    test_setup();
    trace_init();
    pwm_init();
    const uint8_t duty_cycles[] = {100, 75, 50, 25, 1, 0};
    const uint16_t wait_time = 3000;
    while(1){
        for (uint8_t i = 0; i < ARRAY_SIZE(duty_cycles); i++){
            TRACE("Set duty cycle to %d for %d ms", duty_cycles[i], wait_time);
            pwm_set_duty_cycle(PWM_TB6612FNG_LEFT, duty_cycles[i]);
            pwm_set_duty_cycle(PWM_TB6612FNG_RIGHT, duty_cycles[i]);
            BUSY_WAIT_ms(3000);
        }
    }
}

int main()
{
    TEST();
    ASSERT(0);
}
