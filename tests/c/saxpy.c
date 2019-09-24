#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/*#include <cilk/cilk.h>*/
/*#include <cilk/cilk_api.h>*/

#define LOOP_SIZE 1000000
#define TIME
#define N 100

double timespec_to_ms(struct timespec *ts)
{
  return ts->tv_sec*1000.0 + ts->tv_nsec/1000000.0;
}

int saxpy (int n, int a, int x[], int y[]) {
  for (int i = 0; i < n; i++) {
    y[i] = a * x[i] + y[i];
  }
  return 1;
}

int main (int argc, char *argv[]) {
  int n = N;
  int a = 5;
  int x[N];
  int y[N];
  for (int i=0;i<N;i++){
    x[i] = rand() % 10;
    y[i] = rand() % 10;
  }

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
      /*__cilkrts_set_param("nworkers", argv[1]);*/
#endif
    }

  // Time how long it takes to calculate the nth Fibonacci number
#ifdef TIME
  struct timespec start_time, end_time;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
#endif  
  for (int i=0;i<LOOP_SIZE;i++) {
    saxpy(n, a, x, y);
  }

#ifdef TIME
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
  double time_ms = timespec_to_ms(&end_time) - timespec_to_ms(&start_time);
  float time_ns = time_ms / LOOP_SIZE * 1000000;
  /*printf("Calculated in %.3f ns using %d workers.\n",*/
       /*time_ns, __cilkrts_get_nworkers());*/
#endif

  //  for (int i=0;i<n;i++) {
  //    printf("%d\n", y[i]);
  //  }
  return 0;
}
