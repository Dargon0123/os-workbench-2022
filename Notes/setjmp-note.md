
[toc]

# Disassembly code

# setjmp analys
## (gdb) disassemble main
```c
(gdb) disassemble main
Dump of assembler code for function main:
=> 0x00005555555551e3 <+0>:     endbr64 
   0x00005555555551e7 <+4>:     push   %rbp
   0x00005555555551e8 <+5>:     mov    %rsp,%rbp
   0x00005555555551eb <+8>:     sub    $0x10,%rsp
   0x00005555555551ef <+12>:    lea    0x2e4a(%rip),%rdi        # 0x555555558040 <my_jum_buf>
   0x00005555555551f6 <+19>:    callq  0x5555555550a0 <_setjmp@plt>
   0x00005555555551fb <+24>:    endbr64 
   0x00005555555551ff <+28>:    mov    %eax,-0x4(%rbp)
   0x0000555555555202 <+31>:    cmpl   $0x0,-0x4(%rbp)
   0x0000555555555206 <+35>:    je     0x555555555220 <main+61>
   0x0000555555555208 <+37>:    mov    -0x4(%rbp),%eax
   0x000055555555520b <+40>:    mov    %eax,%esi
   0x000055555555520d <+42>:    lea    0xe0c(%rip),%rdi        # 0x555555556020
   0x0000555555555214 <+49>:    mov    $0x0,%eax
   0x0000555555555219 <+54>:    callq  0x555555555090 <printf@plt>
   0x000055555555521e <+59>:    jmp    0x555555555240 <main+93>
   0x0000555555555220 <+61>:    mov    -0x4(%rbp),%eax
   0x0000555555555223 <+64>:    mov    %eax,%esi
   0x0000555555555225 <+66>:    lea    0xe24(%rip),%rdi        # 0x555555556050
   0x000055555555522c <+73>:    mov    $0x0,%eax
   0x0000555555555231 <+78>:    callq  0x555555555090 <printf@plt>
   0x0000555555555236 <+83>:    mov    $0x0,%eax
   0x000055555555523b <+88>:    callq  0x5555555551ce <fun>
   0x0000555555555240 <+93>:    mov    $0x0,%eax
   0x0000555555555245 <+98>:    leaveq 
   0x0000555555555246 <+99>:    retq   
End of assembler dump.
```

## (gdb) disassemble _setjmp
```c
(gdb) disassemble _setjmp
Dump of assembler code for function _setjmp:
   0x00007ffff7e05c80 <+0>:     endbr64 
   0x00007ffff7e05c84 <+4>:     xor    %esi,%esi
   0x00007ffff7e05c86 <+6>:     jmpq   0x7ffff7e05bb0 <__sigsetjmp>
End of assembler dump.
```

