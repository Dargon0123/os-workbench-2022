#include <common.h>

static void os_init() {
  pmm->init();
}

#if defined TESTpmm
static void os_run() {
  printf("CPU #%c test pmm\r\n", ('0' + cpu_current()));
  pmm->test_pmm();
  while (1) ;
}
#else
static void os_run() {
  printf("Hello World from CPU #%c\n", ('0' + cpu_current()));
  while (1) ;
}
#endif

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
};
