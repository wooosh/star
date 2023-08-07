#include "utui.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <inttypes.h>
#include <unistd.h>

static void *xrealloc(void *p, size_t n) {
  p = realloc(p, n);
  if (!p) {
    perror("realloc");
    abort();
  }
  return p;
}

static void *xrecalloc(void *p, size_t n) {
  p = xrealloc(p, n);
  memset(p, 0, n);
  return p;
}

static void WriteBuf(struct UTuiOutput *o, size_t n, const char *s) {
  if (o->output_buffer_len + n > o->output_buffer_cap) {
    if (!o->output_buffer_cap) {
      o->output_buffer_cap = 1;
    }

    while (o->output_buffer_cap < o->output_buffer_len + n)
      o->output_buffer_cap *= 2;

    o->output_buffer = xrealloc(o->output_buffer, o->output_buffer_cap);
  }

  memcpy(o->output_buffer + o->output_buffer_len, s, n);
  o->output_buffer_len += n;
}

static void WriteBufNullStr(struct UTuiOutput *o, const char *s) {
  WriteBuf(o, strlen(s), s);
}

struct UTuiOutput UTuiOutput_Init(void) {
  struct UTuiOutput o = {0};

  o.hash_state = XXH3_createState();
  if (o.hash_state == NULL)
    abort();

  // hide cursor when displaying text
  WriteBufNullStr(&o, "\x1b[?25l");

  return o;
}

void UTuiOutput_Destroy(struct UTuiOutput *o) {
  free(o->prev_row_hashes);
  free(o->output_buffer);
  XXH3_freeState(o->hash_state);

  memset(o, 0, sizeof(*o));
}

void UTuiOutput_Resize(struct UTuiOutput *o, size_t num_cols, size_t num_rows) {
  o->num_cols = num_cols;
  o->num_rows = num_rows;
  o->prev_row_hashes = xrecalloc(o->prev_row_hashes, num_rows);
}

static size_t ColorEscapeStr(size_t n, char *s, struct UTuiColor *c) {
  if (c->kind == kUTuiColorReset)
    return 0;
  else if (c->kind == kUTuiColorIndexed) {
    return snprintf(s, n, "%" PRId16 ";", c->color[0]);
  } else {
    // TODO: implement truecolor
    assert(0);
  }
  return 0;
}

static void WriteStyle(struct UTuiOutput *o, struct UTuiStyle *style) {
  enum {
    // color components are 3 digits followed by a semicolon
    kMaxColorEscLen = 4,
    kNumAttrs = 6,
    // attrs are two bytes each
    kMaxStyleEscLen = kMaxColorEscLen*2 + kNumAttrs*2
  };

  char escape_buf[4 + kMaxStyleEscLen + 1] = "\x1b[0;";
  // index into escape buffer
  size_t cur = 4;

  if (o->current_style.attr != style->attr) {
    static const uint8_t kAttrChars[] = {
      [0] = '1', // bold
      [1] = '2', // dim
      [2] = '4', // underline
      [3] = '5', // blink
      [4] = '7', // reverse
      [5] = '8', // hidden
    };

    for (int i = 0; i < kNumAttrs; i++) {
      if (style->attr & (1 << i)) {
        escape_buf[cur++] = kAttrChars[i];
        escape_buf[cur++] = ';';
      }
    }
  }

  if (memcmp(&o->current_style.fg, &style->fg, sizeof(style->fg)) != 0)
    cur += ColorEscapeStr(sizeof(escape_buf) - cur, escape_buf + cur, &style->fg);

  if (memcmp(&o->current_style.bg, &style->bg, sizeof(style->bg)) != 0)
    cur += ColorEscapeStr(sizeof(escape_buf) - cur, escape_buf + cur, &style->bg);

  // if escape codes have been written, move the cursor over the ; so it will
  // be treated like the end of the string
  if (cur != 4) {
    cur--;
    escape_buf[cur++] = 'm';
    escape_buf[cur++] = 0;
    WriteBuf(o, cur - 1, escape_buf);
  }

  o->current_style = *style;
}

static void MoveCursor(struct UTuiOutput *o, size_t x, size_t y) {
  char buf[16];
  snprintf(buf, sizeof(buf), "\x1b[%zu;%zuH", y + 1, x + 1);
  WriteBufNullStr(o, buf);
}

void UTuiOutput_SetLine(struct UTuiOutput *o, size_t y, size_t len, const char *s, struct UTuiStyle *style) {
  // TODO: handle lines that have control characters in them

  // compare hash to previous row hash
  if (XXH3_128bits_reset(o->hash_state) == XXH_ERROR)
    abort();

  XXH3_128bits_update(o->hash_state, s, len);
  XXH3_128bits_update(o->hash_state, style, len * sizeof(*style));

  XXH128_hash_t hash = XXH3_128bits_digest(o->hash_state);

  // this row has already been set with this data, so bail out early
  if (memcmp(&hash, &o->prev_row_hashes[y], sizeof(hash)) == 0)
    return;

  o->prev_row_hashes[y] = hash;

  // move to start of row
  MoveCursor(o, 0, y);

  // clear row of text
  WriteBufNullStr(o, "\x1bk");

  for (size_t i = 0; i < len; i++) {
    // update style info
    WriteStyle(o, &style[i]);
    WriteBuf(o, 1, s + i);
  }
}

void UTuiOutput_SetCursorPos(struct UTuiOutput *o, size_t x, size_t y) {
  o->cursor_x = x;
  o->cursor_y = y;
}

void UTuiOutput_SetCursorVisible(struct UTuiOutput *o, bool visible) {
  o->cursor_visible = visible;
}

void UTuiOutput_Flip(struct UTuiOutput *o) {
  MoveCursor(o, o->cursor_x, o->cursor_y);
  if (o->cursor_visible)
    WriteBufNullStr(o, "\x1b[?25h");

  size_t i = 0;
  while (i < o->output_buffer_len) {
    ssize_t n = write(STDOUT_FILENO, o->output_buffer + i, o->output_buffer_len - i);
    if (n == -1 || n == 0) {
      perror("write");
      abort();
    }
    i += n;
  }
  // hide cursor when displaying text
  WriteBufNullStr(o, "\x1b[?25l");
}