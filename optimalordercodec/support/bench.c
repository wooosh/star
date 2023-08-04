#include "../optimalordercodec.h"
#include "rng.h"
#include "timer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void BenchInvShuf(unsigned block_size, unsigned num_blocks, uint64_t seed) {
#ifndef NDEBUG
  fprintf(stderr, "BenchInvShuf: warning: debug mode enabled\n");
#endif

  RNG *rng = NewRNG();
  SeedRNG(rng, seed);

  uint8_t *block = (uint8_t *) malloc(block_size * kOrderMaxElemU64);
  if (!block) {
    fprintf(stderr, "BenchInvShuf: error: cannot allocate enough memory for block\n");
    exit(EXIT_FAILURE);
  }

  for (unsigned i = 0; i < block_size; i++) {
    uint8_t *block_entry = block + i*kOrderMaxElemU64;
    for (unsigned j = 0; j<kOrderMaxElemU64; j++) {
      block_entry[j] = j;
    }
  }

  Timer *inv_shuf    = NewTimer();
  Timer *invinv_shuf = NewTimer();

  for (unsigned b = 0; b < num_blocks; b++) {
    for (unsigned i = 0; i < block_size; i++) {
      RandomPermutation(rng, kOrderMaxElemU64, block + i*kOrderMaxElemU64);
    }

    uint8_t *block_entry = block;
    ResumeTimer(inv_shuf);
    for (unsigned i = 0; i < block_size; i++) {
      InvShuf(kOrderMaxElemU64, block_entry);
      block_entry += kOrderMaxElemU64;
    }
    PauseTimer(inv_shuf);

    block_entry = block;
    ResumeTimer(invinv_shuf);
    for (unsigned i = 0; i < block_size; i++) {
      InvInvShuf(kOrderMaxElemU64, block_entry);
      block_entry += kOrderMaxElemU64;
    }
    PauseTimer(invinv_shuf);
  }

  double op_count          = block_size * num_blocks;
  double inv_shuf_ns_op    = TimerDurationNSec(inv_shuf)/op_count;
  double invinv_shuf_ns_op = TimerDurationNSec(invinv_shuf)/op_count;

  fprintf(stderr, "BenchInvShuf:    InvShuf: %.2f ns/op\n", inv_shuf_ns_op);
  fprintf(stderr, "BenchInvShuf: InvInvShuf: %.2f ns/op\n", invinv_shuf_ns_op);

  DestroyTimer(invinv_shuf);
  DestroyTimer(inv_shuf);
  DestroyRNG(rng);
  free(block);
}

__attribute__ ((noinline))
static void PermutationFromFactoradicU64_Unoptimized(uint64_t f, uint8_t n, uint8_t *p) {
  uint64_t fact_i = 1;
  p[0] = 0;
  for (unsigned i = 1; i < n; i++) {
    fact_i *= i;
    p[i] = (f / fact_i) % (i + 1);
  }
}

void BenchFactoradic(unsigned block_size, unsigned num_blocks, uint64_t seed) {
#ifndef NDEBUG
  fprintf(stderr, "BenchFactoradic: warning: debug mode enabled\n");
#endif

  RNG *rng = NewRNG();
  SeedRNG(rng, seed);

  uint8_t  *block_p          = malloc(block_size * kOrderMaxElemU64);
  uint64_t *block_factoradic = malloc(block_size * sizeof(uint64_t));
  if (!block_p) {
    fprintf(stderr, "BenchFactoradic: error: cannot allocate enough memory for block\n");
    exit(EXIT_FAILURE);
  }

  for (unsigned i = 0; i < block_size; i++) {
    uint8_t *block_entry = block_p + i*kOrderMaxElemU64;
    for (unsigned j = 0; j<kOrderMaxElemU64; j++) {
      block_entry[j] = j;
    }
  }

  Timer *to_factoradic         = NewTimer();
  Timer *from_factoradic_unopt = NewTimer();
  Timer *from_factoradic_opt   = NewTimer();

  uint8_t p_initial[kOrderMaxElemU64] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19
  };

  for (unsigned b = 0; b < num_blocks; b++) {
    uint8_t  *block_p_entry = block_p;
    for (unsigned i = 0; i < block_size; i++) {
      memcpy(block_p_entry, p_initial, sizeof(p_initial));
      RandomPermutation(rng, kOrderMaxElemU64, block_p_entry);
      InvShuf(kOrderMaxElemU64, block_p_entry);
      block_p_entry += kOrderMaxElemU64;
    }

    block_p_entry = block_p;
    ResumeTimer(to_factoradic);
    for (unsigned i = 0; i < block_size; i++) {
      block_factoradic[i] = PermutationToFactoradicU64(kOrderMaxElemU64, block_p_entry);
      block_p_entry += kOrderMaxElemU64;
    }
    PauseTimer(to_factoradic);

    block_p_entry = block_p;
    ResumeTimer(from_factoradic_unopt);
    for (unsigned i = 0; i < block_size; i++) {
      PermutationFromFactoradicU64_Unoptimized(block_factoradic[i], kOrderMaxElemU64, block_p_entry);
      block_p_entry += kOrderMaxElemU64;
    }
    PauseTimer(from_factoradic_unopt);

    block_p_entry = block_p;
    ResumeTimer(from_factoradic_opt);
    for (unsigned i = 0; i < block_size; i++) {
      PermutationFromFactoradicU64(block_factoradic[i], block_p_entry);
      block_p_entry += kOrderMaxElemU64;
    }
    PauseTimer(from_factoradic_opt);
  }

  double op_count = block_size * num_blocks;
  double to_ns_op = TimerDurationNSec(to_factoradic)/op_count;
  double from_unopt_ns_op = TimerDurationNSec(from_factoradic_unopt)/op_count;
  double from_opt_ns_op   = TimerDurationNSec(from_factoradic_opt)/op_count;

  fprintf(stderr, "BenchFactoradic:                 To: %3.2f ns/op\n", to_ns_op);
  fprintf(stderr, "BenchFactoradic: From (unoptimized): %3.2f ns/op\n", from_unopt_ns_op);
  fprintf(stderr, "BenchFactoradic: From   (optimized): %3.2f ns/op\n", from_opt_ns_op);

  DestroyTimer(from_factoradic_opt);
  DestroyTimer(from_factoradic_unopt);
  DestroyTimer(to_factoradic);
  DestroyRNG(rng);
  free(block_p);
}