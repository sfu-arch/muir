#include <stdio.h>

unsigned test_cache01(unsigned *a, unsigned n) {
  for (unsigned i = 0; i < n; i += 4) {
    a[i] = 2 * a[i];
    a[i + 1] = 4 * a[i+1];
    a[i + 2] = 6 * a[i+2];
    a[i + 3] = 8 * a[i+3];
  }
  return a[20];
}

int main() {
  unsigned a[20] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10,
                    11, 12, 13, 14, 15, 16, 17, 18, 19};
  test_cache01(a, 20);
  for(unsigned i = 0; i < 20; i++)
      printf("%d, ", a[i]);
    /*printf("a[%d] = %u\n",i, a[i]);*/
  return (0);
}
