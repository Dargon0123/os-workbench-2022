#ifndef __PMM_H__
#define __PMM_H__


/* spin lock */
#define PMMLOCKED 1
#define PMMUNLOCEKD 0
#define CAPACITY 50

/* 定义mem size大小 */
#define KB * 1024LL
#define MB * (1024 KB)
#define MINSIZE (1)
#define PAGESIZE (4 KB) 
#define MAXSIZE (16 MB)

/* 
 * chunks的操作API 
 * bits[31-2] | bits[1] | bits[0]
 */
#define CHUNKS_FLAG_SIZE        (1)
#define CHUNKS_FLAG_BUDDY       (0)
#define CHUNKS_FLAG_SLAB        (1)

#define CHUNKS_STATUS_SIZE      (1)
#define CHUNKS_STATUS_INUSE     (0)
#define CHUNKS_STATUS_UNUSE     (1)

#define CHUNKS_IDX_SIZE         ( (sizeof(uintptr_t) * 8) -CHUNKS_FLAG_SIZE - CHUNKS_FLAG_SIZE )

#define CHUNKS_IDX_MASK         ( ((uintptr_t)(1) << (CHUNKS_IDX_SIZE)) -1 )
#define CHUNKS_STATUS_MASK      ( ((uintptr_t)(1) << (CHUNKS_IDX_SIZE + CHUNKS_STATUS_SIZE)) -1 - CHUNKS_IDX_MASK )
#define CHUNKS_FLAG_MASK        ( (~((uintptr_t)(0))) - CHUNKS_IDX_MASK - CHUNKS_STATUS_MASK )

#define CHUNKS_VAL_GET_IDX(val) ( ((uintptr_t)(val)) & CHUNKS_IDX_MASK )
#define CHUNKS_VAL_GET_STATUS(val)  ( ((uintptr_t)(val)) & CHUNKS_STATUS_MASK )
#define CHUNKS_VAL_GET_FLAG(val) ( ((uintptr_t)(val)) & CHUNKS_FLAG_MASK )

#define CHUNKS_VAL_SET_IDX(ptr, val) \
    (*((uintptr_t*)(ptr))) &= (~CHUNKS_IDX_MASK); \
    (*((uintptr_t*)(ptr))) |= (((uintptr_t)(val)) & CHUNKS_IDX_MASK)
#define CHUNKS_VAL_SET_STATUS(ptr, val) \
    (*((uintptr_t*)(ptr))) &= (~CHUNKS_STATUS_MASK); \
    (*((uintptr_t*)(ptr))) |= ( (((uintptr_t)(val)) << CHUNKS_IDX_SIZE) & CHUNKS_STATUS_MASK)
#define CHUNKS_VAL_SET_FLAG(ptr, val) \
    (*((uintptr_t*)(ptr))) &= (~CHUNKS_FLAG_MASK); \
    (*((uintptr_t*)(ptr))) |= ( (((uintptr_t)(val)) << (CHUNKS_IDX_SIZE + CHUNKS_STATUS_SIZE)) & CHUNKS_FLAG_MASK)

#define CHUNKS_GET_IDX(addr)    ( CHUNKS_VAL_GET_IDX(chunks[(((uintptr_t)(addr)) - chunk_base) / PAGESIZE]) )
#define CHUNKS_GET_STATUS(addr) ( CHUNKS_VAL_GET_STATUS(chunks[(((uintptr_t)(addr)) - chunk_base) / PAGESIZE]) )
#define CHUNKS_GET_FLAG(addr)   ( CHUNKS_VAL_GET_FLAG(chunks[(((uintptr_t)(addr)) - chunk_base) / PAGESIZE]) )

/* 宏函数里面有;外面不加括号 */
#define CHUNKS_SET_IDX(addr, idx)        CHUNKS_VAL_SET_IDX( chunks + ((((uintptr_t)(addr)) - chunk_base)  / PAGESIZE), (idx)) 
#define CHUNKS_SET_STATUS(addr, status)  CHUNKS_VAL_SET_STATUS(chunks + ((((uintptr_t)(addr)) - chunk_base) / PAGESIZE), (status)) 
#define CHUNKS_SET_FLAG(addr, flag)      CHUNKS_VAL_SET_FLAG(chunks + ((((uintptr_t)(addr)) - chunk_base) / PAGESIZE), (flag)) 

int chunk_size2buddy_idx(size_t size) {
    int i = 0;
    while (size >>= 1) {
        ++i;
    }
    return i;
}

#define CHUNK_SIZE2BUDDY_IDX(size)  (chunk_size2buddy_idx(size))
#define BUDDY_IDX2CHUNK_SIZE(idx)   (((size_t)idx + 1) * PAGESIZE)


typedef struct node_t {
    struct node_t *next, *pre;
    uintptr_t start, end;
} Node;
#define BIAS sizeof(Node)

typedef int lock_t;

typedef struct Chunk_t {
    lock_t          lock;   // 访问链表的lock
    struct Chunk_t*  fd;     // -->
    struct Chunk_t*  bk;     // <--
} Chunk;



#endif

