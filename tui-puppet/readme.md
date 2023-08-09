# tui-puppet
`tui-puppet` is a tool that enables [characterization/golden testing](https://en.wikipedia.org/wiki/Characterization_test) for TUIs (terminal user interfaces) using [Janet](https://github.com/janet-lang/janet/) programs.

`tui-puppet` will execute a program inside an emulated terminal (xterm/vt100 compatible) and run a Janet program that controls the terminal, by sending input, resizing the terminal, and capturing the screen data as a hash value.

```
usage: tui-puppet [-r] <file.janet> <executable> <arguments to executable>
  -r | show the terminal state on stderr
     |   this is designed to be used with xterm-truecolor with the following
     |   set in .Xresources:
     |
     |    *VT100.allowWindowOps: true
```

TODO: more docs

# Installation
Requires `libvterm`, `libcrypto`, `janet`, and `meson`.