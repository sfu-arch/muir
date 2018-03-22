#include <stdio.h>
#include <cilk/cilk.h>

int cilk_for_test10_inner(int n) {
  int a = 0;
  a += n;
  return a;
}

int cilk_for_test10(int m, int n) {
  int a = 0;
  cilk_for (int i = 0; i < m; ++i) {
    a += cilk_for_test10_inner(n);
  }
  return a;
}

int main() {
  int result = cilk_for_test10(5,10);
  printf("%d\n",result);
}
