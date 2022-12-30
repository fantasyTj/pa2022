#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void naive_uload(PCB *pcb, const char *filename);
uintptr_t load_getentry(PCB *pcb, const char *filename);

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    if(j % 5000 == 0) {
      Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    }
    j ++;
    yield();
  }
}

void context_kload(PCB *_pcb, void (*entry)(void *), void *arg) {
  Area kstack = {.start = (void *)_pcb, .end = (void *)_pcb + sizeof(PCB)};
  _pcb->cp = kcontext(kstack, entry, arg);
}

#define UP(a, num) (((a) + (num) - 1) & ~((num) - 1))

static void *load_args(void *end, char *const argv[], char *const envp[]) {
  printf("reach here\n");
  int argv_num = 0, envp_num = 0;
  int argv_num_arr[32], envp_num_arr[32]; // assume max_num is 32
  int argv_space = 0, envp_space = 0;
  while(*(argv + argv_num)) {
    printf("argv[%d] is %s\n", argv_num, argv[argv_num]);
    argv_num_arr[argv_num] = UP((strlen(argv[argv_num])+1), 4);
    argv_space += argv_num_arr[argv_num++];
  }
  while(*(envp + envp_num)) {
    printf("%p %p\n", envp, *(envp+envp_num));
    printf("envp_num is %d\n", envp_num);
    printf("envp[%d] is %s\n", envp_num, *(envp+envp_num));
    envp_num_arr[envp_num] = UP((strlen(envp[envp_num])+1), 4);
    envp_space += envp_num_arr[envp_num++];
  }

  int argc = argv_num;
  void *semi = end - (128 + argv_space + envp_space); // 128 is for safe
  void *start = semi - (4*(1+(argv_num+1)+(envp_num+1)));
  memset(start, 0, end - start); // initial all the bits to 0
  // set argc
  int *argc_pt = (int *)start;
  *argc_pt = argc;

  char **char_start = (char **)(start + sizeof(int *));
  char *char_semi = (char *)semi;
  // set argv
  for(int i = 0; i < argv_num; i++) {
    *char_start = strcpy(char_semi, argv[i]);
    char_start++;
    char_semi += argv_num_arr[i];
  }
  *char_start = NULL;
  char_start++;
  // set envp
  for(int i = 0; i < envp_num; i++) {
    *char_start = strcpy(char_semi, envp[i]);
    char_start++;
    char_semi += envp_num_arr[i];
  }
  printf("reach end\n");
  // printf("new heap_end is %p\n", argc_pt);
  return (void *)argc_pt;
}

void context_uload(PCB *_pcb, const char *filename, char *const argv[], char *const envp[]) {
  // printf("%p %p\n", envp, *envp);
  Area kstack = {.start = (void *)_pcb, .end = (void *)_pcb + sizeof(PCB)};
  _pcb->cp = ucontext(NULL, kstack, NULL);
  _pcb->cp->GPRx = (uintptr_t)(load_args((new_page(8)+(8*PGSIZE)), argv, envp));
  uintptr_t entry = load_getentry(_pcb, filename);
  _pcb->cp = ucontext(NULL, kstack, (void *)entry);
  // printf("reach here2\n");
  // printf("reach here3\n");
  // printf("reach here4\n");
  // _pcb->cp->GPRx = (uintptr_t)(load_args(heap.end, argv, envp));
  // _pcb->cp->GPRx = (uintptr_t)heap.end;
  // printf("heap.end is %p\n", heap.end);
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, (void *)1);
  // context_kload(&pcb[1], hello_fun, (void *)1);
  char *empty[] =  {NULL };
  // char *argv[] = {"/bin/pal", "--skip", NULL};
  // context_uload(&pcb[1], "/bin/pal", argv, empty);

  char *argv[] = {"/bin/nterm", NULL};
  context_uload(&pcb[1], "/bin/nterm", argv, empty);
  // context_uload(&pcb[1], "/bin/pal", empty, empty);
  switch_boot_pcb();

  Log("Initializing processes...");
  yield();
  // load program here
  naive_uload(NULL, "/bin/nterm");
}

Context* schedule(Context *prev) {
  // save the context pointer
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  // then return the new context
  return current->cp;
}
