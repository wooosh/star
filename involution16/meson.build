project('involution16', 'c')

utui_dep = subproject('utui').get_variable('utui_dep')

executable('involution16',
  [
    'main.c',
    'involution16.c',
    'disasm.c',
    'debugger.c'
  ],
  dependencies: utui_dep)