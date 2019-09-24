#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

#define LOOP_SIZE 1000000
#define TIME

double timespec_to_ms(struct timespec *ts)
{
  return ts->tv_sec*1000.0 + ts->tv_nsec/1000000.0;
}


int cilk_for_test05(int a[][5], int b[][5], int c[][5]) {
  cilk_for (int i = 0; i < 5; ++i) {
    cilk_for (int j = 0; j < 5; ++j) {
      c[i][j]=a[i][j]+b[i][j];
    }
  }

  return 1;
}

int main(int argc, char *argv[]) {
  int i,j;
  int a[5][5] = {{ 1, 2, 3, 4, 5},
		 {11,12,13,14,15},
		 {21,22,23,24,25},
		 {31,32,33,34,35},
		 {41,42,43,44,45}};
  int b[5][5] = {{ 1, 2, 3, 4, 5},
		 {11,12,13,14,15},
		 {21,22,23,24,25},
		 {31,32,33,34,35},
		 {41,42,43,44,45}};
  int c[5][5] = {0};
  // If we've got a parameter, assume it's the number of workers to be used
  if (argc > 1)
    {
      // Values less than 1, or parameters that aren't numbers aren't allowed
      if (atoi(argv[1]) < 1)
        {
	  printf("Usage: fib [workers]\n");
	  return 1;
        }

      // Set the number of workers to be used
#ifdef TIME      
      __cilkrts_set_param("nworkers", argv[1]);
#endif
    }

  // Time how long it takes
#ifdef TIME
  struct timespec start_time, end_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);
#endif  

  for (int i=0;i<LOOP_SIZE;i++) {
    cilk_for_test05(a,b,c);
  }

#ifdef TIME
  clock_gettime(CLOCK_MONOTONIC, &end_time);
  double time_ms = timespec_to_ms(&end_time) - timespec_to_ms(&start_time);
  float time_ns = time_ms / LOOP_SIZE * 1000000;
  printf("Calculated in %.3f ns using %d workers.\n",
  	 time_ns, __cilkrts_get_nworkers());
#endif
  
  for(i=0;i<5;i++) {
    for(j=0;j<5;j++) {
      printf("%d ", c[i][j]);
    }
    printf("\n");
  }

}
