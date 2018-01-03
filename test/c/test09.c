#include <stdio.h>

unsigned test09(unsigned j) {
  unsigned foo = j;
  for (unsigned i = 0; i < 5; ++i) {
    foo=i+1;
  }
  return foo;
}

int main() {
  int result = test09(100);
  printf("%d\n",result);
}
