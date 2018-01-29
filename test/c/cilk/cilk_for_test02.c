#include <stdio.h>
#include <cilk/cilk.h>

unsigned cilk_for_test02_add(unsigned a, unsigned b) {
  unsigned sum;
  sum = a + b;
  return sum;
}

unsigned cilk_for_test02(unsigned *a, unsigned *b, unsigned *c) {
  cilk_for (unsigned i = 0; i < 5; ++i) {
    c[i] = cilk_for_test02_add(a[i],b[i]);
  }
  return 1;
}

int main() {
  int i;
  unsigned a[5] = {1,2,3,4,5};
  unsigned b[5] = {1,2,3,4,5};
  unsigned c[5] = {0};
  cilk_for_test02(a,b,c);
  for(i=0;i<5;i++) {
    printf("%d\n", c[i]);
  }

}
