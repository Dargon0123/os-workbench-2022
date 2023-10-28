#include <common.h>
#include <pmm.h>

/* global var */
uintptr_t* chunks;
size_t chunks_size;

Chunk* buddys;
size_t buddys_size;
uint64_t chunk_base;


/* 
 * 初始化锁
 */
static void lock_init(int *lock) {
    atomic_xchg(lock, PMMUNLOCEKD);
}

/* 
 * 获取锁，阻塞
 */
static void lock_acquire(int *lock) {
    while (atomic_xchg(lock, PMMLOCKED) == PMMLOCKED) {;}
}

/* 
 * 释放锁
 */
static void lock_release(int *lock) {
    panic_on(atomic_xchg(lock, PMMUNLOCEKD) != PMMLOCKED, "lock is not acquire\r\n");
}

/* 
 * 非阻塞形式获取锁
 * RETVAL: 0 unlock; 1 locked
 */
static int lock_try_acquire(int* lock) {
    return atomic_xchg(lock, PMMLOCKED);
}



/*
 * 计算ceil(log2(n))
 * 返回n最接近的次幂 left
 * n <= 2^left;
*/
static uintptr_t log_ceil(uintptr_t n) {
    panic_on(n <= 0, "error_log_ceil: n\r\n");
    if (n == 1) return 0;
    uintptr_t left = 0, right = sizeof(uintptr_t) * 8 - 1;

    /**
     * 二分 ：2^(a-1) < n <= 2^a
     * 返回合适的a
    */
    --n;
    while (left <= right) {
        uintptr_t mid = left + (right - left) / 2;
        if (n >> mid) left = mid + 1;
        else right = mid - 1;
    }

    return left;
}

/**
 * 将申请的内存大小，向上对齐到 2^left 
 * 大小给出限制
*/
static size_t requeset_size2mem_size(size_t size) {
    size = ((size_t)1) << log_ceil(size);
    panic_on(size > MAXSIZE, "error_requeset_size2mem_size: size \r\n");
    return size < MINSIZE ? MINSIZE : size;
}

/*
 * 将 chunk 插入到链表中
 * 此时的 chunk 已经完成所有的合并工作，只需要进行插入即可
 * 
 * 根据flag，确定需要插入的数组
 * 根据idx，确定需要插入的下标
 * 插入的时候，由于需要访问共享数据，需要确认上锁
 * 
 */
static void list_insert(Chunk* chunk) {
    /* 获取head节点 */
    Chunk* head = NULL;
    switch (CHUNKS_GET_FLAG(chunk))
    {
    case CHUNKS_FLAG_BUDDY:
        /* code */
        panic_on(CHUNKS_GET_IDX(chunk) >= buddys_size, "error idx");
        head = &(buddys[CHUNKS_GET_IDX(chunk)]);
        printf("CHUNKS_GET_IDX(chunk): %p\r\n", (uintptr_t)CHUNKS_GET_IDX(chunk));
        break;
    case CHUNKS_FLAG_SLAB:
        /* code */
        
        break;
    default:
        panic("error flag");
        break;
    }

    /* 确认是否获得锁 */
    panic_on(lock_try_acquire(&head->lock) != PMMLOCKED, "don't have the lock");

    /* 将该chunk插入到head和head->fd之间 */
    Chunk *back = head, *fwd = head->fd;
    chunk->bk = back;
    chunk->fd = head;
    back->fd = chunk;
    fwd->bk = chunk;
}

/**
 * 将chunk移除链表
 * 
 * 在buddys，对于合并的卸载亦或是单纯卸载这一个节点，都应该获取锁
 * 在卸载的时候，将其对应的chunks中的元素idx设置为CHUNKS_IDX_MASK
*/
static void list_remove(Chunk* chunk) {
    Chunk* head = NULL;
    switch (CHUNKS_GET_FLAG(chunk))
    {
    case CHUNKS_FLAG_BUDDY:
        /* code */
        panic_on(CHUNKS_GET_IDX(chunk) >= buddys_size, "error idx");
        head = &(buddys[CHUNKS_GET_IDX(chunk)]);
        break;
    case CHUNKS_FLAG_SLAB:
        break;
    default:
        panic("error flag");
        break;
    }

    /* 确认获取到锁 */
    panic_on(lock_try_acquire(&head->lock) != PMMLOCKED, "don't have the lock");

    /* 移除链表 */
    Chunk *fwd = chunk->fd, *back = chunk->bk;
    fwd->bk = back;
    back->fd = fwd;

}


static void chunks_init() {
    chunks = (uintptr_t*)heap.start;
    chunks_size  = ( (uintptr_t)heap.end - (uintptr_t)heap.start + PAGESIZE - 1 ) / PAGESIZE;

    printf("chunks: [%p, %p), chunks_size: %d\r\n", (uintptr_t)chunks, (uintptr_t)(chunks + chunks_size), chunks_size);
}

