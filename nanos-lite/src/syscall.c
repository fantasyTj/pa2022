#include <common.h>
#include "syscall.h"
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

#ifdef CONFIG_STRACE
  printf("System_call %d with parameters 1. %d, 2. %d, 3. %d\n", ANSI_FMT(a[0], ANSI_FG_CYAN), ANSI_FMT(a[1], ANSI_FG_WHITE), ANSI_FMT(a[2], ANSI_FG_WHITE), ANSI_FMT(a[3], ANSI_FG_WHITE));
#endif

  switch (a[0]) {
    case 0: { // SYS_exit
      halt(a[1]);
    }
    case 1: { // SYS_yield
      yield();
      c->GPRx = 0;
      break;
    }
    case 4: { // SYS_write
      switch(a[1]){
        case 1: case 2: {
          char *p = (void *)a[2];
          int count = a[3];
          for(int i = 0; i < count; i++){
            putch(*p++);
          }
          c->GPRx = count;
          break;
        }
        // default: putch(a[1]+'0');
      }
      break;
    }
    case 9: {
      extern uintptr_t program_break;
      memset((void *)program_break, 0, program_break-a[0]);
      program_break = a[0];
      c->GPRx = 0;
      break;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  c->mepc += 4;
}
