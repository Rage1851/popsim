#include <stdlib.h>
#include <time.h>
#include "TestU01.h"

int main() {
    srand(time(NULL));
    unif01_Gen* gen = unif01_CreateExternGenBits("rand", rand);
    bbattery_SmallCrush(gen);
    unif01_DeleteExternGenBits(gen);
    return 0;
}
