#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

static void sh_handle_cmd(const char *cmd) {
  const char *exec_argv[2];
  char cmd_t[64];
  // if(!strncmp(cmd, "/bin", 4)){
  //   size_t slen = strlen(cmd);
  //   strncpy(cmd_t, cmd, slen);
  //   cmd_t[slen-1] = '\0';
  //   exec_argv[0] = cmd_t;
  //   exec_argv[1] = NULL;
  //   printf("cmd is %s\n", exec_argv[0]);
  //   execve(exec_argv[0], (char **)exec_argv, NULL);
  // }else{
  //   return;
  // }

  size_t slen = strlen(cmd);
  strncpy(cmd_t, cmd, slen);
  cmd_t[slen-1] = '\0';
  exec_argv[0] = cmd_t;
  exec_argv[1] = NULL;
  execvp(exec_argv[0], (char **)exec_argv);
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();
  setenv("PATH", "/bin:", 0);

  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
