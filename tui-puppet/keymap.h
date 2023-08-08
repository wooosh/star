#ifndef KEYMAP_H
#define KEYMAP_H

#include <stddef.h>

struct key_pair {
  char* key;
  char* val;
};

static struct key_pair keymap[] = {
  {"<enter>", "\r"},
  {"<space>", " "},
  {"<tab>",   "\t"},

  {"<esc>",   "\x1b"},
  
  {"<up>",    "\x1b[A"},
  {"<down>",  "\x1b[B"},
  {"<right>", "\x1b[C"},
  {"<left>",  "\x1b[D"},

  {"<backspace>", "\x7F"},
  {"<delete>",    "\x1b[3~"},

  {NULL, NULL}
};
#endif
