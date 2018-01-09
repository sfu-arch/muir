#include <stdio.h>

unsigned test08(unsigned j) {
  unsigned foo = j;
  for (unsigned i = 0; i < 5; ++i) {
    foo++;
  }
  return foo;
}

int main() {
  int result = test08(100);
  printf("%d\n",result);
}
