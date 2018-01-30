#include <stdio.h>
#include <cilk/cilk.h>

unsigned cilk_for_test03(unsigned j) {
  unsigned foo = j;
  cilk_for (unsigned i = 0; i < 5; ++i) {
      cilk_for(unsigned k = 0; k < 5; ++k){
        foo++;
      }
  }
  return foo;
}

int main() {
  int result = cilk_for_test03(100);
  printf("%d\n",result);
}