## (gdb) disassemble __sigsetjmp
```c
(gdb) disassemble __sigsetjmp
Dump of assembler code for function __sigsetjmp:
   0x00007ffff7e05bb0 <+0>:     endbr64 

   // # rbx 存入 jmp_buf[0]里面
   0x00007ffff7e05bb4 <+4>:     mov    %rbx,(%rdi) 

   // # rbp 寄存器进行加密操作,rbp 存入 jmp_buf[1]里面
   0x00007ffff7e05bb7 <+7>:     mov    %rbp,%rax
   0x00007ffff7e05bba <+10>:    xor    %fs:0x30,%rax
   0x00007ffff7e05bc3 <+19>:    rol    $0x11,%rax
   0x00007ffff7e05bc7 <+23>:    mov    %rax,0x8(%rdi)

   // r12-15 依次存入 jmp_buf[2-5]里面
   0x00007ffff7e05bcb <+27>:    mov    %r12,0x10(%rdi)
   0x00007ffff7e05bcf <+31>:    mov    %r13,0x18(%rdi)
   0x00007ffff7e05bd3 <+35>:    mov    %r14,0x20(%rdi)
   0x00007ffff7e05bd7 <+39>:    mov    %r15,0x28(%rdi)

   // [rsp +8]，对应main中调用setjmp时的rsp
   // 进行加密，存入到jmp_buf[6]里面
   0x00007ffff7e05bdb <+43>:    lea    0x8(%rsp),%rdx
   0x00007ffff7e05be0 <+48>:    xor    %fs:0x30,%rdx
   0x00007ffff7e05be9 <+57>:    rol    $0x11,%rdx
   0x00007ffff7e05bed <+61>:    mov    %rdx,0x30(%rdi)

   // rsp指向的是调用setjmp函数时，由硬件入栈的PC值
   // 对PC（rip）加密，存入到jmp_buf[7]里面
   0x00007ffff7e05bf1 <+65>:    mov    (%rsp),%rax
   0x00007ffff7e05bf5 <+69>:    nop
   0x00007ffff7e05bf6 <+70>:    xor    %fs:0x30,%rax
   0x00007ffff7e05bff <+79>:    rol    $0x11,%rax
   0x00007ffff7e05c03 <+83>:    mov    %rax,0x38(%rdi)

   // 
   0x00007ffff7e05c07 <+87>:    testl  $0x2,%fs:0x48
   0x00007ffff7e05c13 <+99>:    je     0x7ffff7e05c1e <__sigsetjmp+110>
   0x00007ffff7e05c15 <+101>:   rdsspq %rax
   0x00007ffff7e05c1a <+106>:   mov    %rax,0x58(%rdi)
   0x00007ffff7e05c1e <+110>:   jmpq   0x7ffff7e05c30 <__sigjmp_save>
End of assembler dump.
```

## (gdb) disassemble __sigjmp_save
```c
(gdb) disassemble __sigjmp_save 
Dump of assembler code for function __sigjmp_save:
   0x00007ffff7fec640 <+0>:     endbr64 
   0x00007ffff7fec644 <+4>:     movl   $0x0,0x40(%rdi)

   // rax，存储返回值，为0，即首次调用后，返回值是0
   0x00007ffff7fec64b <+11>:    xor    %eax,%eax

   // retq 指令，包含 mov (%rsp), $PC;将之前的pc从栈帧弹出
   0x00007ffff7fec64d <+13>:    retq   
End of assembler dump.
```

# longjmp analys
## (gdb) disassemble longjmp
```c
(gdb) disassemble longjmp
Dump of assembler code for function __libc_siglongjmp:
   0x00007ffff7e05c90 <+0>:     endbr64 
   0x00007ffff7e05c94 <+4>:     push   %r12
   0x00007ffff7e05c96 <+6>:     mov    %rdi,%r12
   0x00007ffff7e05c99 <+9>:     push   %rbp
   0x00007ffff7e05c9a <+10>:    mov    %esi,%ebp
   0x00007ffff7e05c9c <+12>:    sub    $0x8,%rsp
   0x00007ffff7e05ca0 <+16>:    callq  0x7ffff7e05dc0 <_longjmp_unwind>
   0x00007ffff7e05ca5 <+21>:    mov    0x40(%r12),%eax
   0x00007ffff7e05caa <+26>:    test   %eax,%eax
   0x00007ffff7e05cac <+28>:    jne    0x7ffff7e05cc2 <__libc_siglongjmp+50>
   0x00007ffff7e05cae <+30>:    test   %ebp,%ebp
   0x00007ffff7e05cb0 <+32>:    mov    $0x1,%eax

   // rdi:第一个参数jmp_buf； rsi:第二个参数
   0x00007ffff7e05cb5 <+37>:    mov    %r12,%rdi
   0x00007ffff7e05cb8 <+40>:    cmove  %eax,%ebp
   0x00007ffff7e05cbb <+43>:    mov    %ebp,%esi
   0x00007ffff7e05cbd <+45>:    callq  0x7ffff7e05d30 <__longjmp>

   0x00007ffff7e05cc2 <+50>:    lea    0x48(%r12),%rsi
   0x00007ffff7e05cc7 <+55>:    xor    %edx,%edx
   0x00007ffff7e05cc9 <+57>:    mov    $0x2,%edi
   0x00007ffff7e05cce <+62>:    callq  0x7ffff7e062c0 <__GI___sigprocmask>
   0x00007ffff7e05cd3 <+67>:    jmp    0x7ffff7e05cae <__libc_siglongjmp+30>
End of assembler dump.
```

