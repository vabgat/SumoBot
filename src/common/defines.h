#ifndef DEFINES_H
#define DEFINES_H

#define UNUSED(x) (void)(x)
#define SUPPRESS_UNUSED __attribute__((unused))
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

// TODO: Change clock rate from 1 MHz to 16 Mhz
#define CYCLES_1MHZ (1000000u)
#define ms_TO_CYCLES(ms) ((CYCLES_1MHZ / 1000) * ms)
#define BUSY_WAIT_ms(ms) (__delay_cycles(ms_TO_CYCLES(ms)))

#endif