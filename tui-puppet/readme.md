# tui-puppet

`tui-puppet` runs a program inside an emulated terminal (xterm/vt100 compatible)
and reads a command file that can send input, resize the terminal, and capture
the screen.

Screen capturing is achieved using the `hash` command (all commands are listed
below), which writes a hash of the screen state to stdout. You can redirect
stdout to a file and use `cmp` to compare with another hash to ensure your TUI 
program renders consistently. You can capture the screen state as many times as
you would like during the lifetime of the process.

```
usage: tui-puppet [-r] <command_file> <executable> <arguments to executable>
  -r | show the terminal state on stderr
     |   this is designed to be used with xterm-truecolor with the following
     |   set in .Xresources:
     |
     |    *VT100.allowWindowOps: true 
```

The command file format is relatively simple. Each line contains a single
command.

Commands:
```
keystream [key] ...
  Sends the stream of keys to stdin. The stream of keys is space separated. if
  a key is not found in the special keys list or parsed as a keybind, the full
  string will be written to stdin.

  example:
    keystream hello <space> world <enter>

  special key names:
    <space>
    <enter>
    <tab>

    <esc>

    <up>, <down>, <right>, <left>
    
    <backspace>
      NOTE: uses ascii delete, not ctrl-h, which is used in some terminals
    <delete>

    <ctrl-X> 
      X can be any one byte character
  keynames to be added:
    <f1> through <f12>
    <ctrl-up,down,right,left>
    <ctrl-shift-x>

  Please create an issue if you need keys other than these.

repeat <number> <key>
  Repeats a key a certain number of times.

hash
  Prints the hashed terminal state to stdout.
```

Commands to be added:
```
#
  Does nothing and ignores the rest of the line. A space is required after #

resize <width <height>
  Resizes the terminal to width x height. The terminal is 132x42 by default.

quit
  Exits the program.
```

# Installation

Run dependencies:
- libvterm
- libcrypto (part of openssl)

Build dependencies:
- pkgconfig
- c compiler
- make

```
make
sudo make install
```

# todo

- [ ] handle ctrl-c properly
- [ ] pause on quit in show terminal mode
- [ ] configurable poll timeout
- [ ] sleep command
- [ ] clean up code
- [ ] resizing
- [ ] quit command
- [ ] full keybind parser
- [ ] comments in commandfile
- [ ] hash color info
- [ ] hash cursor position
- [ ] export proper terminal variable
- [ ] tests
