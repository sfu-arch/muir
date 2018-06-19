/*
 * fib.cpp
 *
 * Time how long it takes to calculate a Fibonacci number. See
 * http://en.wikipedia.org/wiki/Fibonacci_number for information about the
 * Fibonacci sequence. This application demonstrates the use of the cilk_spawn
 * and cilk_sync keywords.
 *
 * This program takes a single parameter to specify the number of workers to
 * be used in the calculation. If not specified, Intel Cilk Plus will query
 * the operating system to determine the number of cores on the system and use
 * that many workers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

#define LOOP_SIZE 100000
#define TIME

double timespec_to_ms(struct timespec *ts)
{
  return ts->tv_sec*1000.0 + ts->tv_nsec/1000000.0;
}

void fib_continue(int *x, int *y, int **r) {
  **r = *x + *y;
}

void fib(int n, int *r)
{
  int x,y;
  int *r_ptr = r;
  if (n < 2) {
    *r = n;
    return;
  } 
  cilk_spawn fib(n-1,&x);
  cilk_spawn fib(n-2,&y);
  cilk_sync;
  fib_continue(&x,&y, &r_ptr);
  return;
}


int main(int argc, char *argv[])
{
  // Fibonacci number to be calculated.  39 is big enough to take a
  // reasonable amount of time
  int n = 15;
  int result;
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

  // Time how long it takes to calculate the nth Fibonacci number
#ifdef TIME
  struct timespec start_time, end_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);
#endif  
  for (int i=0;i<LOOP_SIZE;i++) {
    fib(n,&result);
  }
#ifdef TIME
  clock_gettime(CLOCK_MONOTONIC, &end_time);
  double time_ms = timespec_to_ms(&end_time) - timespec_to_ms(&start_time);
  float time_ns = time_ms / LOOP_SIZE * 1000000;
  printf("Calculated in %.3f ns using %d workers.\n",
  	 time_ns, __cilkrts_get_nworkers());
#endif

  printf("Fibonacci number #%d is %d.\n", n, result);
  return 0;
}
