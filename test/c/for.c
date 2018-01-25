#include <stdio.h>

__always_inline void do_work(int i, int *a) {
  a[i+5] = 2*a[i];
}

void foo(int *a) {
  for (int i = 0; i < 5; ++i) {
    do_work(i, a);
  }
}

int main() {
  int a[10] = {0,1,2,3,4,0,0,0,0,0};
  foo(a);
}
