#include <stdio.h>

unsigned test16(unsigned j) {
    unsigned foo = 0;
  for (unsigned i = 0; i < 5; ++i) {
    foo = foo * i + j;
  }
  return foo;
}

int main() {
  unsigned res = test16(100);
  printf("Result: %d\n", res);
}
