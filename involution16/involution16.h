#ifndef INVOLUTION16_H_
#define INVOLUTION16_H_

#include <stdint.h>

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

extern const char *kOpNames[];

typedef int8_t ExecutionDirection;
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
extern const char *kErrorStrings[];

enum {kSrrCodeCount = 8};
extern const uint8_t kSrrCodes[][4];

struct VM {
  uint16_t reg[16];
  // 2^16 and not 2^16 - 1 because technically the last cell of memory is
  // accessible if you do a two byte read at 0xFFFF
  uint8_t memory[65536];
  uint16_t pc;
  ExecutionDirection direction;
  ExecutionDirection brk_dir;
  ErrorCode err;
};

struct VM *VMCreate(void);
void ExecuteStep(struct VM *vm);

#endif