#include <stdio.h>

unsigned test22(int n, unsigned a[n][n]) {
  int result = 0;
  for (unsigned i = 0; i < n; ++i) {
    for (unsigned j = i + 1; j < n; ++j) {
      result += i + a[i][j];
    }
    /*printf("result: %d\n", result);*/
  }
  return result;
}

int main() {
  unsigned a[9][9] = {0};
  for(int i = 0 ; i<9; i ++ ){
      for(int j = 0 ; j < 9; j++)
          a[i][j] = 2;
  }
  unsigned foo = test22(5, a);
  printf("\nreturned: %u\n", foo);
  return (0);
}
