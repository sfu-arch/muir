#include <stdio.h>

unsigned test10(unsigned *a, unsigned *b, unsigned *c) {
  for (unsigned i = 0; i < 5; ++i) {
    c[i]=a[i]+b[i];
  }
  return 0;
}

int main() {
  int i;
  unsigned a[5] = {1,2,3,4,5};
  unsigned b[5] = {1,2,3,4,5};
  unsigned c[5] = {0};
  test10(a,b,c);
  for(i=0;i<5;i++) {
    printf("%d\n", c[i]);
  }
}
