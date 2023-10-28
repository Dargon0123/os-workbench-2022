#include <stdio.h>

#if defined TESTbuddy
/* 测试buddys方式 */
static void test_pmm() {
    printf("test_pmm: buddys\r\n");
}
#elif defined TESTslab
/* 测试slabs方式 */
static void test_pmm() {
    printf("test_pmm: slabs\r\n");
}
#else
static void test_pmm() {
    printf("test_pmm: normal\r\n");
}
#endif

#if defined TESTpmm
static void os_run() {
  printf("test the pmm\r\n");
  test_pmm();
}
#else
static void os_run() {
  printf("Hello World from CPU\n");
  test_pmm();
}
#endif

int chunk_size2buddy_idx(size_t size) {
    int i = 0;
    while (size >>= 1) {
        ++i;
    }
    return i;
}
int main() {
    // os_run();
    // int i = 2;
    // while (i--) {
    //     printf("i = %d\r\n", i);
    // }
    printf("i = %d\r\n", chunk_size2buddy_idx(4096)); 
    return 0;
}