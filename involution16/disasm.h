#ifndef DISASM_H_
#define DISASM_H_

#include <stdint.h>
#include <stddef.h>

enum {
  // longest disassembly is 21 chars:
  //   srr r1, r2, p.invalid
  kMaxInsnStrLen = 18,
};

// used for syntax highlighting
typedef uint8_t DisAsmFmt;
enum {
  kDisAsmSpace  = '0',
  kDisAsmOp     = '1',
  kDisAsmReg    = '2',
  kDisAsmLitHex = '3',
  kDisAsmLitDec = '4',
  kDisAsmPerm   = '5',
  kDisAsmComma  = '6',
};

// insn must be two bytes
// s must have space for at least kMaxInsnStrLen + 1 chars
// DisAsmKind must have space for at least kMaxInsnStrLen chars, or be NULL
// returns number of chars written to s, not including null terminator
size_t InsnToStr(uint8_t *insn, char *s, DisAsmFmt *fmt);

#endif