#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pty.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <termios.h>
#include <poll.h>
#include <signal.h>

#include <vterm.h>
#include <openssl/sha.h>
#include <janet.h>

// TODO: error handling for c functions in set up
// TODO: janet side error handling
// TODO: resize command
// TODO: janet functions for generate key inputs

pid_t g_slave_pid;
int g_master_fd;
VTerm *g_vt;
bool g_show_terminal;
bool g_terminal_is_alive;
struct winsize g_term_size = {43, 132};

void die(const char* msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

void assume(bool cond) {
  if (!cond) {
    perror("error");
    exit(EXIT_FAILURE);
  }
}


static struct termios origTermios;
void restore_termios() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &origTermios);
  // clear screen
  write(STDERR_FILENO, "\x1b""c", 2);
}

// TODO: fix this (this exists to get ctrl-c to restore termios)
void sigint_handle(int signum) {
  exit(EXIT_FAILURE);
}

void setup_termios() {
  if (tcgetattr(STDIN_FILENO, &origTermios) == -1)
    die("tcgetattr");
  if (atexit(restore_termios) != 0)
    die("atexit");
  if (signal(SIGINT, sigint_handle) == SIG_ERR)
    die("signal");

  struct termios raw = origTermios;
  // set up the terminal to not print characters we type or response to escape
  // codes
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

// returns master fd, sets pid of new process
int exec_in_pty(char **argv, struct winsize term_size, pid_t *pid_out) {
  // master side accepts output from process and provides input to it
  int master, slave;
  openpty(&master, &slave, NULL, NULL, &term_size);

  pid_t pid = fork();

  if (pid == 0) {
    close(master);

    // redirect process io to terminal device
    dup2(slave, STDIN_FILENO);
    dup2(slave, STDOUT_FILENO);
    dup2(slave, STDERR_FILENO);

    close(slave);

    execvp(argv[0], argv);
  }

  close(slave);

  *pid_out = pid;

  return master;
}

static Janet janet_tui_is_alive(int32_t argc, Janet *argv) {
  janet_fixarity(argc, 0);

  // check if the process has exited
  if (g_terminal_is_alive && waitpid(g_slave_pid, NULL, WNOHANG) == g_slave_pid) {
    g_terminal_is_alive = false;
  }

  return janet_wrap_boolean(g_terminal_is_alive);
}

static Janet janet_tui_process(int32_t argc, Janet *argv) {
  janet_fixarity(argc, 0);
  if (!g_terminal_is_alive) return janet_wrap_nil();

  struct pollfd pollfds[] = {
    {g_master_fd, POLLIN}
  };

  int ready = poll(pollfds, 1, 10);

  if (ready == -1) {
    die("poll");
  }

  if (ready > 0) {
    // check the terminal for process output
    if (pollfds[0].revents != 0) {
      if (pollfds[0].revents & POLLIN) { // input available
        char buf[4096];
        ssize_t size = read(g_master_fd, buf, 4096);
        if (size == -1) {
          die("read");
        }
        vterm_input_write(g_vt, buf, size);

        if (g_show_terminal) {
          // TODO: error handling on everything
          // TODO: prevent partial write
          write(STDERR_FILENO, buf, size);
        }

        if (vterm_output_get_buffer_current(g_vt) > 0) {
          size = vterm_output_read(g_vt, buf, 4096);
          // TODO: prevent partial write
          write(g_master_fd, buf, size);
        }
      } else { // POLLERR | POLLHUP
        // TODO: error on janet side
      }
    }
  }

  return janet_wrap_nil();
}

static Janet janet_tui_hash(int32_t argc, Janet *argv) {
  janet_fixarity(argc, 0);

  SHA_CTX ctx;
  SHA1_Init(&ctx);

  VTermScreen *vts = vterm_obtain_screen(g_vt);
  VTermState *vt_state = vterm_obtain_state(g_vt);

  VTermPos cursorpos;
  vterm_state_get_cursorpos(vt_state, &cursorpos);
  SHA1_Update(&ctx, &cursorpos, sizeof(VTermPos));

  for (int col = 0; col < g_term_size.ws_col; col++) {
    for (int row = 0; row < g_term_size.ws_row; row++) {
      VTermScreenCell cell;
      vterm_screen_get_cell(vts, (VTermPos){row, col},&cell);
      SHA1_Update(&ctx, cell.chars, cell.width*4);
    }
  }

  unsigned char hash[SHA_DIGEST_LENGTH];
  SHA1_Final(hash, &ctx);

  // double length because two hex chars per byte
  char hash_hex[SHA_DIGEST_LENGTH*2];
  for (int i = 0; i < sizeof(hash); i++) {
    snprintf(hash_hex + i*2, SHA_DIGEST_LENGTH*2 - i*2, "%x", hash[i]);
  }

  // TODO: probably should be it's on janet function, something like pause-interactive
  if (g_show_terminal) {
    // wait for keypress
    char scratch;
    read(STDIN_FILENO, &scratch, 1);
  }

  // TODO: fix
  //static const char *myhash = "hello janet";
  return janet_wrap_string(janet_string((const uint8_t *) hash_hex, sizeof(hash_hex)));
}

static Janet janet_tui_write(int32_t argc, Janet *argv) {
  janet_fixarity(argc, 1);
  // TODO: return janet error
  if (!g_terminal_is_alive) return janet_wrap_nil();

  // TODO: how does this behave when the argument isn't a string?
  const uint8_t *str = janet_getstring(argv, 0);
  const JanetStringHead *str_head = janet_string_head(str);

  // TODO: avoid partial writes
  write(g_master_fd, str_head->data, str_head->length);

  return janet_wrap_nil();
}

static const JanetReg cfuns[] = {
  // TODO: function documentation
  {"isalive", janet_tui_is_alive, "(tui/is-alive)\nTODO"},
  {"process", janet_tui_process, "(tui/process)\nTODO"},
  {"hash", janet_tui_hash, "(tui/hash)\nTODO"},
  {"write", janet_tui_write, "(tui/write)\nTODO"},
  {NULL, NULL, NULL}
};

int main(int argc, char** argv) {
  // parse arguments
  if (argc > 2 && strcmp(argv[1], "-r") == 0) {
    g_show_terminal = true;
    argv++;
    argc--;
  }

  if (argc < 3) {
    fprintf(stderr, "Usage: %s [-r] <file.janet> <executable> <arguments to executable>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  FILE* f = fopen(argv[1], "r");
  assume(f);
  int success = fseek(f, 0, SEEK_END);
  assume(!success);
  size_t len = ftell(f);
  assume(len >= 0);
  rewind(f);
  uint8_t *input = malloc(len);
  assume(input);
  size_t bytes_read = fread(input, 1, len, f);
  assume(len == bytes_read);
  int close_err = fclose(f);
  assume(close_err != EOF);

  // execute subprocess in virtual terminal
  g_master_fd = exec_in_pty(argv + 2, g_term_size, &g_slave_pid);
  g_terminal_is_alive = true;

  // initialize vterm
  g_vt = vterm_new(g_term_size.ws_row, g_term_size.ws_col);
  vterm_set_utf8(g_vt, 1);

  VTermScreen *vts = vterm_obtain_screen(g_vt);
  vterm_screen_reset(vts, 1);

  // TODO: query for terminal rezize properly

  // resize host terminal and set termios properly
  if (g_show_terminal) {
    fprintf(stderr, "\x1b[8;%zu;%zut", (size_t) g_term_size.ws_row, (size_t) g_term_size.ws_col);
    setup_termios();
  }

  // init janet
  janet_init();
  JanetTable *env = janet_core_env(NULL);
  janet_cfuns_prefix(env, "tui", cfuns);
  janet_dobytes(env, input, len, "main", NULL);

  // cleanup
  // TODO: need to kill forked process
  close(g_master_fd);
  free(input);
  vterm_free(g_vt);
  exit(EXIT_SUCCESS);
}
