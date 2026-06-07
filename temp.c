#include <stdbool.h>
#include <stdint.h>

int32_t temp(int32_t a, int32_t b) {
    return a + b;
}

int32_t someFunc(int32_t abs, int32_t ok) {
    return temp(1 + 2 + temp(1, 2), 0) + 10;
}

int32_t main() {
    int32_t a = 10 + 50 + 5.0;
    a = 1;
    temp(0, 0);
    int32_t c = 10 + temp(1 + 2, 2 + 3) + 1 + someFunc(a + 1, 0);
}

