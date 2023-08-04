#ifndef OPTIMAL_ORDER_CODEC_H_
#define OPTIMAL_ORDER_CODEC_H_

#include <stdint.h>

enum {
  // maximum number of elements that can be encoded in a u64
  kOrderMaxElemU64 = 20
};

// encodes the n-element array p into a uint64
// 0 <= n <= kOrderMaxElemU64
// p must contain every integer from 0 to n with no duplicates
uint64_t OptimalOrderEncode(uint8_t n, uint8_t *p);

// decodes the value o produced by OptimalOrderEncode into n-element array p
// the same n must be used for encoding and decoding
void OptimalOrderDecode(uint64_t o, uint8_t n, uint8_t *p);

// internal functions follow

// transforms p such that p'[i] <= i and InvInvShuf(p') = p
// p must contain each integer in [0, n) exactly once
void InvShuf(uint8_t n, uint8_t *p);
// see InvShuf
void InvInvShuf(uint8_t n, uint8_t *p);

// converts a permutation p that satisifies p[i] <= i for all i < n into a 64
// bit factoradic number
uint64_t PermutationToFactoradicU64(uint8_t n, uint8_t *p);
// decodes a 64 bit factoradic number f into permutation p with kOrderMaxElemU64
// elements
void PermutationFromFactoradicU64(uint64_t f, uint8_t *p);

#endif