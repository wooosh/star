#include "disasm.h"
#include "involution16.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

// TODO: debug labels

const char *kOpNames[] = {
  [kOpAdd] = "add",
  [kOpSub] = "sub",
  [kOpRor] = "ror",
  [kOpRol] = "rol",
  [kOpShr] = "shr",
  [kOpShl] = "shl",
  [kOpAnd] = "and",
  [kOpOra] = "ora",
  [kOpMul] = "mul",
  [kOpDiv] = "div",
  [kOpCmp] = "cmp",
  [kOpJeq] = "jeq",
  [kOpXri] = "xri",
  [kOpSrr] = "srr",
  [kOpSrm] = "srm",
  [kOpBrk] = "brk",
};

size_t InsnToStr(uint8_t *insn, char *s, DisAsmFmt *fmt) {
  uint8_t field[4] = {
    insn[0] >> 4,
    insn[0] & 0xF,
    insn[1] >> 4,
    insn[1] & 0xF
  };

  switch (field[0]) {
    // three register operands
    case kOpAdd:
    case kOpSub:
    case kOpRor:
    case kOpRol:
    case kOpShr:
    case kOpShl:
    case kOpAnd:
    case kOpOra:
    case kOpMul:
    case kOpDiv:
    case kOpCmp:
    case kOpJeq: {
      int n = snprintf(s, kMaxInsnStrLen + 1, "%s r%X, r%X, r%X",
        kOpNames[field[0]], field[1], field[2], field[3]);
      assert(n > 0);
      if (fmt != NULL) {
        const char *fmt_str = "11102260226022";
        memcpy(fmt, fmt_str, n);
      }
      return n;
    };
    case kOpXri: {
      int n = snprintf(s, kMaxInsnStrLen + 1, "xri r%X, 0x%02X (%03u)",
        field[1], insn[1], insn[1]);
      assert(n > 0);
      if (fmt != NULL) {
        const char *fmt_str = "111022603333044444";
        memcpy(fmt, fmt_str, n);
      }
      return n;
    }
    case kOpSrr: {
      if (field[3] >= kSrrCodeCount) {
        int n = snprintf(s, kMaxInsnStrLen + 1,"srr r%X, r%X, p.invalid",
          field[1], field[2]);
        assert(n > 0);
        if (fmt != NULL) {
          const char *fmt_str = "111022602260555555555";
          memcpy(fmt, fmt_str, n);
        }
        return n;
      }
      // permutation for given srr parameter
      const uint8_t *p = kSrrCodes[field[3]];
      int n = snprintf(s, kMaxInsnStrLen + 1, "srr r%X, r%X, p.%c%c%c%c",
        field[1], field[2], 'A' + p[0], 'A' + p[1], 'A' + p[2], 'A' + p[3]);
      assert(n > 0);
      if (fmt != NULL) {
        const char *fmt_str = "111022602260555555";
        memcpy(fmt, fmt_str, n);
      }
      return n;
    }
    case kOpSrm: {
      int n = snprintf(s, kMaxInsnStrLen + 1, "srm r%X, r%X",
        field[1], field[2]);
      assert(n > 0);
      if (fmt != NULL) {
        const char *fmt_str = "1110226022";
        memcpy(fmt, fmt_str, n);
      }
      return n;
    }
    case kOpBrk: {
      int n = snprintf(s, kMaxInsnStrLen + 1, "brk");
      assert(n > 0);
      if (fmt != NULL) {
        const char *fmt_str = "111";
        memcpy(fmt, fmt_str, n);
      }
      return n;
    }
  }

  assert(0);
}