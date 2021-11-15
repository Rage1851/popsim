#include <time.h>
#include "TestU01.h"

unsigned long long u, v, w;

unsigned long long ran(){
    v = 4101842887655102017LL;
    w = 1;

    u = u * 2862933555777941757LL + 7046029254386353087LL;
    v ^= v >> 17; v ^= v << 31; v ^= v >> 8;
    w = 4294957665U*(w & 0xffffffff) + (w >> 32);
    unsigned long long x = u ^ (u << 21); x ^= x >> 35; x ^= x << 4;

    return (x + v) ^ w;
}

void sran(unsigned long long j){
    u = j ^ v; ran();
    v = u; ran();
    w = v; ran();
}
int main() {
    sran(time(NULL));
    unif01_Gen* gen = unif01_CreateExternGenBits("ran", ran);
    bbattery_BigCrush(gen);
    unif01_DeleteExternGenBits(gen);
    return 0;
}