## (gdb) disassemble __longjmp
```c
(gdb) disassemble __longjmp
Dump of assembler code for function __longjmp:
   0x00007ffff7e05d30 <+0>:     endbr64 
   // rsp = jmp_buf[6] 弹出，解密
   0x00007ffff7e05d34 <+4>:     mov    0x30(%rdi),%r8
   // rbp = jmp_buf[1] 弹出，解密
   0x00007ffff7e05d38 <+8>:     mov    0x8(%rdi),%r9
   // PC(rip) = jmp_buf[7] 弹出，解密
   0x00007ffff7e05d3c <+12>:    mov    0x38(%rdi),%rdx

   0x00007ffff7e05d40 <+16>:    ror    $0x11,%r8
   0x00007ffff7e05d44 <+20>:    xor    %fs:0x30,%r8
   0x00007ffff7e05d4d <+29>:    ror    $0x11,%r9
   0x00007ffff7e05d51 <+33>:    xor    %fs:0x30,%r9
   0x00007ffff7e05d5a <+42>:    ror    $0x11,%rdx
   0x00007ffff7e05d5e <+46>:    xor    %fs:0x30,%rdx

   0x00007ffff7e05d67 <+55>:    testl  $0x2,%fs:0x48
   0x00007ffff7e05d73 <+67>:    je     0x7ffff7e05da1 <__longjmp+113>
   0x00007ffff7e05d75 <+69>:    rdsspq %rax
   0x00007ffff7e05d7a <+74>:    sub    0x58(%rdi),%rax
   0x00007ffff7e05d7e <+78>:    je     0x7ffff7e05da1 <__longjmp+113>
   0x00007ffff7e05d80 <+80>:    neg    %rax
   0x00007ffff7e05d83 <+83>:    shr    $0x3,%rax
   0x00007ffff7e05d87 <+87>:    add    $0x1,%rax
   0x00007ffff7e05d8b <+91>:    mov    $0xff,%ebx
   0x00007ffff7e05d90 <+96>:    cmp    %rbx,%rax
   0x00007ffff7e05d93 <+99>:    cmovb  %rax,%rbx
   0x00007ffff7e05d97 <+103>:   incsspq %rbx
   0x00007ffff7e05d9c <+108>:   sub    %rbx,%rax
   0x00007ffff7e05d9f <+111>:   ja     0x7ffff7e05d90 <__longjmp+96>
   0x00007ffff7e05da1 <+113>:   nop
   0x00007ffff7e05da2 <+114>:   mov    (%rdi),%rbx

   // 依次恢复 r[12-15] 从jmp_buf[2-5]
   0x00007ffff7e05da5 <+117>:   mov    0x10(%rdi),%r12
   0x00007ffff7e05da9 <+121>:   mov    0x18(%rdi),%r13
   0x00007ffff7e05dad <+125>:   mov    0x20(%rdi),%r14
   0x00007ffff7e05db1 <+129>:   mov    0x28(%rdi),%r15

   0x00007ffff7e05db5 <+133>:   mov    %esi,%eax

   // 恢复 rsp，rbp
   0x00007ffff7e05db7 <+135>:   mov    %r8,%rsp
   0x00007ffff7e05dba <+138>:   mov    %r9,%rbp
   0x00007ffff7e05dbd <+141>:   nop

   // jmp 恢复PC
   0x00007ffff7e05dbe <+142>:   jmpq   *%rdx
End of assembler dump.
```


## 网址参考
[1. 博客网网址setjmp.c](https://www.cnblogs.com/maowen/p/5070002.html)




