#include <time.h>
#include "TestU01.h"

FILE* f;

unsigned long devrandom() {
    unsigned long r;
    fread(&r, sizeof(unsigned long), 1, f);
    return r;
}

int main() {
    f = fopen("/dev/random", "r");
    unif01_Gen* gen = unif01_CreateExternGenBits("devrandom", devrandom);
    bbattery_BigCrush(gen);
    unif01_DeleteExternGenBits(gen);
    return 0;
}
