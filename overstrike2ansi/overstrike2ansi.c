#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <wchar.h>
#include <locale.h>

typedef char Overstrike;
enum {
  kOverstrikeBlank     = 0,

  kOverstrikeNormal    = (1 << 0),
  kOverstrikeBold      = (1 << 1),
  kOverstrikeUnderline = (1 << 2),
  kOverstrikeCedilla   = (1 << 3),

  // only one upper diacritic is supported at a time
  kOverstrikeGrave      = (1 << 4),
  kOverstrikeTilde      = (2 << 4),
  kOverstrikeAcute      = (3 << 4),
  kOverstrikeDiaeresis  = (4 << 4),
  kOverstrikeCircumflex = (5 << 4)
};

// TODO: bold underscore instead of underlining underscore, check if the previous char was underscore or not

static void PutWChar(wchar_t wc) {
  char str[MB_CUR_MAX];
  int n = wctomb(str, wc);
  // we only use this function on unicode literals, so this should never fail
  assert(n != -1);
  fwrite(str, 1, n, stdout);
}

static void SetUpperDiacritic(size_t lineno, Overstrike *o, Overstrike d) {
  if (*o >> 4) {
    fprintf(stderr, "error @ line %zu: cannot set multiple upper diacritics\n",
            lineno);
    exit(EXIT_FAILURE);
  }
  *o |= d;
}

// oc -> overstrike char
// lc -> line char
static void SetOverstrike(size_t lineno, Overstrike *o, wchar_t *oc, wchar_t lc) {
  char x = *oc, y = lc;
  for (int i=0; i<2; i++) {
  switch (x) {
    case '_':  *o |= kOverstrikeUnderline;                          *oc = y; return;
    case ',':  *o |= kOverstrikeCedilla;                            *oc = y; return;
    case '`':  SetUpperDiacritic(lineno, o, kOverstrikeGrave);      *oc = y; return;
    case '~':  SetUpperDiacritic(lineno, o, kOverstrikeTilde);      *oc = y; return;
    case '\'': SetUpperDiacritic(lineno, o, kOverstrikeAcute);      *oc = y; return;
    case '"':  SetUpperDiacritic(lineno, o, kOverstrikeDiaeresis);  *oc = y; return;
    case '^':  SetUpperDiacritic(lineno, o, kOverstrikeCircumflex); *oc = y; return;
    default:
      if (x == y) {
        *o |= kOverstrikeBold;
        return;
      } else {
        x = lc, y = *oc;
      }
    }
  }
  fprintf(stderr, "error @ line %zu: cannot overstrike '0x%x' with '0x%x'\n",
          lineno, lc, *oc);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  setlocale(LC_ALL, "");

  char *line = NULL;
  size_t line_sz;
  wchar_t *wline = NULL;
  size_t wline_sz = 0;
  Overstrike *overstrikes = NULL;
  size_t overstrikes_sz = 0;
  ssize_t len;
  size_t lineno = 0;

  while ((len = getline(&line, &line_sz, stdin)) != -1) {
    lineno++;

    if (wline_sz < len*sizeof(wchar_t)) {
      wline_sz = line_sz*sizeof(wchar_t);
      wline = realloc(wline, wline_sz);
      if (!wline) {
        perror("realloc");
        return EXIT_FAILURE;
      }
    }

    const char *src = line;
    size_t wlen = mbsrtowcs(wline, &src, len, NULL);
    if (wlen == (size_t) -1) {
      perror("mbsrtowcs");
      return EXIT_FAILURE;
    }

    if (overstrikes_sz < wlen) {
      overstrikes_sz = line_sz;
      overstrikes = realloc(overstrikes, overstrikes_sz);
      if (!overstrikes) {
        perror("realloc");
        return EXIT_FAILURE;
      }
    }
    memset(overstrikes, 0, wlen);

    size_t cursor = 0;
    for (size_t i=0; i<wlen; i++) {
      if (wline[i] == 0x08) {
        if (cursor > 0) cursor--;
      } else {
        if (overstrikes[cursor] == kOverstrikeBlank) {
          overstrikes[cursor] = kOverstrikeNormal;
          wline[cursor] = wline[i];
        } else {
          SetOverstrike(lineno, overstrikes + cursor, wline + cursor, wline[i]);
        }
        cursor++;
      }
    }

    int bold = 0, underline = 0;
    for (size_t i=0; i<cursor; i++) {
      Overstrike o = overstrikes[i];
      assert(o != kOverstrikeBlank);

      int o_bold = o & kOverstrikeBold;
      int o_underline = o & kOverstrikeUnderline;
      if (bold != o_bold || underline != o_underline) {
        printf("\x1b[%s;%sm",
               o_bold      ? "1" : "22",
               o_underline ? "4" : "24");
        bold = o_bold;
        underline = o_underline;
      }

      PutWChar(wline[i]);

      if (o & kOverstrikeCedilla) PutWChar(L'\u0327');

      // upper diacritics
      o &= 0xFF << 4;
      if (o == kOverstrikeGrave)      PutWChar(L'\u0300');
      if (o == kOverstrikeTilde)      PutWChar(L'\u0303');
      if (o == kOverstrikeAcute)      PutWChar(L'\u0301');
      if (o == kOverstrikeDiaeresis)  PutWChar(L'\u0308');
      if (o == kOverstrikeCircumflex) PutWChar(L'\u0302');
    }
  }

  int errno_save = errno;
  if (!feof(stdin)) {
    errno = errno_save;
    perror("getline");
    return EXIT_FAILURE;
  }

  free(line);
  free(wline);
  free(overstrikes);

  return EXIT_SUCCESS;
}