#include <stdio.h>
#include <cilk/cilk.h>

unsigned cilk_for_test03(unsigned *a, unsigned *b, unsigned *c) {
  cilk_for (unsigned i = 0; i < 10; ++i) {
    c[i]=a[i]+b[i];
  }
  return 1;
}

int main() {
  int i;
  unsigned a[10] = {1,2,3,4,5,6,7,8,9,10};
  unsigned b[10] = {1,2,3,4,5,6,7,8,9,10};
  unsigned c[10] = {0};
  cilk_for_test03(a,b,c);
  for(i=0;i<10;i++) {
    printf("%d\n", c[i]);
  }

}
