#include <stdio.h>
#include <cilk/cilk.h>


unsigned cilk_for_test12(unsigned *a, unsigned n) {
  int result = 0;
  cilk_for (unsigned i = 0; i < 3; ++i) {
    cilk_for (unsigned j = 0; j < n; ++j) {
      cilk_for (unsigned k = 0; k < n; ++k) {
	a[k] = 2*a[k];
      }
      a[n-1]++;
    }
    result += a[n-1]++;
  }
  result = result/2;
  return result;
}

int main() {
  unsigned a[8] = {0,1,2,3,4,5,6,7};
  int foo = cilk_for_test12(a,8);
  for(int i=0;i<8;i++) {
    printf("%u,", a[i]);
  }
  printf("\nreturned: %u\n", foo);
  return(0);
}
