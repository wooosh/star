#ifndef RNG_H_
#define RNG_H_

#include <stdint.h>

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

typedef struct RNG RNG;

EXTERNC RNG *NewRNG(void);
EXTERNC void DestroyRNG(RNG *rng);
EXTERNC void SeedRNG(RNG *rng, uint64_t x);
// shuffles the first n elements of p randomly
EXTERNC void RandomPermutation(RNG *rng, uint8_t n, uint8_t *p);

#undef EXTERNC
#endif