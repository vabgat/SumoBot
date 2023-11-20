#include "app/drive.h"
#include "app/enemy.h"
#include "drivers/i2c.h"
#include "drivers/uart.h"

int main(void) {
    // TODO: these are just to test build
    uart_init();
    i2c_init();
    enemy_init();
    drive_init();
    return 0;
}