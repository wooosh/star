#include "debugger.h"

#include "involution16.h"
#include "disasm.h"

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

// TODO: catch resizes

void Die(const char* msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

static struct termios orig_termios;
void TermiosRestore() {
  // TODO: call on exit
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
  printf("\x1b[?25h" "\x1b[0m" "\x1b""c");
  fflush(stdout);
}

void TermiosSetup() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    Die("tcgetattr");
  if (atexit(TermiosRestore) != 0)
    Die("atexit");

  struct termios raw = orig_termios;
  // set up the terminal to not print characters we type or response to escape
  // codes
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    Die("tcsetattr");
}

enum {kRegPaneHeight = 20};

struct Debugger DebuggerCreate(struct VM *vm) {
  struct Debugger dbg;
  dbg.vm = vm;
  dbg.input = UTuiInput_Init();
  dbg.output = UTuiOutput_Init();
  dbg.asm_addr_top = 0xFFFE;

  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  UTuiOutput_Resize(&dbg.output, w.ws_col, w.ws_row);

  return dbg;
}

void DrawAsmPane(struct Debugger *dbg) {
  size_t height = dbg->output.num_rows - kRegPaneHeight;

  const size_t min_line_size = 9 + kMaxInsnStrLen + 1;
  size_t line_size = dbg->output.num_cols;
  if (line_size < min_line_size)
    line_size = min_line_size;

  char *line = malloc(line_size);
  struct UTuiStyle *style = malloc(line_size * sizeof(*style));

  memset(line, ' ', dbg->output.num_cols);
  memset(style, 0, dbg->output.num_cols * sizeof(*style));
  for (size_t i = 0; i < dbg->output.num_cols; i++) {
    style[i].attr = kUTuiReverse;
  }

  snprintf(line, dbg->output.num_cols, " ASSEMBLY src: factorial.bin");
  UTuiOutput_SetLine(&dbg->output, 0, dbg->output.num_cols, line, style);

  bool drew_pc = false;
  for (size_t y = 0; y < height - 1; y++) {
    // address + instruction
    memset(line, ' ', line_size);
    memset(style, 0, line_size * sizeof(*style));

    DisAsmFmt fmt[kMaxInsnStrLen];

    const size_t prefix_len = 18;
    for (size_t j = 0; j < 8; j++) {
      style[j].bg.kind = kUTuiColorIndexed;
      style[j].bg.color[0] = 47;
    }

    for (size_t j = 8; j < prefix_len - 2; j++) {
      style[j].bg.kind = kUTuiColorIndexed;
      style[j].bg.color[0] = 107;
    }

    uint16_t insn_addr = dbg->asm_addr_top + (y - drew_pc) * 2;
    if (!drew_pc && dbg->vm->pc == insn_addr) {
      size_t n = snprintf(line, line_size, " ------  ------  PROGRAM COUNTER");
      for (size_t j = prefix_len-1; j < n; j++) {
        style[j].attr = kUTuiBold;
      }
      UTuiOutput_SetLine(&dbg->output, y + 1, n, line, style);
      drew_pc = true;
      continue;
    }

    snprintf(line, line_size, " 0x%04X  0x%02X%02X  ",
      insn_addr, dbg->vm->memory[insn_addr], dbg->vm->memory[insn_addr+1]);

    size_t n = InsnToStr(dbg->vm->memory + insn_addr, line + prefix_len, fmt);

    // convert DisAsmFmt to UTuiStyle
    for (size_t j = 0; j < n; j++) {
      style[prefix_len+j].fg.kind = kUTuiColorIndexed;
      style[prefix_len+j].fg.color[0] = 30 + fmt[j] - '0';
    }

    n += prefix_len;

    UTuiOutput_SetLine(&dbg->output, y + 1, n, line, style);
  }
}

