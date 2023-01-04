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
    if(j % 100 == 0) {
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
  int argv_num = 0, envp_num = 0;
  int argv_num_arr[32], envp_num_arr[32]; // assume max_num is 32
  int argv_space = 0, envp_space = 0;
  while(*(argv + argv_num)) {
    // printf("argv[%d] is %s\n", argv_num, argv[argv_num]);
    argv_num_arr[argv_num] = UP((strlen(argv[argv_num])+1), 4);
    argv_space += argv_num_arr[argv_num++];
  }
  while(*(envp + envp_num)) {
    // printf("envp[%d] is %s\n", envp_num, *(envp+envp_num));
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
  // printf("reach end\n");
  // printf("new heap_end is %p\n", argc_pt);
  return (void *)argc_pt;
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]) {
  protect(&pcb->as);
  context_uload_without_protect(pcb, filename, argv, envp);
  // printf("protect done\n");
  // Area kstack = {.start = (void *)pcb, .end = (void *)pcb + sizeof(PCB)};
  // uintptr_t entry = load_getentry(pcb, filename);
  // printf("load done\n");
  // pcb->cp = ucontext(&pcb->as, kstack, (void *)entry);

  // // alloc stack
  // void *end = pcb->as.area.end;
  // void *va = end - (8 * PGSIZE);
  // for( ; va < end; va += PGSIZE) {
  //   map(&pcb->as, va, new_page(1), 0);
  // }
  // pcb->cp->GPRx = (uintptr_t)(load_args(end, argv, envp));
  // _pcb->cp->GPRx = (uintptr_t)(load_args(heap.end, argv, envp));
  // _pcb->cp->GPRx = (uintptr_t)heap.end;
}

void context_uload_without_protect(PCB *pcb, const char *filename, char *const argv[], char *const envp[]) {
  Area kstack = {.start = (void *)pcb, .end = (void *)pcb + sizeof(PCB)};
  uintptr_t entry = load_getentry(pcb, filename);
  pcb->cp = ucontext(&pcb->as, kstack, (void *)entry);
  printf("ucontext done\n");

  // alloc stack
  void *end = pcb->as.area.end;
  void *va = end - (8 * PGSIZE);
  for( ; va < end; va += PGSIZE) {
    map(&pcb->as, va, new_page(1), 0);
  }
  pcb->cp->GPRx = (uintptr_t)(load_args(end, argv, envp));
}

void init_proc() {
  char *empty[] =  {NULL };
  char *argv0[] = {"/bin/hello", NULL};
  context_uload(&pcb[0], "/bin/hello", argv0, empty);
  // context_kload(&pcb[0], hello_fun, (void *)1);
  // char *argv[] = {"/bin/pal", "--skip", NULL};
  // context_uload(&pcb[0], "/bin/pal", argv, empty);

  char *argv1[] = {"/bin/nterm", NULL};
  context_uload(&pcb[1], "/bin/nterm", argv1, empty);
  printf("pcb0 %p, pcb1 %p\n", &pcb[0], &pcb[1]);
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
  // current = &pcb[0];
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  // then return the new context
  return current->cp;
}
