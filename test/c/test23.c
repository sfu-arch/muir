#include <stdio.h>

unsigned test23(int n, unsigned a[n]) {
  for (unsigned i = 1; i < n; ++i) {
      a[i-1] = a[i];
  }
  return a[n-1];
}

int main() {
  unsigned a[12] = {0,1,2,3,4,5,6,7,8,9,10,11};
  unsigned foo = test23(9, a);
  for(int i = 0 ; i<9; i ++ ){
    printf("%d, ", a[i]);
  }
  printf("\n");
  return (0);
}
