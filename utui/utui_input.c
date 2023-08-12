#include "utui.h"

#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <ctype.h>
#include <assert.h>

struct UTuiInput UTuiInput_Init(void) {
  return (struct UTuiInput){0};
}

static void UpdateInputQueue(struct UTuiInput *i, int timeout) {
  struct pollfd pfd = {STDIN_FILENO, POLLIN};
  int ready = poll(&pfd, 1, timeout);

  if (ready == -1) {
    i->error = kUTuiInputErrorPoll;
    return;
  }

  if (ready > 0) {
    if (pfd.revents & POLLIN) {
      ssize_t result = read(STDIN_FILENO, i->seq + i->seq_cnt, sizeof(i->seq) - i->seq_cnt);
      if (result == -1) {
        i->error = kUTuiInputErrorRead;
        return;
      }
      i->seq_cnt += result;
    } else if (pfd.revents & POLLHUP) {
      i->error = kUTuiInputErrorHup;
    } else {
      i->error = kUTuiInputErrorPoll;
    }
  }
}

static void ConsumeInputQueue(struct UTuiInput *i, uint8_t count) {
  assert(count <= i->seq_cnt);
  i->seq_cnt -= count;
  memmove(i->seq, i->seq + count, i->seq_cnt);
}

static struct EscapeRule {
  UTuiKey key;
  // null terminated
  char pattern[kUTuiMaxInputSeq + 1];
  bool supports_modifiers;
} kEscapeRules[] = {
  {kUTuiDelete,   "\x1b[3~", false},
  {kUTuiPageUp,   "\x1b[5~", false},
  {kUTuiPageDown, "\x1b[6~", false},

  {kUTuiUpArrow,    "\x1b[A", true},
  {kUTuiRightArrow, "\x1b[C", true},
  {kUTuiDownArrow,  "\x1b[B", true},
  {kUTuiLeftArrow,  "\x1b[D", true},
  {kUTuiHome,       "\x1b[H", true},
  {kUTuiEnd,        "\x1b[F", true},

  // terminating element
  {kUTuiKeyNone, "", false},
};

static bool SeqStartsWith(struct UTuiInput *i, const char *s) {
  size_t len = strlen(s);
  if (len > i->seq_cnt)
    return false;
  return strncmp(i->seq, s, len) == 0;
}

// TODO: functions/macros for generating key numbers
UTuiKey UTuiInput_ReadKey(struct UTuiInput *in, int timeout) {
  if (!in->seq_cnt)
    UpdateInputQueue(in, timeout);
  else
    UpdateInputQueue(in, 0);
  if (in->error) return kUTuiInputError;
  if (in->seq_cnt == 0) return kUTuiKeyNone;

  // try to match a special key
  if (SeqStartsWith(in, "\x1b[")) {
    for (size_t i = 0; kEscapeRules[i].key != kUTuiKeyNone; i++) {
      struct EscapeRule *rule = &kEscapeRules[i];

      if (SeqStartsWith(in, rule->pattern)) {
        ConsumeInputQueue(in, strlen(rule->pattern));
        return rule->key;
      } else if (rule->supports_modifiers && in->seq_cnt >= 6 && SeqStartsWith(in, "\x1b[1;")) {
        /* Keys with modifiers held down use the following format:
         *    "\x1b[1;NX";
         *            ^^- key type        (index 5)
         *            |-- modifier number (index 4)
         */
        // check if single char key type matches (kind of hacky, but works)
        if (rule->pattern[2] != in->seq[5])
          continue;

        UTuiKey key = rule->key;
        // the modifier number is an ASCII digit representing a bit field
        // (control, alt, shift)
        int mod = in->seq[4] - '1';
        key |= mod << 13;
        ConsumeInputQueue(in, 6);

        return key;
      }
    }
  }

  UTuiKey key = in->seq[0];
  // transform control-h to backspace
  if (in->seq[0] == ('h' ^ 0x60)) {
    key = kUTuiKeyBackspace;
  // special case control characters that correspond directly to keys so that
  // they don't get interpreted as control keys
  } else if (in->seq[0] != kUTuiKeyEnter && in->seq[0] != kUTuiKeyEscape) {
    if (isupper(in->seq[0])) key |= kUTuiKeyShiftMask;
    if (iscntrl(in->seq[0])) key |= kUTuiKeyControlMask;
  }

  ConsumeInputQueue(in, 1);
  return key;
}