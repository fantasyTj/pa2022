#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  // test
  // for(int i = 0; i < 32; i++){
  //   printf("reg%d is %u\n", i, c->gpr[i]);
  // }
  printf("mcause is %u\n", c->mcause);
  // printf("mstatus is %u\n", c->mstatus);
  // printf("mepc is %u\n", c->mepc);
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case EVENT_YIELD: {
        ev.event = EVENT_YIELD;
        printf("here\n");
        // uint32_t *mepc_addr = (uint32_t *)(void *)c + 34;
        // *mepc_addr = c->mepc + 4;
        c->mepc += 4;
        break;
      }
      case EVENT_SYSCALL: {
        ev.event = EVENT_SYSCALL;
        break;
      }
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
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
  return NULL;
}

void yield() {
  asm volatile("li t0, 1; li a7, -1; ecall"); // can i modify this?
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
