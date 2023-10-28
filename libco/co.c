#include "co.h"
#include <stdlib.h>

#include <stdio.h>
#include "setjmp.h"
#include "string.h"
#include <stdint.h>

#define KB * 1024LL
#define STACK_SIZE 32 KB
#define CO_MAX_NUMS 128

enum co_status {
    CO_NEW = 1, // 新创建，还未执行过
    CO_RUNNING, // 已经执行过
    CO_WAITING, // 在 co_wait 上等待
    CO_DEAD,    // 已经结束，但还未释放资源
};

struct co {
    char name[64];
    enum co_status status; // co 的状态
    void (*func)(void *); /* co_start指定入口地址 */
    void* arg; /* 参数 */
    void* stackptr;
    
    struct co*     waiter; // 是否有其它co在等待当前co
    jmp_buf        context;// 寄存器现场保存
    char        stack[STACK_SIZE] __attribute__ ((aligned(16) )); // Oops!! co的堆栈区域，栈内存小，也会错 
};

// struct co* co_queue[CO_MAX_NUMS];
int co_num = 0;
struct co* cur;
struct co* head;

// /*
// asm ( assembler template 
//            : output operands                  /* optional */
//            : input operands                   /* optional */
//            : list of clobbered registers      /* optional */
//            );
// */

static inline void stack_switch_call(void *sp, void *entry, void* arg) {
	asm volatile (
#if __x86_64__
			"movq %%rcx, 0(%0); movq %0, %%rsp; movq %2, %%rdi; call *%1"
			: : "b"((uintptr_t)sp - 16), "d"((uintptr_t)entry), "a"((uintptr_t)arg)
#else
			"movl %%ecx, 4(%0); movl %0, %%esp; movl %2, 0(%0); call *%1"
			: : "b"((uintptr_t)sp - 8), "d"((uintptr_t)entry), "a"((uintptr_t)arg) 
#endif
			);
}

static inline void restore_return() {
	asm volatile (
#if __x86_64__
			"movq 0(%%rsp), %%rcx" : : 
#else
			"movl 4(%%esp), %%ecx" : :  
#endif
			);
}


void co_init() {
    printf("co_init\n");
    head = (struct co*)malloc(sizeof(struct co));
    struct co* new_co = (struct co*)malloc(sizeof(struct co));

    strcpy(new_co->name, "main");
    new_co->waiter = NULL;
    new_co->status = CO_RUNNING; // main init status

    /* link */
    cur = new_co;
    head->waiter = new_co;
    co_num = 1;
}

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
    // malloc 初始化
    struct co* new_co = (struct co*)malloc(sizeof(struct co));
    if (!new_co) return NULL;
    co_num++;
    strcpy(new_co->name, name);
    new_co->func = func;
    new_co->arg = arg;
    new_co->status = CO_NEW; // Oops! ==

    /* link */
    new_co->waiter = head->waiter;
    head->waiter = new_co;

    return new_co;
}

static struct co* next_co(struct co* head) {
    int rand_step = rand() % (co_num);
    struct co* pivot = head->waiter;
    while (pivot && rand_step --) {
        pivot = pivot->waiter;
    }
    return pivot;
}

void co_yield() {
    // 随机选择下一个co 执行
    struct co* pre = cur;
    do {
        cur = next_co(head);
    } while (cur->status == CO_RUNNING || cur->status == CO_DEAD);
    if (pre->status != CO_DEAD)
        pre->status = CO_WAITING;

    int val = setjmp(pre->context);
    if (val == 0) {
        // 判断将执行的携程 cur 的status  
        if (cur->status == CO_NEW) {
            ((struct co volatile *)cur)->status = CO_RUNNING;
            stack_switch_call(cur->stack + STACK_SIZE, cur->func, cur->arg);
            restore_return();
            // 执行结束，函数返回
            ((struct co volatile *)cur)->status = CO_DEAD;
            co_yield();
        }
        else if (cur->status == CO_WAITING) {
            cur->status = CO_RUNNING;
            longjmp(cur->context, 1);
        }        
    }
    else {
        return; 
    }    
}

static void delete_co(struct co* node) {
    struct co* pre = head;
    struct co* curr = pre->waiter;
    while (curr && curr != node) {
        pre = curr;
        curr =curr->waiter;
    }
    if (curr == NULL) {
        printf("delete_co: not find delete node\n");
        return;
    }
    pre->waiter = curr->waiter;
    free(node);
    co_num--; // Oops !!!
}

void co_wait(struct co *co) {
    if (!co) return;
    // if (co->status == CO_RUNNING) co->status = CO_WAITING;
    while (co->status != CO_DEAD) {   
        co_yield();        
    }
    /* co 已经变为CO_DEAD状态 */
    delete_co(co);
}

