#include <stdio.h>

void test16(unsigned j) {
  for (unsigned i = 0; i < 5; ++i) {
    unsigned foo = i;
    foo = foo * i;
  }
}

int main() {
  test16(100);
  printf("%d\n", 0);
}
