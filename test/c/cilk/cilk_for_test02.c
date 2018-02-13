#include <stdio.h>
#include <cilk/cilk.h>

void cilk_for_test02_mul(unsigned *a, unsigned *b, unsigned i) {
  b[i] = a[i]*2;
  return;
}

void cilk_for_test02(unsigned *a, unsigned *b) {
  cilk_for (unsigned i = 0; i < 5; ++i) {
    cilk_for_test02_mul(a,b,i);
  }
  return;
}

int main() {
  int i;
  unsigned a[5] = {1,2,3,4,5};
  unsigned b[5] = {0};
  cilk_for_test02(a,b);
  for(i=0;i<5;i++) {
    printf("b[%d]=%d\n", i, b[i]);
  }

}
