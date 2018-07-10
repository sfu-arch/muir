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

int vector_scale(
    int* a, 
    int* c, 
    int scale, // 24.8 bit fixed point
	int N) {
  cilk_for (int32_t i = 0; i < N; ++i) {
    // Clip pixel values
    if (a[i] < 0) { 
      c[i] = 0;
    } else {
      c[i] = (a[i] * scale) >> 8;
      if (c[i] > 255) c[i] = 255;
    }
  }
  return 1;
}

#define TEST_SIZE 100
#define SEED 4

int main(int argc, char *argv[]) {

  int A[TEST_SIZE];
  int C[TEST_SIZE];

  // prepare the input data
  srand(SEED);
  for (int i = 0; i < TEST_SIZE; ++i) {
    A[i] = rand();
    //    B[i] = rand();
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
      __cilkrts_set_param("nworkers", argv[1]);
#endif
    }

  // Time how long it takes to calculate the nth Fibonacci number
#ifdef TIME
  struct timespec start_time, end_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);
#endif  
  for (int i=0;i<LOOP_SIZE;i++) {
    // Run the component
    vector_scale(A, C, 32, TEST_SIZE);
  }
#ifdef TIME
  clock_gettime(CLOCK_MONOTONIC, &end_time);
  double time_ms = timespec_to_ms(&end_time) - timespec_to_ms(&start_time);
  float time_ns = time_ms / LOOP_SIZE * 1000000;
  printf("Calculated in %.3f ns using %d workers.\n",
  	 time_ns, __cilkrts_get_nworkers());
#endif
  
  return 0;
}
