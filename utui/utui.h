#ifndef UTUI_H_
#define UTUI_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <xxhash.h>

typedef uint8_t UTuiColorKind;
enum {
  kUTuiColorReset,
  kUTuiColorIndexed,
  kUTuiColorTrueColor,
};

struct UTuiColor {
  UTuiColorKind kind;
  uint8_t color[3]; // rgb or single u8 indexed w/remaining cells zero
};

typedef uint8_t UTuiAttr;
enum {
  kUTuiBold      = 1 << 0,
  kUTuiDim       = 1 << 1,
  kUTuiUnderline = 1 << 2,
  kUTuiBlink     = 1 << 3,
  kUTuiReverse   = 1 << 4,
  kUTuiHidden    = 1 << 5,
};

struct UTuiStyle {
  struct UTuiColor fg;
  struct UTuiColor bg;
  UTuiAttr attr;
};

struct UTuiOutput {
  size_t num_cols, num_rows;

  bool cursor_visible;
  size_t cursor_x, cursor_y;

  // hash of the previous row contents for diffing
  XXH128_hash_t *prev_row_hashes;
  XXH3_state_t *hash_state;

  struct UTuiStyle current_style;

  size_t output_buffer_len;
  size_t output_buffer_cap;
  uint8_t *output_buffer;
};

struct UTuiOutput UTuiOutput_Init(void);
void UTuiOutput_Destroy(struct UTuiOutput *);

void UTuiOutput_Resize(struct UTuiOutput *, size_t num_cols, size_t num_rows);

// sets the line of text at row y to the given n byte long string with the given style information per byte
void UTuiOutput_SetLine(struct UTuiOutput *, size_t y, size_t len, const char *, struct UTuiStyle *);

void UTuiOutput_SetCursorPos(struct UTuiOutput *, size_t x, size_t y);
void UTuiOutput_SetCursorVisible(struct UTuiOutput *, bool);

// display all of the drawn data to the terminal
void UTuiOutput_Flip(struct UTuiOutput *);

#endif