// TODO: show error down here
void DrawRegPane(struct Debugger *dbg) {
  char *line = malloc(dbg->output.num_cols);
  struct UTuiStyle *style = malloc(dbg->output.num_cols * sizeof(*style));

  size_t y = dbg->output.num_rows - kRegPaneHeight;

  memset(line, ' ', dbg->output.num_cols);
  memset(style, 0, dbg->output.num_cols * sizeof(*style));
  for (size_t i = 0; i < dbg->output.num_cols; i++) {
    style[i].attr = kUTuiReverse;
  }

  snprintf(line, dbg->output.num_cols, " REGISTERS");
  UTuiOutput_SetLine(&dbg->output, y, dbg->output.num_cols, line, style);
  y++;

  memset(style, 0, dbg->output.num_cols * sizeof(*style));
  assert(dbg->output.num_cols >= 4);
  // bold the name of the register
  for (size_t i = 0; i < 8; i++) {
    style[i].attr = kUTuiBold;
  }

  for (size_t j = 0; j < 8; j++) {
    style[j].bg.kind = kUTuiColorIndexed;
    style[j].bg.color[0] = 47;
  }

  for (size_t j = 8; j < 16; j++) {
    style[j].bg.kind = kUTuiColorIndexed;
    style[j].bg.color[0] = 107;
  }

  // show pc
  uint8_t color = dbg->vm->pc ? 34 : 0;
  for (size_t i = 8; i < dbg->output.num_cols; i++) {
    style[i].fg.kind = kUTuiColorIndexed;
    style[i].fg.color[0] = color;
  }
  size_t n = snprintf(line, dbg->output.num_cols, " pcnext  0x%04X  %5u", dbg->vm->pc, dbg->vm->pc);
  UTuiOutput_SetLine(&dbg->output, y, n, line, style);
  y++;
  uint16_t pc_p = dbg->vm->pc - 2;
  color = pc_p ? 34 : 0;
  for (size_t i = 8; i < dbg->output.num_cols; i++) {
    style[i].fg.kind = kUTuiColorIndexed;
    style[i].fg.color[0] = color;
  }
  n = snprintf(line, dbg->output.num_cols, " pcprev  0x%04X  %5u", pc_p, pc_p);
  UTuiOutput_SetLine(&dbg->output, y, n, line, style);
  y++;

  // show remaining registers
  for (int i = 0; i < 16; i++) {
    uint8_t color = dbg->vm->reg[i] ? 34 : 0;
    for (size_t i = 8; i < dbg->output.num_cols; i++) {
      style[i].fg.kind = kUTuiColorIndexed;
      style[i].fg.color[0] = color;
    }
    size_t n = snprintf(line, dbg->output.num_cols, "     r%X  0x%04X  %5u", i, dbg->vm->reg[i], dbg->vm->reg[i]);
    UTuiOutput_SetLine(&dbg->output, y, n, line, style);
    y++;
  }

  memset(line, ' ', dbg->output.num_cols);
  memset(style, 0, dbg->output.num_cols * sizeof(*style));
  for (size_t i = 0; i < dbg->output.num_cols; i++) {
    style[i].attr = kUTuiReverse;
  }

  snprintf(line, dbg->output.num_cols, " error: %s", kErrorStrings[dbg->vm->err]);
  UTuiOutput_SetLine(&dbg->output, y, dbg->output.num_cols, line, style);

  free(style);
  free(line);
}

void DrawDebugger(struct Debugger *dbg) {
  DrawAsmPane(dbg);
  DrawRegPane(dbg);
  UTuiOutput_Flip(&dbg->output);
}

void RunDebugger(struct Debugger *dbg) {
  TermiosSetup();
  write(STDOUT_FILENO, "\x1b""c", 2);
  while (1) {
    DrawDebugger(dbg);

    UTuiKey key = UTuiInput_ReadKey(&dbg->input, -1);
    if (key == kUTuiKeyNone) continue;
    if (key == kUTuiInputError) {
      TermiosRestore();
      fprintf(stderr, "input error\n");
      exit(EXIT_FAILURE);
    }

    switch (key & kUTuiKeyBaseMask) {
      case kUTuiKeyNone:
        continue;
      case kUTuiInputError:
        TermiosRestore();
        fprintf(stderr, "input error\n");
        exit(EXIT_FAILURE);
      case 'q':
        TermiosRestore();
        exit(EXIT_SUCCESS);
      // TODO: keep PC in center of window when stepping
      case 'n':
        if (dbg->vm->err) break;
        // TODO: error handling
        dbg->vm->direction = kExecutingForward;
        ExecuteStep(dbg->vm);
        break;
      case 'p':
        if (dbg->vm->err) break;
        // TODO: error handling
        dbg->vm->direction = kExecutingBackward;
        ExecuteStep(dbg->vm);
        break;
      case kUTuiUpArrow:
        dbg->asm_addr_top -= 2;
        break;
      case kUTuiDownArrow:
        dbg->asm_addr_top += 2;
        break;
    }
  }
}