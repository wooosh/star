#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

// TODO: enforce first argument being unique from remaining arguments

void Assume(bool cond) {
  if (!cond) {
    perror("error");
    exit(EXIT_FAILURE);
  }
}

uint32_t Ror32(uint32_t x, uint8_t n) {
  assert(n < 32);
  return (x >> n) | (x << (32 - n));
}

uint32_t Rol32(uint32_t x, uint8_t n) {
  assert(n < 32);
  return (x << n) | (x >> (-n & 31));
}

enum {
  kOpAdd = 0x0,
  kOpSub = 0x1,
  kOpRor = 0x2,
  kOpRol = 0x3,
  kOpShr = 0x4,
  kOpShl = 0x5,
  kOpAnd = 0x6,
  kOpOra = 0x7,
  kOpMul = 0x8,
  kOpDiv = 0x9,
  kOpCmp = 0xA,
  kOpJeq = 0xB,
  kOpXri = 0xC,
  kOpSrr = 0xD,
  kOpSrm = 0xE,
  kOpBrk = 0xF
};

typedef uint8_t ExecutionDirection;
enum {
  kExecutingBackward = -1,
  kExecutingForward  = 1
};

typedef uint8_t ErrorCode;
enum {
  kErrorNone = 0,
  kErrorMisalignedJump,
  kErrorMismatchedJump,
  kErrorInvalidSrrEncoding,
};

const char *kErrorStrings[] = {
  [kErrorNone] = "no error",
  [kErrorMisalignedJump] = "jump address not aligned to 2 bytes",
  [kErrorMismatchedJump] = "jump instruction is not equal to it's target",
  [kErrorInvalidSrrEncoding] = "srr encoded with unknown control byte",
};

// TODO: maybe use the extra bit for memory info, but make it so that the first
// byte of memory operand gets swapped
static const uint8_t kSrrCodes[][4] = {
  // swap registers
  {2, 3, 0, 1},
  // reverse bytes
  {3, 2, 1, 0},
  // swap bytes of first register
  {1, 0, 2, 3},
  // swap bytes of both registers
  {1, 0, 3, 2},
  // swap inner bytes
  {0, 2, 1, 3},
  // swap outer bytes
  {3, 1, 2, 0},
  // swap upper bytes of registers
  {0, 3, 2, 1},
  // swap lower bytes of registers
  {2, 1, 0, 3},
};

struct VM {
  uint16_t reg[16];
  // 2^16 and not 2^16 - 1 because technically the last cell of memory is
  // accessible if you do a two byte read at 0xFFFF
  uint8_t memory[65536];
  uint16_t pc;
  ExecutionDirection direction;
  ErrorCode err;
};

struct VM *VMCreate(void) {
  struct VM *vm = malloc(sizeof(*vm));
  Assume(vm);
  memset(vm->reg, 0, sizeof(vm->reg));
  memset(vm->memory, (kOpBrk << 4) | 0xF, sizeof(vm->memory));
  vm->pc = 0;
  vm->direction = kExecutingForward;
  vm->err = kErrorNone;

  return vm;
}

