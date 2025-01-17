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

#include <proc.h>
void naive_uload(PCB *pcb, const char *filename);
const char *default_bin = "/bin/nterm";

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

// #define CONFIG_STRACE
#ifdef CONFIG_STRACE
  char *syscall_name[20] = {"SYS_exit","SYS_yield","SYS_open","SYS_read",
  "SYS_write","SYS_kill","SYS_getpid","SYS_close","SYS_lseek","SYS_brk",
  "SYS_fstat","SYS_time","SYS_signal","SYS_execve","SYS_fork","SYS_link",
  "SYS_unlink","SYS_wait","SYS_times","SYS_gettimeofday"};

  // if(a[0]!=4)
  
  if(a[0] == 3 || a[0] == 4){
    printf("System_call %s with parameters 1:%d, 2:%p, 3:%d\n", syscall_name[a[0]], a[1], a[2], a[3]);
    char fmt_[6];
    if(a[0] == 3) strcpy(fmt_, "Read");
    else strcpy(fmt_, "Write");
    printf("%s file %s\n", fmt_, fd2name(a[1]));
  }else if(a[0] == 9){
    printf("System_call %s with parameters 1:%p, 2:%d, 3:%d\n", syscall_name[a[0]], a[1], a[2], a[3]);
  }else{
    printf("System_call %s with parameters 1:%d, 2:%d, 3:%d\n", syscall_name[a[0]], a[1], a[2], a[3]);
  }
#endif

  switch (a[0]) {
    case SYS_exit: {
      // halt(a[1]);
      naive_uload(NULL, default_bin);
    }
    case SYS_yield: {
      yield();
      c->GPRx = 0;
      break;
    }
    case SYS_write: {
      c->GPRx = vfs_write(a[1], (void *)a[2], a[3]);
      break;
    }
    case SYS_read: {
      c->GPRx = vfs_read(a[1], (void *)a[2], a[3]);
      break;
    }
    case SYS_brk: {
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
    case SYS_execve: {
      naive_uload(NULL, (char *)a[1]);
      break;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  c->mepc += 4;
}
