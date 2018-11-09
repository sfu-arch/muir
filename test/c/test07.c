#include <stdio.h>
unsigned test07(int j) {
  for (int i = 0; i < 5; ++i) {
    j = j - 1;
  }
  return j;
}

int main() {
  int j = 100;
  unsigned res = test07(j);
  printf("Res: %d\n", res);
}
