#include <common.h>

void do_syscall(Context *c);

static Context* do_event(Event e, Context* c) {
  printf("event is %d\n", e.event);
  switch (e.event) {
    case 1: { // EVENT_YIELD
      // printf("yield\n");
      break;
    }
    case 2: { // EVENT_SYSCALL
      do_syscall(c);
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
