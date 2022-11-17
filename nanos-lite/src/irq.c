#include <common.h>

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
    case 4: {
      // printf("yield\n");
      // for(int i = 0; i < 32; i++){
      //   printf("reg%d is %u\n", i, c->gpr[i]);
      // }
      // panic("Unhandled event ID = %d", e.event);
      break;
    }
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
