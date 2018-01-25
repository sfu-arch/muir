#include <stdio.h>
/*#include <cilk/cilk.h>*/

unsigned int_exp(unsigned *m, unsigned exp ) {
  //  cilk_for (unsigned i = 0; i < exp; ++i) {
  for (unsigned i = 0; i < exp; ++i) {
    *m *= *m;
  }
  return *m;
}

int main() {
  unsigned f;
  unsigned m = 2;
  f=int_exp(&m,5);
}
