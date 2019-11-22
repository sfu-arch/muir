#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LOOP_SIZE 300000
#define TIME

double timespec_to_ms(struct timespec *ts) {
  return ts->tv_sec * 1000.0 + ts->tv_nsec / 1000000.0;
}

void relu(int *in, int *out, unsigned N) {
  for (unsigned j = 0; j < N; ++j) {
    for (unsigned i = 0; i < N; ++i) {
      unsigned index = (j * N) + i;
      if (in[index] < 0)
        out[index] = 0;
      else
        out[index] = in[index];
    }
  }
}

#define TEST_SIZE 64

#define SEED 4

int main(int argc, char *argv[]) {

  int IN[TEST_SIZE];
  int OUT[TEST_SIZE];

  // prepare the input data
  srand(SEED);
  for (int i = 0; i < TEST_SIZE; ++i) {
    if (i < TEST_SIZE / 2)
      IN[i] = -i;
    else
      IN[i] = i;
  }

  printf("IN: ");
  for (int i = 0; i < TEST_SIZE; ++i) {
    printf(", %d", IN[i]);
  }

  // If we've got a parameter, assume it's the number of workers to be used
  if (argc > 1) {
    // Values less than 1, or parameters that aren't numbers aren't allowed
    if (atoi(argv[1]) < 1) {
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
  clock_gettime(CLOCK_MONOTONIC, &start_time);
#endif
  for (int i = 0; i < LOOP_SIZE; i++) {
    relu(IN, OUT, 8);
  }
#ifdef TIME
  clock_gettime(CLOCK_MONOTONIC, &end_time);
  double time_ms = timespec_to_ms(&end_time) - timespec_to_ms(&start_time);
  float time_ns = time_ms / LOOP_SIZE * 1000000;
  printf("Calculated in %.3f ns using %d workers.\n", time_ns, 1);
  /*__cilkrts_get_nworkers());*/
#endif

  return 0;
}
