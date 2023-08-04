project('optimalordercoder', ['c', 'cpp'], default_options: 'b_ndebug=if-release')

support_srcs = [
  'support/driver.c',
  'support/bench.c',
  'support/test.c',
  'support/rng.cc',
  'support/timer.cc'
]

executable('testbench', sources : support_srcs + 'optimalordercodec.c')