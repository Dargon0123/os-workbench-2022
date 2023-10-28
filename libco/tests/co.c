#define Example_1

#if defined Example_1

#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <stdlib.h>
#include "co.h"
#include <assert.h>

#define KB * 1024LL
#define MB KB * 1024LL
#define GB MB * 1024LL
#define MAX_CO 10
#define START_OF_STACK(stack) ((stack)+sizeof(stack))

#ifdef DEBUG
#define Log(format, ...) \
        printf("\33[1;34m[%s,%d,%s] " format "\33[0m\n", \
                        __FILE__, __LINE__, __func__, ## __VA_ARGS__)
#define Assert(cond, ...) \
      do { \
          if (!(cond)) { \
                Log(__VA_ARGS__); \
                assert(cond); \
              } \
        } while (0)
#define panic(format, ...) \
      Assert(0, format, ## __VA_ARGS__)
#define TODO() panic("please implement me")
static int times;
#define debug() do {\
    Log("DEBUG #%d", ++times);\
    void* sp;\
    asm volatile("mov " SP ", %0": "=g"(sp));\
    Log("SP = %p", sp);\
    for (int i=0;i<3;++i){\
        Log("stackptr %d: %p", i, crs[i].stackptr);\
    }\
} while (0);
#else
#define Log(format, ...)
#define Assert()
#define panic()
#define TODO()
#define debug()
#endif

#if defined(__i386__)
#define SP "%%esp"
#define BP "%%ebp"
#elif defined(__x86_64__)
#define SP "%%rsp"
#define BP "%%rbp"
#endif

#define changeframe(old, new)\
  asm volatile("mov " SP ", %0" :\
               "=g"(crs[old].stackptr));\
  asm volatile("mov %0, " SP :\
               :\
               "g"(crs[new].stackptr))
#define restoreframe(num)\
  asm volatile("mov %0," SP : : "g"(crs[num].stackptr))

struct co {
        char name[64];
        jmp_buf env;
        char done;
        void *stackptr;
        char stack[32 KB] __attribute__ (( aligned(16) ));
};
struct co crs[MAX_CO];
int co_num, cur;

//Variable storing local variable
func_t func_;
void *arg_;

void co_init()
{
        strcpy(crs[0].name, "main");
        crs[0].stackptr = NULL;
        co_num = cur = 0;
}

struct co *co_start(const char *name, func_t func, void *arg)
{
        func_ = func;
        arg_ = arg;

        co_num++;
        strcpy(crs[co_num].name, name);
        crs[co_num].done = 0;
        crs[co_num].stackptr = START_OF_STACK(crs[co_num].stack);

        int ind = setjmp(crs[cur].env);
        if (!ind) {
                changeframe(cur, co_num);
                cur = co_num;
                func_(arg_);    // Test #2 hangs
                crs[cur].done = 1;
                co_yield();
        }
        restoreframe(cur);

        return &(crs[co_num]);
}

void co_yield()
{
        int pre = cur;
        do {
                cur = rand() % (co_num + 1);
        } while (crs[cur].done);

        int ind = setjmp(crs[pre].env);
        debug();
        if (!ind) {
                changeframe(pre, cur);
                longjmp(crs[cur].env, 1);
        }
        restoreframe(cur);
}

void co_wait(struct co *thd)
{
        while (!thd->done) {
                int ind = setjmp(crs[cur].env);
                if (!ind) {
                        co_yield();
                }
        }
}


#elif defined Example_2


/*
 * 切换栈，即让选中协程的所有堆栈信息在自己的堆栈中，而非调用者的堆栈。保存调用者需要保存的寄存器，并调用指定的函数
 */
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
/*
 * 从调用的指定函数返回，并恢复相关的寄存器。此时协程执行结束，以后再也不会执行该协程的上下文。这里需要注意的是，其和上面并不是对称的，因为调用协程给了新创建的选中协程的堆栈，则选中协程以后就在自己的堆栈上执行，永远不会返回到调用协程的堆栈。
 */
static inline void restore_return() {
	asm volatile (
#if __x86_64__
			"movq 0(%%rsp), %%rcx" : : 
#else
			"movl 4(%%esp), %%ecx" : :  
#endif
			);
}

#define __LONG_JUMP_STATUS (1)
void co_yield() {
	int status = setjmp(current->context);
	if(!status) {
		//此时开始查找待选中的进程，因为co_node应该指向的就是current对应的节点，因此首先向下移动一个，使当前线程优先级最低
		co_node = co_node->bk;
		while(!((current = co_node->coroutine)->status == CO_NEW || current->status == CO_RUNNING)) { co_node = co_node->bk; }

		assert(current);

		if(current->status == CO_RUNNING) { longjmp(current->context, __LONG_JUMP_STATUS); }
		else {
			((struct co volatile*)current)->status = CO_RUNNING;	//这里如果直接赋值，编译器会和后面的覆写进行优化

			// 栈由高地址向低地址生长
			stack_switch_call(current->stack + STACK_SIZE, current->func, current->arg);
			//恢复相关寄存器
			restore_return();

			//此时协程已经完成执行
			current->status = CO_DEAD;

			if(current->waiter) { current->waiter->status = CO_RUNNING; }
			co_yield();
		}
	}

	assert(status && current->status == CO_RUNNING);		//此时一定是选中的进程通过longjmp跳转到的情况执行到这里
}

#elif defined Example_3

#include "co.h"
#include <stdlib.h>

#include <stdio.h>
#include "setjmp.h"
#include "string.h"
#include <stdint.h>

#define KB * 1024LL
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
    char        stack[32 KB] __attribute__ ((aligned(16) )); // co的堆栈区域，栈内存小，也会错 
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


static inline void change_frame(struct co* cur, struct co* new) {
#if __x86_64__
    asm volatile (
    "mov %%rsp, %0"
    :
    "=g"(cur->stackptr)
    );
    asm volatile (
    "mov %0, %%rsp"
    :
    :
    "c"(new->stackptr)
    );
#else 
    asm volatile (
    "movl %%esp, %0"
    :
    "=g"(cur->stackptr)
    );
    asm volatile (
    "movl %0, %%esp"
    :
    :
    "c"(new->stackptr)
    );
#endif
}

static inline void restore_frame(struct co* cur) {
#if __x86_64__
    asm volatile (
    "mov %0, %%rsp"
    :
    :
    "g"(cur->stackptr)
    );

#else   
    asm volatile (
    "mov %0, %%esp"
    :
    :
    "g"(cur->stackptr)
    );    
#endif
}



void co_init() {
    printf("co_init\n");
    head = (struct co*)malloc(sizeof(struct co));
    struct co* new_co = (struct co*)malloc(sizeof(struct co));
    // if (!new_co || !head) return;

    strcpy(new_co->name, "main");
    new_co->stackptr = NULL;
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
    // new_co->done = 0;
    new_co->stackptr = new_co->stack + sizeof(new_co->stack);
    new_co->status = CO_NEW; // Oops! ==

    new_co->waiter = head->waiter;
    head->waiter = new_co;


    int val = setjmp(cur->context); // 这里出错误,保存当前的状态
    if (val == 0) {
        change_frame(cur, new_co);
        cur->status = CO_WAITING;
        // new_co->waiter = cur;
        new_co->status = CO_RUNNING;
        cur = new_co;
        
        cur->func(cur->arg); /* running, call yield in work */
        cur->status = CO_DEAD;
        co_yield();
    }

    restore_frame(cur);
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
    struct co* pre = cur;
    do {
        cur = next_co(head);
    } while (cur->status == CO_RUNNING || cur->status == CO_DEAD);

    int val = setjmp(pre->context);
    if (val == 0) {
        // 保存当前栈帧，切换到cur环境运行
        change_frame(pre, cur);
        if (pre->status != CO_DEAD) {
            pre->status = CO_WAITING;
        }       
        cur->status = CO_RUNNING;
        longjmp(cur->context, 1);
    }
    restore_frame(cur);   
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
    while (co->status != CO_DEAD) {
        int val = setjmp(cur->context);
        if (val == 0) {
            co_yield();
        }
    }
    /* co 已经变为CO_DEAD状态 */
    delete_co(co);
}


# endif