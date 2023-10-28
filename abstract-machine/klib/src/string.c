#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
    const char* p = s;
    size_t size = 0;
    while (*p++ != '\0') {
        size++;
    }
    return size;
}


char *strcpy(char *dst, const char *src) {
    if (dst == NULL || src ==NULL) return dst;
    char* p = dst;
    while ((*p++ = *src++) != '\0') {;}
    return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
    if (dst == NULL || src ==NULL || n <= 0) return dst;
    char* p = dst;
    while (n && (*p++ = *src++) != '\0') {
        n--;
    } 
    if (n) {
        while (--n) {
            *p++ = '\0';
        }
    }
    return dst;
}

char *strcat(char *dst, const char *src) {
    if (dst == NULL || src == NULL) return dst;
    char* p = dst;
    while (*p != '\0') {
        p++;
    }
    while (*src != '\0') {
        *p++ = *src++;
    }
    *p = '\0';
    return dst;
}

/*
 * @strcmp
 * retval: 
 * 0 -- euqal; >0 -- s1 > s2; <0 -- s1 < s2;
 */
int strcmp(const char *s1, const char *s2) {
    assert((s1 != NULL) && (s2 != NULL));
    if (s1 == NULL && s2 != NULL) return -1;
    else if (s1 != NULL && s2 == NULL) return 1;
    else if (s1 == NULL && s2 == NULL) return 0;
    while (*s1 && *s2 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    assert((s1 != NULL) && (s2 != NULL));
    if (s1 == NULL && s2 != NULL) return -1;
    else if (s1 != NULL && s2 == NULL) return 1;
    else if (s1 == NULL && s2 == NULL) return 0;
    while (n && *s1 && *s2 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

void *memset(void *s, int c, size_t n) {
    // 将指针转换为unsigned char*，按字节进行操作
    unsigned char* byte_ptr = (unsigned char*)s;
    unsigned char byte_val = (unsigned char)c;

    for (size_t i = 0; i < n; ++i) {
        byte_ptr[i] = byte_val;
    }
    return byte_ptr;
}

void *memmove(void *dst, const void *src, size_t n) {
    void* ret = dst;
    /* 注意可能出现重叠 */
    if (dst <= src || (char*)dst >= ((char*)src + n)) {
        // 从前向后复制
        while (n--) {
            *(char*)dst++ = *(char*)src++;
        }
    }
    else {
        // 从后向前复制
        dst = (char*)dst + n -1;
        src = (char*)src + n -1;
        while (n--) {
            *(char*)dst-- = *(char*)src--;
        }
    }
    return ret;
    // panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
    assert(out != NULL && in != NULL);
    unsigned char* p = out;
    const unsigned char* s = in;
    while (n--) {
        *p++ = *s++;
    } 
    return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    assert(s1 != NULL && s2 != NULL);
    const char* s = (char*)s1;
    const char* p = (char*)s2;
    while (n-- && *s && *p && (*s == *p)) {
        s++;
        p++;
    }
    return *s - *p;
}

#endif
