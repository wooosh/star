See https://wooo.sh/involution16.html for documentation on the ISA itself.

`involution.c` contains C source code for an emulator for the ISA.

The `fasm` directory contains sources to be used with the [`fasmg`](https://flatassembler.net/docs.php?article=fasmg) assembler.

The `fasm/involution16.inc` file enables `fasmg` to produce ROMs for the `involution16` emulator.

The WIP debugger keybinds are as follows:
 - `q` - quit
 - `n` - next instruction
 - `p` - previous instruction
 - `uparrow` - scroll up
 - `downarrow` - scroll down