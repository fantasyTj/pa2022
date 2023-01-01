#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  printf("pdir is %p, reach here1\n", pdir);
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
  printf("reach here2\n");
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;
  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

#define HIGH_T(addr) ((uintptr_t)addr>>22)
#define LOW_T(addr) (((uintptr_t)addr & (0x003ff000)) >> 12)
#define PNN_MASK (0xfffff000)

void map(AddrSpace *as, void *va, void *pa, int prot) {
  printf("va is %p, pa is %p\t", va, pa);
  uintptr_t u_ptr = (uintptr_t)as->ptr, u_va = (uintptr_t)va, u_pa = (uintptr_t)pa;
  uintptr_t first_level = (u_ptr&PNN_MASK) | (HIGH_T(u_va)<<2);
  uint32_t first_val = *(uint32_t *)first_level;
  if(first_val % 2 != 0) { // already has firstpagetable
    // printf("hit\n");
    uintptr_t second_level = (first_val&PNN_MASK) | (LOW_T(u_va)<<2);
    *(uint32_t *)(second_level) = ((u_pa&PNN_MASK) | 1);
  }else {
    printf("miss\n");
    uintptr_t second_page = (uintptr_t)pgalloc_usr(PGSIZE);
    *(uint32_t *)first_level = ((second_page&PNN_MASK) | 1);
    uintptr_t second_level = (second_page&PNN_MASK) | (LOW_T(u_va)<<2);
    *(uint32_t *)second_level = ((u_pa&PNN_MASK) | 1);
  }
  printf("\n");
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context *context_start = (Context *)(kstack.end - sizeof(Context));
  context_start->mepc = (uintptr_t)entry;
  return context_start;
}
