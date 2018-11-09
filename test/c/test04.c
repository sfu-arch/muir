#include <stdio.h>

int test04(int a, int b) {
  for (int i = 0; i < a; i++) {
    if (a > b)
      a -= b;
    else
      b -= a;
  }
  return a * b;
}

int main() {
  unsigned int a = 50;
  unsigned int b = 5;
  unsigned res = test04(a, b);
  printf("Res: %d\n", res);
  return 0;
}
