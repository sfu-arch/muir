#include <stdio.h>
#include <cilk/cilk.h>


int cilk_for_test09(int m, int n) {
  int a = 0;
  cilk_for (int i = 0; i < m; ++i) {
    int b = 0;
    for(int j = 0; j < n; ++j) {
      b++;
    }
    a += b;
  }
  return a;
}

int main() {
  int result = cilk_for_test09(5,10);
  printf("%d\n",result);
}
