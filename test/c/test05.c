#include <stdio.h>


__always_inline void do_work(unsigned i, unsigned *a) {
  a[i+5] = 2*a[i];
}

unsigned foo(unsigned *a) {
  for (unsigned i = 0; i < 5; ++i) {
    do_work(i, a);
  }

  return a[5];
}

int main() {

  unsigned a[10] = {0,1,2,3,4,0,0,0,0,0};

  foo(a);

}
