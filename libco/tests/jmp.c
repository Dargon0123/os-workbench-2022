#include <stdio.h>
#include <setjmp.h>

void fun2();

jmp_buf my_buf[2];

void fun1()
{
    printf("Enter fun1 ...\n");
    // int val = setjmp(my_buf[1]);
    // if (val == 0) {
    //     printf("fun1: first time return from setjmp\n");
    //     fun2(); // debug ！！！
    // }
    // else if (val  == 2) {
    //     printf("fun1: first time return from fun2 longjmp\n");
    // }
    
    // printf("fun2 return\n");

    longjmp(my_buf[0], 1);//以下代码不会被执行
    printf("fun1::can't see\n");
}

void fun2() {
    printf("Enter fun2 ...\n");
    longjmp(my_buf[1], 2);//以下代码不会被执行
    printf("fun2::can't see\n");
}

int main()
{
    int ret = setjmp(my_buf[0]);
    if(ret == 1)
    {
        printf("Main fun_1: return after calling  longjmp, ret = %d.\n", ret);
    }
    printf("fun1 return after setjmp/longjmp, ret = %d\n", ret);
    if (ret == 0) 
    {
        printf("Main: first time return from setjmp, ret = %d\n", ret);
        fun1();
    }

    
    return 0;
}

// /* setjmp for x86-64.
//    Copyright (C) 2001-2018 Free Software Foundation, Inc.
//    This file is part of the GNU C Library.

//    The GNU C Library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.

//    The GNU C Library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.

//    You should have received a copy of the GNU Lesser General Public
//    License along with the GNU C Library; if not, see
//    <http://www.gnu.org/licenses/>.  */

// #include <sysdep.h>
// #include <jmpbuf-offsets.h>
// #include <jmp_buf-ssp.h>
// #include <asm-syntax.h>
// #include <stap-probe.h>

// /* Don't save shadow stack register if shadow stack isn't enabled.  */
// #if !SHSTK_ENABLED
// # undef SHADOW_STACK_POINTER_OFFSET
// #endif
// #define PTR_MANGLE

// ENTRY (__sigsetjmp)
// 	/* Save registers.  */
// 	movq %rbx, (JB_RBX*8)(%rdi)
// #ifdef PTR_MANGLE
// # ifdef __ILP32__
// 	/* Save the high bits of %rbp first, since PTR_MANGLE will
// 	   only handle the low bits but we cannot presume %rbp is
// 	   being used as a pointer and truncate it.  Here we write all
// 	   of %rbp, but the low bits will be overwritten below.  */
// 	movq %rbp, (JB_RBP*8)(%rdi)
// # endif
// 	mov %RBP_LP, %RAX_LP
// 	PTR_MANGLE (%RAX_LP)
// 	mov %RAX_LP, (JB_RBP*8)(%rdi)
// #else
// 	movq %rbp, (JB_RBP*8)(%rdi)
// #endif
// 	movq %r12, (JB_R12*8)(%rdi)
// 	movq %r13, (JB_R13*8)(%rdi)
// 	movq %r14, (JB_R14*8)(%rdi)
// 	movq %r15, (JB_R15*8)(%rdi)
// 	lea 8(%rsp), %RDX_LP	/* Save SP as it will be after we return.  */
// #ifdef PTR_MANGLE
// 	PTR_MANGLE (%RDX_LP)
// #endif
// 	movq %rdx, (JB_RSP*8)(%rdi)
// 	mov (%rsp), %RAX_LP	/* Save PC we are returning to now.  */
// 	LIBC_PROBE (setjmp, 3, LP_SIZE@%RDI_LP, -4@%esi, LP_SIZE@%RAX_LP)
// #ifdef PTR_MANGLE
// 	PTR_MANGLE (%RAX_LP)
// #endif
// 	movq %rax, (JB_PC*8)(%rdi)

// #ifdef SHADOW_STACK_POINTER_OFFSET
// # if IS_IN (libc) && defined SHARED && defined FEATURE_1_OFFSET
// 	/* Check if Shadow Stack is enabled.  */
// 	testl $X86_FEATURE_1_SHSTK, %fs:FEATURE_1_OFFSET
// 	jz L(skip_ssp)
// # else
// 	xorl %eax, %eax
// # endif
// 	/* Get the current Shadow-Stack-Pointer and save it.  */
// 	rdsspq %rax
// 	movq %rax, SHADOW_STACK_POINTER_OFFSET(%rdi)
// # if IS_IN (libc) && defined SHARED && defined FEATURE_1_OFFSET
// L(skip_ssp):
// # endif
// #endif
// #if IS_IN (rtld)
// 	/* In ld.so we never save the signal mask.  */
// 	xorl %eax, %eax
// 	retq
// #else
// 	/* Make a tail call to __sigjmp_save; it takes the same args.  */
// 	jmp __sigjmp_save
// #endif
// END (__sigsetjmp)
// hidden_def (__sigsetjmp)