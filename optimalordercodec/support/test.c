#include "../optimalordercodec.h"
#include "rng.h"
#include "timer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: test various sizes of vector

void TestInvShuf(unsigned trials, uint64_t seed) {
  uint8_t p_original[kOrderMaxElemU64] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19
  };

  uint8_t p_transformed[kOrderMaxElemU64];

  RNG *rng = NewRNG();
  SeedRNG(rng, seed);

  for (unsigned i = 0; i < trials; i++) {
    RandomPermutation(rng, kOrderMaxElemU64, p_original);
    memcpy(p_transformed, p_original, sizeof(p_transformed));

    InvShuf(sizeof(p_transformed), p_transformed);

    for (unsigned i = 0; i < kOrderMaxElemU64; i++) {
      if (p_transformed[i] > i) {
        fprintf(stderr, "TestInvShuf: ordering constraint failure\n");
        exit(EXIT_FAILURE);
      }
    }

    InvInvShuf(sizeof(p_transformed), p_transformed);

    if (memcmp(p_original, p_transformed, sizeof(p_original)) != 0) {
      fprintf(stderr, "TestInvShuf: reconstruction failure\n");
      exit(EXIT_FAILURE);
    }
  }

  fprintf(stderr, "TestInvShuf: success\n");

  DestroyRNG(rng);
}

void TestFactoradic(unsigned trials, uint64_t seed) {
  uint8_t p_original[kOrderMaxElemU64] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19
  };

  uint8_t p_factoradic[kOrderMaxElemU64];
  uint8_t p_restored[kOrderMaxElemU64];

  RNG *rng = NewRNG();
  SeedRNG(rng, seed);

  for (unsigned i = 0; i < trials; i++) {
    RandomPermutation(rng, kOrderMaxElemU64, p_original);
    memcpy(p_factoradic, p_original, sizeof(p_factoradic));

    // convert to factoradic sequence
    InvShuf(kOrderMaxElemU64, p_factoradic);

    uint64_t factoradic = PermutationToFactoradicU64(kOrderMaxElemU64, p_factoradic);
    PermutationFromFactoradicU64(factoradic, p_restored);

    if (memcmp(p_factoradic, p_restored, sizeof(p_original)) != 0) {
      fprintf(stderr, "TestFactoradic: failure\n");
      exit(EXIT_FAILURE);
    }
  }

  fprintf(stderr, "TestFactoradic: success\n");

  DestroyRNG(rng);
}