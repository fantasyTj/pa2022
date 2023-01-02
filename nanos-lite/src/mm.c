// #include <memory.h>
#include <proc.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
  void *old_pf = pf;
  // printf("old_pf is %p\n", old_pf);
  pf += nr_page * PGSIZE;
  return old_pf;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  assert(n % PGSIZE == 0);
  void *st = new_page(n / PGSIZE);
  memset(st, 0, n);
  return st;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  if(brk <= current->max_brk) return 0;
  else {
    void *end = (void *)ROUNDDOWN(brk, PGSIZE);
    void *va = (void *)ROUNDDOWN(current->max_brk, PGSIZE) + 0x1000;
    for( ; va <= end; va += PGSIZE) {
      map(&current->as, va, new_page(1), 0);
    }
    current->max_brk = brk;
    return 0;
  }
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
