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
    switch (c->mcause) {
      case EVENT_YIELD: {
        printf("here\n");
        ev.event = EVENT_YIELD; 
        uint32_t *mepc_addr = (uint32_t *)(void *)c + 34;
        *mepc_addr = c->mepc + 4;
        // asm("lw t2, %0\n\t"
        //       "addi t2, t2, 4\n\t"
        //       "sw t2, %0"
        //       : :"r"(c)
              // :"t2");
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
  asm volatile("li a7, 1; ecall"); // 
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
