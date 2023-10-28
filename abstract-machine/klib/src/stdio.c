#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

void my_swap(char* a, char* b) {
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

void reverse_string(int start, int end, char* str) {
    if (start >= end) return;
    while (start < end) {
        my_swap(&str[start], &str[end]);
        start++;
        end--;
    }
}

static void my_itoa(int val, char* buf) {
    int i = 0;
    if (val < 0) {
        putch('-');
        val = -val;
    }
    while (val) {
        buf[i++] = ((val % 10) + '0');
        val /= 10;
    }
    buf[i] = '\0';
    reverse_string(0, i - 1, buf);
}

static void pointer_to_string(void* str, char* buffer) {
    uintptr_t addr = (uintptr_t)str;
    int i = 0;
    buffer[i++] = '0';
    buffer[i++] = 'x';
    if (addr == 0) buffer[i++] = '0';
    else {
        while (addr > 0) {
            int rem = addr % 16;
            buffer[i++] = (rem < 10) ? ('0' + rem) : (rem - 10 + 'A');
            addr /= 16;
        }
    }
    buffer[i] = '\0';

    // reverse string
    reverse_string(2, i - 1, buffer);
}

static inline void puts(const char *s) {
  for (; *s; s++) putch(*s);
}

// usage: printf("test printf: %d\r\n", var);
int printf(const char *fmt, ...) {
//   panic("Not implemented");
    va_list arg;
    va_start(arg, fmt);

    const char *p = fmt;
    char c, tmp;
    char buffer[20];

    while ((c = *p++) != '\0') {
        switch (c) {
        case '%':
            if ((tmp = *p) != '\0') {
                switch(tmp) {
                case '%':
                    /* %% --> % */
                    putch('%');
                    break;
                case 'd':
                    /* int --> str */
                    my_itoa(va_arg(arg, int), buffer);
                    puts(buffer);
                    break;
                case 'c':
                    putch(va_arg(arg, int));
                    break;
                case 's':
                    puts(va_arg(arg, char*));
                    break;
                case 'p': /* 指针 pointer */ 
                    pointer_to_string(va_arg(arg, void*), buffer);
                    puts(buffer);
                    break;
                case 'x': /* 打印十六进制 */ 
                    pointer_to_string((void*)va_arg(arg, uintptr_t), buffer);
                    puts(buffer);
                    break;
                case 'f':
                    puts("%f not implement!");
                    break;
                default:
                    break;
                }
            }
            else {
                putch(c);
            }
            p++;
            break;
        // case '\r':
        //     putch(c);
        //     break;
        // case '\n':
        //     putch(c);
        //     break;
        default:
            putch(c);
            break;
        }
    }

    va_end(arg);
        
    return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
