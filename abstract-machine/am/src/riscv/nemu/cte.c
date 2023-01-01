#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case 11: {
        switch(c->GPR1) {
          case -1: ev.event = EVENT_YIELD; break;
          case 0 : case 1 : case 2 : case 3 : case 4 : 
          case 5 : case 6 : case 7 : case 8 : case 9 :
          case 10: case 11: case 12: case 13: case 14:
          case 15: case 16: case 17: case 18: case 19: ev.event = EVENT_SYSCALL; break;
          default: printf("unhandle when ecall\n");
        }
        break;
      }
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    c->mstatus = 0x1800; // due to change the context, here are some problems, we may need to handle it in function "user_handler"
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}


Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context *context_start = (Context *)(kstack.end - sizeof(Context));
  context_start->mepc = (uintptr_t)entry;
  context_start->gpr[10] = (uintptr_t)arg; // set a[0]
  return context_start;
}

void yield() {
  asm volatile("li a7, -1; ecall"); // can i modify this?
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
