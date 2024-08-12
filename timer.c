#include <unistd.h>

void delay_ms(int milliseconds) {
    usleep(milliseconds * 1000); // usleep takes microseconds
}

