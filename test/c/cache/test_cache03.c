#include <stdio.h>

unsigned test_cache03(unsigned *a, unsigned *b, unsigned *c, unsigned n) {
  unsigned sum = 0;
  for (unsigned i = 0; i < n; i += 4) {
    c[i] = a[i] + b[i];
  }
  return c[20];
}

int main() {
  unsigned a[20] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                    10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
  unsigned b[20] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                    10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
  unsigned c[2] = {0};

  test_cache03(a, b, c, 20);
  for (unsigned i = 0; i < 20; i++)
    printf("%d, ", a[i]);
  /*printf("a[%d] = %u\n",i, a[i]);*/
  return (0);
}