static void buddys_init(uint64_t start, uint64_t end) {
    buddys = (Chunk*)(void*)(uintptr_t)(chunks + chunks_size);
    buddys_size = (size_t)(log_ceil(MAXSIZE / PAGESIZE) + 1); 

    printf("buddys: [%p, %p), buddys_size: %d, Chunk_size: %d\r\n", (uintptr_t)buddys, (uintptr_t)buddys + (size_t)(buddys_size * sizeof(Chunk)), buddys_size, sizeof(Chunk));

    panic_on(start % MAXSIZE, "error start");
    panic_on(end % MAXSIZE, "error end");

    chunk_base = start;

    /* 初始化buddys数组元素的lock，fd，bk结构 */
    for (int i = 0; i < buddys_size; ++i) {
        buddys[i].fd = buddys[i].bk = &buddys[i];
        lock_init(&buddys[i].lock);
    }

    /* 将剩余内存以最大MAXSIZE大小的chunk 插入到链表中去 */
    for (uintptr_t iter = start; iter < end; iter += MAXSIZE) {
        /* 插入到buddys[buddys_size - 1]双向链表中 */
        CHUNKS_SET_IDX(iter, buddys_size - 1);
        CHUNKS_SET_STATUS(iter, CHUNKS_STATUS_UNUSE);
        CHUNKS_SET_FLAG(iter, CHUNKS_FLAG_BUDDY);
        printf("iter :%p, chunks: %p\r\n", (uintptr_t)iter, *(uintptr_t*)chunks);

        lock_acquire(&buddys[buddys_size - 1].lock);
        /* insert link list上 */
        list_insert((Chunk*)iter);
        lock_release(&buddys[buddys_size - 1].lock);
    }
    printf("buddys_init(%p, %p)\r\n", start, end);
}

static void slabs_init() {
    printf("slabs_init success\r\n");
}


static void pmm_init() {
    uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
    printf("Got %d MiB heap: [%p, %p)\n", (pmsize >> 20), heap.start, heap.end);

    chunks_init();
    buddys_init((uint64_t)0x4000000, (uint64_t)0x8000000);
    slabs_init();
}


/**
 * 从buddy中分配内存
 * 
 * 分配的时候，首先从低向高查询，找到第一个足够分配的内存
 * 如果该内存大小恰好对应申请的size大小，即可返回
 * 如果大于，则不停的进行二分拆分，将拆分的高地址一部分保留在buddy中
*/
static Chunk* buddys_alloc(size_t size) {
    uintptr_t res = 0;
    int idx = CHUNK_SIZE2BUDDY_IDX(size);
    panic_on(size < PAGESIZE, "size is too small");
    panic_on(size > MAXSIZE, "size is too big");

    /**
     * 此时从低向高遍历 
     * 找到第一个满足的chunk进行分配
    */
    int iter = idx;
    while (iter < buddys_size) {
        Chunk* head = &(buddys[iter]);
        lock_acquire(&(head->lock));

        if (head->fd != head) {
            res = (uintptr_t)head->fd;

            panic_on(CHUNKS_GET_IDX(res) != iter, "error idx");
            panic_on(CHUNKS_GET_STATUS(res) != CHUNKS_STATUS_UNUSE, "error status");
            panic_on(CHUNKS_GET_FLAG(res) != CHUNKS_FLAG_BUDDY, "error flag");

            CHUNKS_SET_STATUS(res, CHUNKS_STATUS_INUSE);
            list_remove((Chunk*)res);

            lock_release(&(head->lock));
            break;
        }

        lock_release(&(head->lock));
        ++iter;
    }

    /* 走到这里，表示buddys内存不足 */
    if (res == 0) return NULL;

    /**
     * 如需要1page，分出来2page，需要分割该chunk
     * 开始从高到底一直二分切割chunk
     * 然后设置chunks相关的idx，并将其放回到链表中去
     * 
     * 知识盲点：iter-- 在while()里面已经执行过
    */
    while (iter-- > idx) {
        // 此时iter已经减1操作了
        Chunk* chunk = (Chunk*)(uintptr_t)(res + BUDDY_IDX2CHUNK_SIZE(iter));
        Chunk* head = &(buddys[iter]);

        /* 设置chunk信息 */
        CHUNKS_SET_FLAG(chunk, CHUNKS_FLAG_BUDDY);
        CHUNKS_SET_STATUS(chunk, CHUNKS_STATUS_UNUSE);
        CHUNKS_SET_IDX(chunk, iter);

        lock_acquire(&(head->lock));
        list_insert((Chunk*)chunk);
        lock_release(&(head->lock));
    }

    /* 更改返回的idx */
    CHUNKS_SET_IDX(res, idx);
    CHUNKS_SET_STATUS(res, CHUNKS_STATUS_INUSE);
    CHUNKS_SET_FLAG(res, CHUNKS_FLAG_BUDDY);

    printf("buddys_alloc(%p), res: %p\r\n", (uint64_t)size, (uint64_t)res);

    /* 返回内存申请地址 */
    return (Chunk*)res;
}

