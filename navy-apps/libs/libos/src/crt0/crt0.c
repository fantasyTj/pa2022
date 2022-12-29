#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  printf("args is %p\n", args);
  void *pt = (void *)args;
  int argc = *(int *)pt;
  printf("argc is %d\n", argc);
  char **argv = (char **)(pt + sizeof(int *));
  printf("reach here1\n");
  char **envp = argv;
  printf("reach here2\n");
  while(*envp) envp++;
  envp++;
  printf("reach here3\n");
  // char *argv[] = {"--skip", NULL};
  char *empty[] =  {NULL };
  environ = empty;
  // environ = envp;
  // printf("pt is %p, argv is %p, envp is %p\n", pt, argv, envp);
  // exit(main(0, empty, empty));
  exit(main(argc, argv, envp));
  assert(0);
}
