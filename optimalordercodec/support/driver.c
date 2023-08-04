#include <stdint.h>

void TestInvShuf(unsigned trials, uint64_t seed);
void TestFactoradic(unsigned trials, uint64_t seed);

void BenchInvShuf(unsigned block_size, unsigned num_blocks, uint64_t seed);
void BenchFactoradic(unsigned block_size, unsigned num_blocks, uint64_t seed);

int main(int argc, char **argv) {
  uint64_t seed;
  unsigned trials, block_size, num_blocks;
  seed       = 1234;
  trials     = 10000000;
  block_size = trials;
  num_blocks = 1;

  TestInvShuf(trials, 0);
  BenchInvShuf(block_size, num_blocks, seed);
  TestFactoradic(trials, 0);
  BenchFactoradic(block_size, num_blocks, seed);

  return 0;
}