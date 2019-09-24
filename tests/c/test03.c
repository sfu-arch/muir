#include <stdio.h>

int test03(int a, int b) {
    if (a > b)
      a -= b;
    else
      b -= a;
    return a * b;
}

int main() {
  unsigned int a = 50;
  unsigned int b = 5;
  unsigned res = test03(a, b);
  printf("Res: %d\n", res);
  return 0;
}
