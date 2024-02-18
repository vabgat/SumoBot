#ifndef ASSERT_HANDLER_H

// Assert implementation suitable for a microcontroller

#define ASSERT(expression)                                                                         \
    do {                                                                                           \
        if (!(expression)) {                                                                       \
            assert_handler();                                                                      \
        }                                                                                          \
    } while (0)

#define ASSERT_INTERRUPT(expression)                                                               \
    do {                                                                                           \
        if (!(expression)) {                                                                       \
            while (1)                                                                              \
                ;                                                                                  \
        }                                                                                          \
    } while (0)

void assert_handler(void);

#endif