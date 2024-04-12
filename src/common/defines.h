#ifndef DEFINES_H
#define DEFINES_H

#define UNUSED(x) (void)(x)
#define SUPPRESS_UNUSED __attribute__((unused))
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define INTERRUPT_FUNCTION(vector) void __attribute__((interrupt(vector)))
#define IS_ODD(x) (x & 1)

#define CYCLES_1MHZ (1000000u)
#define CYCLES_16MHZ (16u * CYCLES_1MHZ)
#define CYCLES_PER_MS (CYCLES_16MHZ / 1000u)
#define ms_TO_CYCLES(ms) (CYCLES_PER_MS * ms)
#define BUSY_WAIT_ms(ms) (__delay_cycles(ms_TO_CYCLES(ms)))

#define MCLK CYCLES_16MHZ
#define SMCLK MCLK

#define TIMER_INPUT_DIVIDER_ID_3 (8u)
#define TIMER_MC_MASK (0x0030)

#endif