#include "rng.h"

#include <random>
#include <algorithm>



struct RNG {
  std::default_random_engine engine;
};

RNG *NewRNG(void) {
  return new RNG;
}

void DestroyRNG(RNG *rng) {
  delete rng;
};

void SeedRNG(RNG *rng, uint64_t x)  {
  std::seed_seq seq({x}); // very silly 
  rng->engine.seed(seq);
}

void RandomPermutation(RNG *rng, uint8_t n, uint8_t *p) {
  std::shuffle(p, p + n, rng->engine);
}