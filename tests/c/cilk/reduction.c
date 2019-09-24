#include <stdio.h>
#include <cilk/cilk.h>
#include <stdlib.h>

#define N (1 << 4)
#define NWORKERS 4

int reduction (int a[], int n) {
  for (int step = n / 2; step > 0; step /= 2) {
    // #pragma cilk grainsize = step / NWORKERS
    cilk_for(int start = 0; start < step; start ++) {
      //printf("\nworker id = %d", __cilkrts_get_worker_number ());
      a[start] = a[start] + a[start + step];
    }
  }
  return a[0];
}

int main () {
  int a[N];
  for( int i = 0 ; i < N ; i++ ) {
    a[i] = rand() % 10;
    printf(",%d", a[i]);
  }

  int res = reduction(a, N);
  printf("\nsum = %d\n", res);
  return 0;

}


