#include "thread.h"

void Ta() {
    while (1) {
        printf("abcdefg\r\n");
    }
}

void Tb() {
    while (1) {
        printf("1234567\r\n");
    }
}

int main() {
    create(Ta);
    create(Tb);
    return 0;
}