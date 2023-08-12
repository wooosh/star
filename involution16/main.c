#include "debugger.h"
#include "involution16.h"
#include "disasm.h"

#include "utui.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

// TODO: enforce first argument being unique from remaining arguments

static void Assume(bool cond) {
  if (!cond) {
    perror("error");
    exit(EXIT_FAILURE);
  }
}

// TODO: toggle colors
static void DumpDisasm(size_t len, uint8_t *input) {
  for (size_t i = 0; i < len; i += 2) {
    char line[kMaxInsnStrLen + 1];
    struct UTuiStyle style[sizeof(line)] = {0};
    DisAsmFmt fmt[kMaxInsnStrLen];

    size_t n = InsnToStr(input + i, line, fmt);

    // convert DisAsmFmt to UTuiStyle
    for (size_t j = 0; j < n; j++) {
      style[j].fg.kind = kUTuiColorIndexed;
      style[j].fg.color[0] = 30 + fmt[j] - '0';
    }

    line[n] = '\n';
    n++;

    UTui_Write(n, line, style);
  }
  printf("\x1b[m");
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
  uint8_t *input = malloc(len);
  Assume(input);
  size_t bytes_read = fread(input, 1, len, f);
  Assume(len == bytes_read);
  int close_err = fclose(f);
  Assume(close_err != EOF);

  DumpDisasm(len, input);

  struct VM *vm = VMCreate();
  memcpy(vm->memory, input, len);

  struct Debugger dbg = DebuggerCreate(vm);
  RunDebugger(&dbg);

  /*
  while (!vm->err) ExecuteStep(vm);
  fprintf(stderr, "error @ pc=0x%04x - %s\n", vm->pc, kErrorStrings[vm->err]);
  */
  return 0;
}
