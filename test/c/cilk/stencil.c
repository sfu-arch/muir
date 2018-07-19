#include <cilk/cilk.h>
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

#define NROWS (4)
#define NCOLS (4)  // NCOLS must be power of 2 to avoid use of MOD operator (unsupported)
#define NBRROWS (1)
#define NBRCOLS (1)
#define NWORKERS (4)
#define NSTEPS (5)
#define FILTCOLS (1 + NBRROWS * 2)
#define FILTROWS (1 + NBRCOLS * 2)
#define UNSIGNED

void print(unsigned x[]) {
    printf(
        "----------------------------------------------------------------------"
        "--------------------------\n");
    for (int i = 0; i < NROWS; i++) {
        for (int j = 0; j < NCOLS; j++) {
            printf("\t%d", x[i * NCOLS + j]);
        }
        printf("\n");
    }
}

#ifdef UNSIGNED
void stencil_inner(unsigned in[], unsigned out[], unsigned i, unsigned j,
                   unsigned nr) {
    for (unsigned nc = 0; nc <= 2 * NBRCOLS; nc++) {
        unsigned row = i + nr - NBRROWS;
        unsigned col = j + nc - NBRCOLS;
        if ((row < NROWS)) {
            if ((col < NCOLS)) {
                out[i * NCOLS + j] += in[row * NCOLS + col];
            }
        }
    }
}

void stencil(unsigned in[], unsigned out[]) {
    cilk_for(unsigned pos = 0; pos < NROWS * NCOLS; pos++) {
        unsigned i = pos / NCOLS;
        unsigned j = pos & (NCOLS - 1);
        for (unsigned nr = 0; nr <= 2 * NBRROWS; nr++) {
            stencil_inner(in, out, i, j, nr);
        }
        out[i * NCOLS + j] = (out[i * NCOLS + j] + (FILTROWS * FILTCOLS)) /
                             (FILTROWS * FILTCOLS);
    }
}
#else
void stencil(unsigned in[], unsigned out[]) {
    cilk_for(int pos = 0; pos < NROWS * NCOLS; pos++) {
        int i = pos / NCOLS;
        int j = pos & (NCOLS - 1);
        for (int nr = -NBRROWS; nr <= NBRROWS; nr++) {
            for (int nc = -NBRCOLS; nc <= NBRCOLS; nc++) {
                int row = i + nr;
                int col = j + nc;
                if ((row >= 0) && (row < NROWS)) {
                    if ((col >= 0) && (col < NCOLS)) {
                        out[i * NCOLS + j] += in[row * NCOLS + col];
                    }
                }
            }
        }
        out[i * NCOLS + j] = (out[i * NCOLS + j] + (FILTROWS * FILTCOLS)) /
                             (FILTROWS * FILTCOLS);
    }
}
#endif

int main(int argc, char *argv[]) {
    // initialize data
    unsigned buf0[NROWS * NCOLS];
    unsigned buf1[NROWS * NCOLS];
    for (int i = 0; i < NROWS; i++) {
        for (int j = 0; j < NCOLS; j++) {
            buf0[i * NCOLS + j] = rand() % 10;
            buf1[i * NCOLS + j] = 0;
        }
    }
    print(buf0);
    
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
    for (int i = 0; i < NSTEPS; i++) {
      //        if (i % 2)
      // stencil(buf1, buf0);
	    //        else
         stencil(buf0, buf1);
    }
  }
  #ifdef TIME
  clock_gettime(CLOCK_MONOTONIC, &end_time);
  double time_ms = timespec_to_ms(&end_time) - timespec_to_ms(&start_time);
  float time_ns = time_ms / LOOP_SIZE * 1000000;
  printf("Calculated in %.3f ns using %d workers.\n",
  	 time_ns, __cilkrts_get_nworkers());
#endif

    print(buf1);
    return 0;
}
