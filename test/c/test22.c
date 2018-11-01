#include <stdio.h>

unsigned test22(int n, unsigned a[n][n]) {
  int result = 0;
  for (unsigned i = 0; i < n; ++i) {
    for (unsigned j = i + 1; j < n; ++j) {
      result += i + a[i][j];
    }
  }
  return result;
}

int main() {
  unsigned a[9][9] = {0};
  unsigned foo = test22(9, a);
  printf("\nreturned: %u\n", foo);
  return (0);
}
