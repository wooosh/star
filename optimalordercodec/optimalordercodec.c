#include "optimalordercodec.h"

#include <assert.h>
#include <string.h>

void InvShuf(uint8_t n, uint8_t *p) {
  assert(n <= kOrderMaxElemU64);
  uint8_t s[kOrderMaxElemU64] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19
  };

  for (unsigned i = n; i-- > 0;) {
    unsigned x = p[i];

    p[i]    = s[x]; // s[x] holds the index that the value x resides at
    s[s[x]] = s[i];
    s[s[i]] = p[i];
  }
}

void InvInvShuf(uint8_t n, uint8_t *p) {
  assert(n <= kOrderMaxElemU64);
  uint8_t s[kOrderMaxElemU64] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19
  };

  for (unsigned i = n; i-- > 0;) {
    unsigned rand = p[i];

    p[i] = s[rand];
    s[rand] = s[i];
  }
}

uint64_t PermutationToFactoradicU64(uint8_t n, uint8_t *p) {
  uint64_t acc = 0;
  uint64_t fact_i = 1;

  // skip first element because it will always be zero, and thus needs zero bits
  // to be encoded
  for (unsigned i = 1; i < n; i++) {
    fact_i *= i;
    acc += p[i] * fact_i;
  }

  return acc;
}

// constants for fast mod using the technique described in:
//   Faster Remainder by Direct Computation, arXiv:1902.01961 [cs.MS]
enum {kF59_Offset = 4, kF27_Offset = 16};
static uint64_t kModX_F61[] = {0xaaaaaaaaaaaaaab};
static uint64_t kModX_F59[] = {
  0x200000000000000, 0x19999999999999a, 0x155555555555556, 0x124924924924925,
  0x100000000000000, 0x0e38e38e38e38e4, 0x0cccccccccccccd, 0x0ba2e8ba2e8ba2f,
  0x0aaaaaaaaaaaaab, 0x09d89d89d89d89e, 0x092492492492493, 0x088888888888889,
};

static uint32_t kModX_F27[] = {
  0x800000, 0x787879, 0x71c71d, 0x6bca1b, 0x666667,
};

// returns n % d for d in [1, kMaxElemU64 + 1), but really fast
static inline uint64_t FastMod(uint64_t n, uint64_t d) {
  if ((d & (d - 1)) == 0) {
    return n & (d - 1);
  } else if (d == 3) {
    enum {F = 61};
    uint64_t x = kModX_F61[0] * n;
    x &= (1ULL << F) - 1;
    return (x * d) >> F;
  } else if (d < 16) {
    enum {kF = 59};
    uint64_t lowbits = kModX_F59[d - kF59_Offset] * n;
    lowbits &= (1ULL << kF) - 1;
    return (lowbits * d) >> kF;
  } else {
    enum {kF = 27};
    uint32_t lowbits = kModX_F27[d - kF27_Offset] * (uint32_t) n;
    lowbits &= (1 << kF) - 1;
    return (lowbits * d) >> kF;
  }
}

void PermutationFromFactoradicU64(uint64_t f, uint8_t *p) {
  unsigned i = 1;
  uint64_t fact_i = 1;
  p[0] = 0;

#define ITER\
  fact_i *= i; p[i] = FastMod(f / fact_i, i + 1); i++;

  // yes, this needs to be manually unrolled, otherwise the division won't
  // undergo strength reduction
  // only 19 iterations because the last element is 0, which doesn't need to be
  // decoded
  assert(kOrderMaxElemU64 == 20);
  ITER ITER ITER ITER ITER
  ITER ITER ITER ITER ITER
  ITER ITER ITER ITER ITER
  ITER ITER ITER ITER
#undef ITER
}

uint64_t OptimalOrderEncode(uint8_t n, uint8_t *p) {
  uint8_t p_copy[kOrderMaxElemU64];
  memcpy(p_copy, p, n);

  InvShuf(n, p_copy);
  return PermutationToFactoradicU64(n, p_copy);
}

void OptimalOrderDecode(uint64_t o, uint8_t n, uint8_t *p) {
  PermutationFromFactoradicU64(o, p);
  InvInvShuf(n, p);
}