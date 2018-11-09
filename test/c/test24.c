#include <stdio.h>

unsigned test24(int n, unsigned a[n][n][n]) {
  for (unsigned i = 0; i < n; ++i) {
      for(unsigned j = 0; j < n; ++j){
          for(unsigned k = 0; k < n; ++k){
            a[i][j][k] = a[i][j][k] + n;
          }
      }
  }
  return a[n-1][n-2][n];
}

int main() {
  unsigned a[4][4][4] = {0,1,2,3,4,5,6,7,8,9,10,11, 12, 13};
  unsigned foo = test24(4, a);
  for(int i = 0 ; i<12; i ++ ){
    printf("%d, ", a[i]);
  }
  printf("\n");
  return (0);
}
