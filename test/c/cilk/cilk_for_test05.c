#include <stdio.h>
#include <cilk/cilk.h>

unsigned cilk_for_test05(unsigned *a, unsigned *b, unsigned *c) {
  cilk_for (unsigned i = 0; i < 20; ++i) {
  if (a[i] > b[i]) {
    c[i]=a[i]-b[i];
  } else {
    c[i]=b[i]-a[i];
  }      
}
  return 1;
}

int main() {
  int i;
  unsigned a[20] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
		     10,11,12,13,14,15,16,17,18,19};
  unsigned b[20] = {10,11,12,13,14,15,16,17,18,19,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  unsigned c[20] = {0};
  cilk_for_test05(a,b,c);
  for(i=0;i<20;i++) {
    printf("%d\n", c[i]);
  }

}

