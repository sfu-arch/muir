#include <stdio.h>

unsigned test08(int j) {
  int foo = j;
  for (int i = 0; i < 5; ++i) {
    foo--;
  }
  return foo;
}

int main() {
  int result = test08(100);
  printf("%d\n",result);
}
