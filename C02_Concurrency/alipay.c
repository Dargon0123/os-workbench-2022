#include "thread.h"

unsigned long balance = 100;

void Alipay_withdraw(int amt) {
  if (balance >= amt) {
    usleep(1); // unexpected delays
    balance -= amt;
  }
}

void Talipay(int id) {
  Alipay_withdraw(100);
}

static void fn(void) {
    // return;
} 

int main() {
  create(Talipay);
  create(Talipay);
  join(fn);
  printf("balance = %lu\n", balance);
}