/**
 * 将内存释放到buddy中，也是只有buddys中的才需要释放
 * 
 * 需要注意的是，buddys数组在释放的时候，如果条件合适的话，要进行合并
 * 1. 如果释放的是chunk1，则要释放的内存地址是chunk2 = chunk1 ^ size
 * 2. chunk1和chunk2的FLAG都为BUDDY
 * 3. chunk1和chunk2的IDX都相同
 * 4. chunk1和chunk2的STATUS都为UNUSE
*/
static void buddys_free(Chunk* chunk) {
    panic_on(CHUNKS_GET_IDX(chunk) >= chunks_size, "error idx");
    panic_on(CHUNKS_GET_STATUS(chunk) != CHUNKS_STATUS_INUSE, "error status");
    panic_on(CHUNKS_GET_FLAG(chunk) != CHUNKS_FLAG_BUDDY, "error flag");

    int idx = CHUNKS_GET_IDX(chunk);
    Chunk* head = &(buddys[idx]);

    lock_acquire(&(head->lock));

    /* 尝试合并相邻的buddys */
    while (idx < buddys_size - 1) {
        size_t size = BUDDY_IDX2CHUNK_SIZE(idx);
        /**
         * 此处用异或操作
         * 类似减去一个释放的chunk的大小
         * 11 0000 0000 ^ 1 0000 0000 = 10 0000 0000 
         * */
        Chunk* another_chunk = (Chunk*)( ( (uintptr_t)(chunk) ) ^ size);

        /* 若不满足以下条件，则不能合并 */
        if ( (CHUNKS_GET_IDX(another_chunk) != idx) || 
             (CHUNKS_GET_STATUS(another_chunk) != CHUNKS_STATUS_UNUSE) || 
             (CHUNKS_GET_FLAG(another_chunk) != CHUNKS_FLAG_BUDDY) ) {
            break;
        }

        CHUNKS_SET_STATUS(another_chunk, CHUNKS_STATUS_INUSE);
        list_remove(another_chunk);
        lock_release(&(head->lock));

        chunk = chunk < another_chunk ? chunk : another_chunk;
        idx++;

        head = &(buddys[idx]);
        lock_acquire(&(head->lock));
    }
    CHUNKS_SET_IDX(chunk, idx);
    CHUNKS_SET_STATUS(chunk, CHUNKS_STATUS_UNUSE);
    printf("buddys_free(%p), size: %p\r\n", (uint64_t)chunk, (uint64_t)(BUDDY_IDX2CHUNK_SIZE(CHUNKS_GET_IDX(chunk))));
    list_insert(chunk);
    lock_release(&(head->lock));
}


static Chunk* slabs_alloc(size_t size) {
    uintptr_t res = 0;

    return (Chunk*)res;
}

static void slabs_free(Chunk* chunk) {

}


static void *kalloc(size_t size) {
    void* ret = NULL;
    if (size > MAXSIZE) { return NULL; }
    if (size < MINSIZE) { size = MINSIZE; }

    size_t size_align = requeset_size2mem_size(size);

    // check size vailidity
    panic_on(size_align < MINSIZE, "size is too small");
    panic_on(size_align > MAXSIZE, "size is too big");

    ret = size_align < PAGESIZE ? slabs_alloc(size_align) : buddys_alloc(size_align);
    
    printf("kalloc(%p) = %p\r\n", (uint64_t)(size), (uint64_t)(uintptr_t)ret);
    return ret;
}

static void kfree(void *ptr) {
    if (ptr == NULL) return;
    panic_on((uintptr_t)ptr > (uintptr_t)heap.end, "invalid ptr");
    panic_on((uintptr_t)ptr < (uintptr_t)chunk_base, "invalid ptr");

    switch (CHUNKS_GET_FLAG(ptr))
    {
    case CHUNKS_FLAG_BUDDY:
        buddys_free(ptr);
        /* code */
        break;
    case CHUNKS_FLAG_SLAB:
        slabs_free(ptr);
        /* code */
        break;
    default:
        panic("error flag");
        break;
    }
}

#if defined TESTbuddy
/* 测试buddys方式 */
#define CAPACITY (500)
static void test_pmm() {
    printf("test_pmm: buddys\r\n");
    int size = 0;
    char* array[CAPACITY] = {NULL};
    int array_size[CAPACITY] = {0};

    array_size[0] = BUDDY_IDX2CHUNK_SIZE(11);
    array[0] = (char*)buddy_alloc(array_size[size]);

    if (array[0] == NULL) { 
        panic("buddy_alloc fail");
    }
    buddys_free((Chunk*)array[0])
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

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
  .test_pmm = test_pmm,
};
