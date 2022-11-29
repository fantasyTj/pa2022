#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  // test
  // for(int i = 0; i < 32; i++){
  //   printf("reg%d is %u\n", i, c->gpr[i]);
  // }
  // printf("mcause is %u\n", c->mcause);
  // printf("mstatus is %u\n", c->mstatus);
  // printf("mepc is %u\n", c->mepc);
  if (user_handler) {
    Event ev = {0};

    // printf("temp_cause is %d\n", temp_cause);
    // c->mcause = temp_cause;
    switch (c->mcause) {
      case 11: {
        // uint32_t temp_cause = c->GPR1;
        // asm volatile("sw a7, %0": "=m"(temp_cause));
        // printf("cause is %d", c->GPR1);
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
      // case -1: { // yield
      //   ev.event = EVENT_YIELD;
      //   // uint32_t *mepc_addr = (uint32_t *)(void *)c + 34;
      //   // *mepc_addr = c->mepc + 4;
      //   // c->mepc += 4;
      //   break;
      // }
      // case 0: case 1: {
      //   ev.event = EVENT_SYSCALL;
      //   break;
      // }
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
  asm volatile("li a7, -1; ecall"); // can i modify this?
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
