extern int volatile done;

void join() {
    while (!done) {
        asm volatile("":::"memory");
    }
}