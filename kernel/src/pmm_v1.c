// #include <common.h>
// #include <pmm.h>


// /* spin lock */
// #define PMMLOCKED 1
// #define PMMUNLOCEKD 0
// #define CAPACITY 50

// /* 
//  * 初始化锁
//  */
// static void lock_init(int *lock) {
//     atomic_xchg(lock, PMMUNLOCEKD);
// }

// /* 
//  * 获取锁，阻塞
//  */
// static void lock_acquire(int *lock) {
//     while (atomic_xchg(lock, PMMLOCKED) == PMMLOCKED) {;}
// }

// /* 
//  * 释放锁
//  */
// static void lock_release(int *lock) {
//     panic_on(atomic_xchg(lock, PMMUNLOCEKD) != PMMLOCKED, "lock is not acquire\r\n");
// }

// /* 
//  * 非阻塞形式获取锁
//  * RETVAL: 0 unlock; 1 locked
//  */
// // static int lock_try_acquire(int* lock) {
// //     return atomic_xchg(lock, PMMLOCKED);
// // }


// /* global var */
// uintptr_t* chunks;
// size_t chunks_size;
// uintptr_t* buddys;
// size_t buddys_size;


// /*
//  * 计算ceil(log2(n))
//  * 返回n最接近的次幂 left
//  * n <= 2^left;
// */
// static uintptr_t log_ceil(uintptr_t n) {
//     panic_on(n <= 0, "error_log_ceil: n\r\n");
//     if (n == 1) return 0;
//     uintptr_t left = 0, right = sizeof(uintptr_t) * 8 - 1;

//     /**
//      * 二分 ：2^(a-1) < n <= 2^a
//      * 返回合适的a
//     */
//     --n;
//     while (left <= right) {
//         uintptr_t mid = left + (right - left) / 2;
//         if (n >> mid) left = mid + 1;
//         else right = mid - 1;
//     }

//     return left;
// }

// /**
//  * 将申请的内存大小，向上对齐到 2^left 
//  * 大小给出限制
// */
// static size_t requeset_size2mem_size(size_t size) {
//     size = ((size_t)1) << log_ceil(size);
//     panic_on(size > MAXSIZE, "error_requeset_size2mem_size: size \r\n");
//     return size < MINSIZE ? MINSIZE : size;
// }

// // static void chunks_init() {
// //     chunks = (uintptr_t*)heap.start;
// //     chunks_size = ( ((uintptr_t)heap.end) - ((uintptr_t)heap.start) + PAGESIZE - 1 ) / PAGESIZE;
// //     printf("chunks: [%x, %x), chunks_size: %d", (uintptr_t)chunks, (uintptr_t)(chunks + chunks_size), chunks_size);
// // }

// // static void buddys_init(uint64_t start, uint64_t end) {

// // }

// // static void slabs_init() {

// // }

// typedef int lock_t;
// Node *head, *tail;

// // lock
// lock_t alloc_lock;
// uintptr_t pm_start, pm_end;

// /* 初始化一个双向链表 */
// void list_init(uintptr_t start, uintptr_t end) {
//     head = (void*)start;
//     tail = (void*)end - BIAS;
//     head->next = head->pre = tail;
//     tail->next = tail->pre = head;

//     /* link */
//     head->start = start;
//     head->end = head->start + BIAS;
    
//     tail->end = end;
//     tail->start = tail->end - BIAS;
//     printf("head->start: %p\t head->end: %p\t \r\n", head->start, head->end);
//     printf("tail->start: %p\t tail->end: %p\t \r\n", tail->start, tail->end);
//     printf("start: %p\t end: %p\t \r\n", start, end);
// }

// void* add_node(Node* p, size_t size) {
//     Node* tmp;
//     tmp = (void*)p->end;
//     tmp->start = p->end;
//     tmp->end = tmp->start + size + BIAS;

//     /* link */
//     tmp->pre = p;
//     tmp->next = p->next;
//     p->next->pre = tmp;
//     p->next = tmp;
    
//     return (void*)(tmp->start + BIAS);
// }

// void delete_node(Node* p) {
//     p->next->pre = p->pre;
//     p->pre->next = p->next;
// }

// static void pmm_init() {
//     pm_start = (uintptr_t)heap.start;
//     pm_end = (uintptr_t)heap.end;
//     uintptr_t pmsize = ((uintptr_t)heap.start - (uintptr_t)heap.end);
//     printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
//     lock_init(&alloc_lock);

//     // 初始化 list
//     list_init(pm_start, pm_end);
// }


// static void *kalloc(size_t size) {
//     void* ret = NULL;
//     if (size > MAXSIZE) { return NULL; }
//     if (size < MINSIZE) { size = MINSIZE; }

//     size_t size_align = requeset_size2mem_size(size);

//     // check size vailidity
//     panic_on(size_align < MINSIZE, "size is too small");
//     panic_on(size_align > MAXSIZE, "size is too big");
 
//     lock_acquire(&alloc_lock);
//     for (Node* p = head; p != tail; p = p->next) {
//         // enough space to alloc
//         if ((p->next->start - p->end) >= size_align + BIAS) {
//             ret = add_node(p, size_align);
//             printf("cpu #%c: malloc(%d) = %p [%p, %p)\r\n", ('0' + cpu_current()), size_align, ret, p->next->start + BIAS, p->next->end);
//             break;
//         }
//     }

//     if (ret == NULL) {
//         printf("No enough space!\r\n");
//     }
//     memset(ret, 0, size_align);
//     lock_release(&alloc_lock);
//     return ret;
// }

// static void kfree(void *ptr) {
//     Node* p = (Node*)((uintptr_t)ptr - BIAS);
//     lock_acquire(&alloc_lock);
//     delete_node(p);
//     printf("cpu #%c: free [%p, %p)\r\n", ('0' + cpu_current()), p->start + BIAS, p->end);
//     lock_release(&alloc_lock);
// }

// #if defined TESTbuddy
// /* 测试buddys方式 */
// static void test_pmm() {
//     printf("test_pmm: buddys\r\n");
// }
// #elif defined TESTslab
// /* 测试slabs方式 */
// static void test_pmm() {
//     printf("test_pmm: slabs\r\n");
// }
// #else
// static void test_pmm() {
//     int idx = 0;
//     char* array[CAPACITY] = {NULL};
//     int array_size[CAPACITY] = {0};
//     while (1) {
//         switch (rand() % 2) {
//             case 0:
//                 if (idx > CAPACITY) break;
//                 array_size[idx] = rand() % 1024;
//                 array[idx] = (char*)kalloc(array_size[idx]);
//                 if (!array[idx]) { printf("kalloc error!\r\n"); }
//                 ++idx;
//                 break;

//             case 1:
//                 if (idx <= 0) break;
//                 --idx;
//                 kfree(array[idx]);
//                 break;
//         }
//     }

//     // printf("test_pmm: normal\r\n");
// }
// #endif

// MODULE_DEF(pmm) = {
//   .init  = pmm_init,
//   .alloc = kalloc,
//   .free  = kfree,
//   .test_pmm = test_pmm,
// };
