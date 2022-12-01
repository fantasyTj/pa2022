#include <common.h>
#include "syscall.h"

int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t count);
size_t fs_write(int fd, const void *buf, size_t count);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);
const char *fd2name(int fd);
size_t vfs_read(int fd, void *buf, size_t count);
size_t vfs_write(int fd, const void *buf, size_t count);

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

#ifdef CONFIG_STRACE
  printf("System_call %d with parameters 1. %d, 2. %d, 3. %d\n", ANSI_FMT(a[0], ANSI_FG_CYAN), ANSI_FMT(a[1], ANSI_FG_WHITE), ANSI_FMT(a[2], ANSI_FG_WHITE), ANSI_FMT(a[3], ANSI_FG_WHITE));
  if(a[0] == 3 || a[0] == 4){
    char fmt_[6];
    if(a[0] == 3) strcpy(fmt_, "Read");
    else strcpy(fmt_, "Write");
    printf("%s file %s\n", fmt, ANSI_FMT(fd2name(a[1]), ANSI_FG_GREEN));
  }
#endif

  switch (a[0]) {
    case SYS_exit: {
      halt(a[1]);
    }
    case SYS_yield: {
      yield();
      c->GPRx = 0;
      break;
    }
    case SYS_write: {
      vfs_write(a[1], (void *)a[2], a[3]);
      // switch(a[1]){
      //   case 0: break;
      //   case 1: case 2: {
      //     char *p = (void *)a[2];
      //     int count = a[3];
      //     for(int i = 0; i < count; i++){
      //       putch(*p++);
      //     }
      //     c->GPRx = count;
      //     break;
      //   }
      //   default: {
      //     c->GPRx = fs_write(a[1], (void *)a[2], a[3]);
      //   }
      // }
      break;
    }
    case SYS_read: {
      // vfs_read(a[1], (void *)a[2], a[3]);
      switch(a[1]){
        case 0: break;
        case 1: case 2: {
          break;
        }
        default: {
          c->GPRx = fs_read(a[1], (void *)a[2], a[3]);
        }
      }
      break;
    }
    case SYS_brk: {
      // extern uintptr_t program_break;
      // memset((void *)program_break, 0, program_break-a[0]);
      // program_break = a[0];
      c->GPRx = 0;
      break;
    }
    case SYS_open: {
      c->GPRx = fs_open(((char *)a[1]), a[2], a[3]);
      break;
    }
    case SYS_close: {
      c->GPRx = fs_close(a[1]);
      break;
    }
    case SYS_lseek: {
      c->GPRx = fs_lseek(a[1], a[2], a[3]);
      break;
    }
    case SYS_gettimeofday: {
      __time_t tm = io_read(AM_TIMER_UPTIME).us;
      struct timeval *tv = (void *)a[1];
      tv->tv_sec = tm / 1000000;
      tv->tv_usec = tm;
      c->GPRx = 0;
      break;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  c->mepc += 4;
}
