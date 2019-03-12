#include <cilk/cilk.h>
#include <stdio.h>

/*void cilk_for_test01_add(unsigned *a, unsigned *b, unsigned i) {*/
/*b[i] = a[i] * 2;*/
/*}*/

unsigned cilk_for_test01(unsigned *a, unsigned *b) {
  cilk_for(int i = 0; i < 5; i++) {
    /*cilk_for_test01_add(a, b, i); */
    b[i] = a[i] * 2;
  }
  return 1;
}

int main() {
  unsigned a[5] = {0, 1, 2, 3, 4};
  unsigned b[5] = {0};
  cilk_for_test01(a, b);
  for (int i = 0; i < 5; i++) {
    printf("b[%d]=%d\n", i, b[i]);
  }
}
