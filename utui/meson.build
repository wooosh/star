project('utui', 'c')

utui = library('tui',
  ['utui_output.c', 'utui_input.c'],
  dependencies: dependency('libxxhash'))

utui_dep = declare_dependency(include_directories: include_directories('.'),
  link_with: utui)