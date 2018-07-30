#include <stdio.h>

int main() {

  int out[1000];
  int j;

#pragma clang loop unroll(full)
  for (j = 0; j < 1000; j++) {
    out[j] += 10;
  }

  return 1;
}
