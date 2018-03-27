#include <stdio.h>
#include <stdlib.h>
#include <cilk/cilk.h>

#define N 10

void saxpy (int n, int a, int x[], int y[]) {
  cilk_for (int i = 0; i < n; i++) {
    y[i] = a * x[i] + y[i];
  }
}

int main () {
  int n = N;
  int a = 5;
  int x[N];
  int y[N];
  for (int i=0;i<N;i++){
    x[i] = rand() % 10;
    y[i] = rand() % 10;
  }
  saxpy(n, a, x, y);
  for (int i=0;i<n;i++) {
    printf("%d\n", y[i]);
  }
  return 0;
}
