#include "rng.h"

void rng_seed(Rng *rng, uint32_t seed) {
    /* State 0 would make xorshift output 0 forever. Remap it to a safe value. */
    rng->state = seed ? seed : 1u;
}

uint32_t rng_next(Rng *rng) {
    uint32_t x = rng->state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng->state = x;
    return x;
}

uint32_t rng_range(Rng *rng, uint32_t n) {
    return rng_next(rng) % n;
}

float rng_unit(Rng *rng) {
    return (rng_next(rng) % 10000) / 10000.0f;
}