void ExecuteStep(struct VM *vm) {
  uint8_t insn[2];
  memcpy(&insn, vm->memory + vm->pc, sizeof(insn));
  uint8_t field[4] = {
    insn[0] >> 4,
    insn[0] & 0xF,
    insn[1] >> 4,
    insn[1] & 0xF
  };

  switch (field[0]) {
    case kOpAdd:
      vm->reg[field[1]] ^= vm->reg[field[2]] + vm->reg[field[3]];
      break;
    case kOpSub:
      vm->reg[field[1]] ^= vm->reg[field[2]] - vm->reg[field[3]];
      break;
    case kOpRor:
      vm->reg[field[1]] ^= Ror32(vm->reg[field[2]], vm->reg[field[3]] & 0xF);
      break;
    case kOpRol:
      vm->reg[field[1]] ^= Rol32(vm->reg[field[2]], vm->reg[field[3]] & 0xF);
      break;
    case kOpShr:
      vm->reg[field[1]] ^= vm->reg[field[2]] >> (vm->reg[field[3]] & 0xF);
      break;
    case kOpShl:
      vm->reg[field[1]] ^= vm->reg[field[2]] << (vm->reg[field[3]] & 0xF);
      break;
    case kOpAnd:
      vm->reg[field[1]] ^= vm->reg[field[2]] & vm->reg[field[3]];
      break;
    case kOpOra:
      vm->reg[field[1]] ^= vm->reg[field[2]] | vm->reg[field[3]];
      break;
    case kOpMul:
      vm->reg[field[1]] ^= (unsigned) vm->reg[field[2]] * vm->reg[field[3]];
      break;
    case kOpDiv:
      if (vm->reg[field[3]] != 0)
        vm->reg[field[1]] ^= vm->reg[field[2]] / vm->reg[field[3]];
      break;
    // special ops
    case kOpCmp: {
      uint16_t a = vm->reg[field[2]];
      uint16_t b = vm->reg[field[3]];
      uint16_t flag;

      if (a == b)     flag = 0;
      else if (a > b) flag = 1;
      else            flag = -1;
      vm->reg[field[1]] ^= flag;
      break;
    }
    case kOpJeq: {
      if (vm->reg[field[2]] == vm->reg[field[3]]) {
        uint16_t target = vm->reg[field[1]];
        if (target & 1) {
          vm->err = kErrorMisalignedJump;
          return;
        }

        if (memcmp(vm->memory + vm->pc, vm->memory + target, 2) != 0) {
          vm->err = kErrorMismatchedJump;
          return;
        }
        vm->reg[field[1]] = vm->pc;
        vm->pc = target;
      }
      break;
    }
    case kOpXri: {
      vm->reg[field[1]] ^= insn[1];
      break;
    }
    case kOpSrr: {
      uint16_t ctl = field[3];
      if (ctl > sizeof(kSrrCodes)/sizeof(kSrrCodes[0])) {
        vm->err = kErrorInvalidSrrEncoding;
        return;
      }

      uint8_t bytes[4], bytes_new[4];
      memcpy(bytes, &vm->reg[field[1]], 2);
      memcpy(bytes + 2, &vm->reg[field[2]], 2);

      const uint8_t *perm = kSrrCodes[ctl];
      for (int i = 0; i < 4; i++) {
        bytes_new[i] = bytes[perm[i]];
      }

      memcpy(&vm->reg[field[1]], bytes_new, 2);
      memcpy(&vm->reg[field[2]], bytes_new + 2, 2);

      break;
    }
    case kOpSrm: {
      uint16_t tmp;
      memcpy(&tmp, vm->memory + vm->reg[field[2]], 2);
      memcpy(vm->memory + vm->reg[field[2]], &vm->reg[field[1]], 2);
      vm->reg[field[1]] = tmp;
      break;
    }
    case kOpBrk:
      fprintf(stderr, "\nPROGRAM BREAK\n");
      fprintf(stderr, "pc = 0x04%x\n", vm->pc);
      fprintf(stderr, "break field = %x\n", field[1]);
      fprintf(stderr, "register dump:\n");
      for (int i = 0; i < 16; i++) {
        fprintf(stderr, "r%x: %6d, %6d, 0x%04x\n", i, vm->reg[i], (int16_t)vm->reg[i], vm->reg[i]);
      }

      if (field[1] == 0xF && field[2] == 0xF) {
        exit(0);
      }
      break;

  }

  vm->pc += sizeof(uint16_t) * vm->direction;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "expected exactly one filename argument\n");
    exit(EXIT_FAILURE);
  }

  FILE *f = fopen(argv[1], "rb");
  Assume(f);
  int success = fseek(f, 0, SEEK_END);
  Assume(!success);
  size_t len = ftell(f);
  Assume(len >= 0);
  if (len > 65535) {
    fprintf(stderr, "rom file too large\n");
    exit(EXIT_FAILURE);
  }
  rewind(f);
  char *input = malloc(len);
  Assume(input);
  size_t bytes_read = fread(input, 1, len, f);
  Assume(len == bytes_read);
  int close_err = fclose(f);
  Assume(close_err != EOF);

  struct VM *vm = VMCreate();
  memcpy(vm->memory, input, len);

  while (!vm->err) ExecuteStep(vm);
  fprintf(stderr, "error @ pc=0x%04x - %s\n", vm->pc, kErrorStrings[vm->err]);
  return 0;
}
