#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  // void *pt = (void *)args;
  // int argc = *(int *)pt;
  // char **argv = (char **)(pt + sizeof(int *));
  // char **envp = argv;
  // while(*envp) envp++;
  // envp++;
  char *empty[] =  {NULL };
  environ = empty;
  exit(main(0, empty, empty));
  // exit(main(argc, argv, envp));
  assert(0);
}
