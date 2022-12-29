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
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB *_pcb, void (*entry)(void *), void *arg) {
  Area kstack = {.start = (void *)_pcb, .end = (void *)_pcb + sizeof(PCB)};
  _pcb->cp = kcontext(kstack, entry, arg);
}

void context_uload(PCB *_pcb, const char *filename) {
  Area kstack = {.start = (void *)_pcb, .end = (void *)_pcb + sizeof(PCB)};
  uintptr_t entry = load_getentry(_pcb, filename);
  _pcb->cp = ucontext(NULL, kstack, (void *)entry);
  _pcb->cp->GPRx = (uintptr_t)heap.end;
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, (void *)1);
  // context_kload(&pcb[1], hello_fun, (void *)1);
  context_uload(&pcb[1], "/bin/pal");
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
