#ifndef DEBUGGER_H_
#define DEBUGGER_H_

#include "involution16.h"
#include "utui.h"

struct Debugger {
  struct VM *vm;

  struct UTuiInput input;
  struct UTuiOutput output;
  // assembly pane data
  // the address of the first line of text on the window in the VM's memory
  uint16_t asm_addr_top;

  // TODO: scratch line buffer
};

struct Debugger DebuggerCreate(struct VM *vm);
void RunDebugger(struct Debugger *dbg);

#endif